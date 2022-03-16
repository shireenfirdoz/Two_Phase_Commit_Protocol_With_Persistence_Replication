#include <iostream>
#include <memory>
#include <fstream>
#include <unistd.h>

#include "ServerThread.h"

using namespace std;

LaptopFactory::LaptopFactory(std::vector<peer_servers_info> peers_input, int peer_count_input)
{
	last_index = -1;
	committed_index = -1;
	primary_id = -1;
	factory_id = -1;
	peers = peers_input;
	peer_count = peer_count_input;
	for (int i = 0; i < peer_count; i++)
	{
		peerNodes.push_back(ClientStub());
		livePeers.push_back(false);
	}
	recovery_cust_rec();
	recovery_txn_log();
}

void LaptopFactory::setFactoryId(int id)
{
	factory_id = id;
}

LaptopInfo LaptopFactory::CreateRegularLaptop(CustomerOrder order, int engineer_id)
{
	LaptopInfo laptop;
	laptop.CopyOrder(order);
	laptop.SetEngineerId(engineer_id);

	std::promise<LaptopInfo> prom;
	std::future<LaptopInfo> fut = prom.get_future();

	std::unique_ptr<AdminRequest> req =
		std::unique_ptr<AdminRequest>(new AdminRequest);
	req->laptop = laptop;
	req->customer_id = order.GetCustomerId();
	req->last_order_num = order.GetOrderNumber();
	req->prom = std::move(prom);

	arq_lock.lock();
	arq.push(std::move(req));
	arq_cv.notify_one();
	arq_lock.unlock();

	laptop = fut.get();
	return laptop;
}

void LaptopFactory::EngineerIFAThread(std::unique_ptr<ServerSocket> socket, int engineerId)
{
	ServerStub stub;
	stub.Init(std::move(socket));
	ReceiveIDMsgAndInitiateFlow(stub, engineerId);
}

void LaptopFactory::ReceiveIDMsgAndInitiateFlow(ServerStub &stub, int engineerId)
{
	//std::cout << "waiting for id msg\n";
	std::string IDMsg = stub.IdentificationMsg();
	//std::cout << "ID msg is: " << IDMsg << std::endl;

	// If the client is identified, follow engineer flow:
	if (IDMsg.compare("CLIENT") == 0)
	{
		ReadEngineerFlow(stub);
	}

	// IF the PFA is identified, follow IFA flow:
	else if (IDMsg.compare("PFA") == 0)
	{
		IFAFlow(stub, engineerId);
	}

	else if (IDMsg.compare("TMPrepare") == 0)
	{
		TMPrepare(stub, engineerId);
	}

	else if (IDMsg.compare("TMCommit") == 0)
	{
		TMCommit(stub, engineerId);
	}

	else if (IDMsg.compare("TMAbort") == 0)
	{
		TMAbort(stub, engineerId);
	}

	else if  (IDMsg.compare("PrPrepare") == 0)
	{
       PeerPrepare(stub, engineerId);
	}

}

void LaptopFactory::PeerPrepare(ServerStub &stub, int engineerId)
{

    //std::cout << "in rm backup prepare 1" << std::endl;
	Transaction txn = stub.ReceiveTxnRequest();
	//std::cout << "in rm backup prepare 2" << std::endl;
    store_txn_log_in_txn_map(txn);
	stub.SendResponseReplicationReq((char *)"Ok");
   //std::cout << "in rm backup prepare send ok" << std::endl;

	ReceiveIDMsgAndInitiateFlow(stub, engineerId);

}

void LaptopFactory::CopyTranlogReplica(Transaction txn){
 
     for (int i = 0; i < peer_count; i++)
			{	   
             if (peerNodes[i].Init(peers[i].ip, peers[i].port) == 1)
				{
					// send identification message to only those that succeed in connecting
					// std::cout << "succ.. sending id msg\n";
					
						//std::cout << "before sendIDMsg\n";
						peerNodes[i].SendIDMsg("PrPrepare");
						livePeers[i] = true;
						//std::cout << "before sendTxn... \n";
                    	std::string response = peerNodes[i].SendTxnPrepareRequest(txn);
				    	//std::cout<<"response from rm backup's: "<<response<<std::endl;
					
				}
			}	
    
}

void LaptopFactory::TMPrepare(ServerStub &stub, int engineerId)
{
	//std::cout << "in rm prepare 1" << std::endl;
	Transaction txn = stub.ReceiveTxnRequest();
	//std::cout << "in rm prepare 2" << std::endl;
	bool allOk = true;
	for (auto &record : txn.getReadRecords())
	{
		if (customer_record.find(record.getCustomerId()) != customer_record.end() && record.getLastOrder() != customer_record[record.getCustomerId()])
		{
			allOk = false;
			break;
		}
	}
	if (allOk)
	{
		store_txn_log_in_txn_map(txn);
        
		//changes for the replication of txn log starts here
        //send transaction log to the rm backups
			CopyTranlogReplica(txn);
     
        //changes for the replication of txn log ends here 

		stub.SendResponseReplicationReq((char *)"Ok");
		//std::cout << "in rm prepare send ok" << std::endl;
	}
	else
	{
		stub.SendResponseReplicationReq((char *)"No");
		//std::cout << "in rm prepare send No" << std::endl;
	}
	//std::cout << "done TMPrepare\n";
	ReceiveIDMsgAndInitiateFlow(stub, engineerId);
}

int LaptopFactory::store_txn_log_in_txn_map(Transaction txn)
{
	std::vector<TxnLogRequest> txnRecords;
	txn_log_mutex.lock();
	for (auto &order : txn.getWriteRecords())
	{
		if (!order.IsValid())
		{
			break;
		}

		TxnLogRequest rec = {txn.getTxnId(), 1, order.GetCustomerId(), order.GetOrderNumber(), order.GetRequestType()};
		txnRecords.push_back(rec);
	}
	txn_log.insert(std::pair<int, vector<TxnLogRequest> >(txn.getTxnId(), txnRecords));
	txn_log_mutex.unlock();
	store_txn_log_record(std::ref(txnRecords));

	return 1;
}

void LaptopFactory::TMCommit(ServerStub &stub, int engineerId)
{
	int txnidtocommit = stub.ReceiveTxnId();
	//std::cout<<"txnidtocommit: "<<txnidtocommit<<std::endl;
	std::vector<TxnLogRequest> txnRecords;
	//add commmit status transaction records in the txn_log and persistent
	std::vector<TxnLogRequest> txnRecordsCommitted;

	//std::cout << "in rm TMCommit 1" << std::endl;

	std::map<int, std::vector<TxnLogRequest> > new_txn_log;

	auto it = txn_log.find(txnidtocommit);
	// pre-c++11: std::map<vector<int>, vector<int> >::iterator it = info.find(key)
	if (it != txn_log.end())
	{
		CustomerOrder order;
		for (auto &txnRec : it->second)
		{
			txn_log_mutex.lock();

			// std::cout << txnRec.txn_id // string (key)
			// 		  << ':'
			// 		  << txnRec.txn_operation // string's value
			// 		  << ':'
			// 		  << txnRec.customer_id
			// 		  << ':'
			// 		  << txnRec.last_order_num
			// 		  << ':'
			// 		  << txnRec.customer_opr
			// 		  << std::endl;
			int result_new = update_value(std::ref(new_txn_log), txnRec.txn_id, txnRec);
			//std::cout << "result_new in commit " << result_new << std::endl;
			order.SetOrder(txnRec.customer_id, txnRec.last_order_num, txnRec.customer_opr);

			TxnLogRequest rec = {txnidtocommit, 2, txnRec.customer_id, txnRec.last_order_num, txnRec.customer_opr};
			txnRecordsCommitted.push_back(rec);
			int result = update_value(std::ref(new_txn_log), txnRec.txn_id, rec);
			//std::cout << "result in commit " << result << std::endl;
			txn_log_mutex.unlock();
			LaptopInfo laptop = CreateRegularLaptop(order, engineerId);
		}
		//std::cout << "in rm TMCommit 0" << std::endl;
	}

	//std::cout << "read in commit" << std::endl;
	std::vector<TxnLogRequest> txnRecordsRead;

	auto itr = new_txn_log.find(txnidtocommit);
	// if (itr != new_txn_log.end())
	// {

	// 	txnRecordsRead = itr->second;
	// 	for (auto &tx : txnRecordsRead)
	// 	{
	// 		std::cout << tx.txn_id // string (key)
	// 				  << ':'
	// 				  << tx.txn_operation // string's value
	// 				  << ':'
	// 				  << tx.customer_id
	// 				  << ':'
	// 				  << tx.last_order_num
	// 				  << ':'
	// 				  << tx.customer_opr
	// 				  << std::endl;
	// 	}
	// }

	txn_log = new_txn_log;

	//std::cout << "read txn_log in commit" << std::endl;
	auto itre = txn_log.find(txnidtocommit);
	// if (itre != txn_log.end())
	// {

	// 	txnRecordsRead = itre->second;
	// 	for (auto &tx : txnRecordsRead)
	// 	{
	// 		std::cout << tx.txn_id // string (key)
	// 				  << ':'
	// 				  << tx.txn_operation // string's value
	// 				  << ':'
	// 				  << tx.customer_id
	// 				  << ':'
	// 				  << tx.last_order_num
	// 				  << ':'
	// 				  << tx.customer_opr
	// 				  << std::endl;
	// 	}
	// }

	store_txn_log_record(std::ref(txnRecordsCommitted));

	//  apply transaction log to customer record and then persist in file and then empty the log file
	//apply_txn_cust_log();
	store_cust_record_txn();
	//empty_log_file();

	//std::cout << "in rm TMCommit 1" << std::endl;

	stub.SendResponseReplicationReq((char *)"Done commit");

	//std::cout << "done TMCommit\n";
	ReceiveIDMsgAndInitiateFlow(stub, engineerId);
}

int LaptopFactory::update_value(std::map<int, std::vector<TxnLogRequest> > &d, int key, TxnLogRequest new_val)
{
	// std::map<string, std::vector<TxnLogRequest>>::iterator it = d.find(key);
	auto it = d.find(key);
	//std::cout << "key is" << key << std::endl;

	if (it != d.end())
	{
		it->second.push_back(new_val);
	}
	else
	{
		std::vector<TxnLogRequest> v;
		v.push_back(new_val);
		d.insert(make_pair(key, v));
		return 2;
	}
	return 1;
}

void LaptopFactory::TMAbort(ServerStub &stub, int engineerId)
{
	//std::cout << "in rm TMAbort 0" << endl;
	//Transaction txn = stub.ReceiveTxnRequest();
	int txnidtoabort = stub.ReceiveTxnId();
	//std::cout<<"txnidtoabort: "<<txnidtoabort<<std::endl;
	//	txn_write_file_mutex.lock();
	//	txn_log.insert(std::pair<int, int>(txn.getTxnId(), 2));
	//  store_txn_log_record(txn.getTxnId(),2);
	//  txn_write_file_mutex.unlock();

	std::vector<TxnLogRequest> txnRecords;
	TxnLogRequest rec = {txnidtoabort, 3, -1, -1, -1};
	txn_log_mutex.lock();
	//txnlogRecords.push_back(rec);
	int result = update_value(std::ref(txn_log), txnidtoabort, rec);
	//std::cout << "result in TMAbort " << result << std::endl;
	txnRecords.push_back(rec);
	txn_log_mutex.unlock();
	store_txn_log_record(std::ref(txnRecords));
	//std::cout << "in rm TMAbort 1" << std::endl;
	stub.SendResponseReplicationReq((char *)"Done abort");
	//std::cout << "done TMAbort\n";
	ReceiveIDMsgAndInitiateFlow(stub, engineerId);
}

void LaptopFactory::EngineerFlow(ServerStub &stub, int id)
{
	CustomerOrder order;
	int request_type;
	LaptopInfo laptop;
	int engineer_id = id;
	while (true)
	{
		order = stub.ReceiveRequest();
		if (!order.IsValid())
		{
			break;
		}
		request_type = order.GetRequestType();
		switch (request_type)
		{
		case 1:
		{
			laptop = CreateRegularLaptop(order, engineer_id);
			stub.SendLaptop(laptop);
			break;
		}
		case 2:
		{
			CustomerRecord customerRecord;
			curstomer_record_mutex.lock();
			if (customer_record.find(order.GetCustomerId()) != customer_record.end())
			{
				customerRecord.setCustomerId(order.GetCustomerId());
				customerRecord.setLastOrder(customer_record[order.GetCustomerId()]);
			}
			curstomer_record_mutex.unlock();
			stub.ReturnRecord(customerRecord);
			break;
		}
		default:
		{
			std::cout << "Undefined laptop type: "
					  << request_type << std::endl;
		}
		}
	}
}

void LaptopFactory::ReadEngineerFlow(ServerStub &stub)
{
	CustomerOrder order;
	int request_type;
	LaptopInfo laptop;
	while (true)
	{
		order = stub.ReceiveRequest();
		//std::cout << "recevd request: ";
		//order.Print();
		//std::cout << std::endl;
		if (!order.IsValid())
		{
			break;
		}
		request_type = order.GetRequestType();
		switch (request_type)
		{
		case 2:
		{
			CustomerRecord customerRecord;
			curstomer_record_mutex.lock();
			if (customer_record.find(order.GetCustomerId()) != customer_record.end())
			{
				customerRecord.setCustomerId(order.GetCustomerId());
				customerRecord.setLastOrder(customer_record[order.GetCustomerId()]);
			}
			curstomer_record_mutex.unlock();
			stub.ReturnRecord(customerRecord);
			break;
		}
		default:
		{
			std::cout << "Undefined laptop type: "
					  << request_type << std::endl;
		}
		}
	}
}

void LaptopFactory::IFAFlow(ServerStub &stub, int id)
{
	while (true)
	{
		ReplicationRequest req = stub.ReceiveReplicationReq();
		// std::cout<<"recvd rep reqst \n";
		// req.print();

		if (req.GetFactoryId() == -1)
		{
			// Primary has failed, close this thread
			primary_id = -1;
			break;
		}

		primary_id = req.GetFactoryId();
		smr_log.push_back(req.getMapOp());
		last_index++;

		if (req.GetCommittedIndex() >= 0)
		{
			// std::cout<<"ifa: " << req.GetCommittedIndex() << "\n";
			// printIndices();
			MapOp lastCommitted = smr_log.at(req.GetCommittedIndex());
			curstomer_record_mutex.lock();
			customer_record[lastCommitted.arg1] = lastCommitted.arg2;
			curstomer_record_mutex.unlock();
			committed_index++;
		}

		stub.SendResponseReplicationReq((char *)"DONE");
	}
}

void LaptopFactory::AdminThread(int id)
{
	std::unique_lock<std::mutex> ul(arq_lock, std::defer_lock);
	while (true)
	{
		ul.lock();

		if (arq.empty())
		{
			arq_cv.wait(ul, [this]
						{ return !arq.empty(); });
		}

		auto req = std::move(arq.front());
		arq.pop();

		ul.unlock();

		// Mark yourself as primary id by setting to primary_id to your own factory_id if not set
		// and establish connection to all backup nodes
		if (primary_id != factory_id)
		{
			// std::cout << "Making myself primary\n";
			primary_id = factory_id;

			for (int i = 0; i < peer_count; i++)
			{
				if (peerNodes[i].Init(peers[i].ip, peers[i].port) == 1)
				{
					// send identification message to only those that succeed in connecting
					// std::cout << "succ.. sending id msg\n";
					peerNodes[i].SendIDMsg("PFA");
					livePeers[i] = true;
				}
			}

			// If this node became primary and was a backup earlier,
			// update uncomitted ops and update committed_id
			while (committed_index < last_index)
			{
				updateCustomerRecord(smr_log[last_index]);
			}
		}

		// Append request to own log and update last_index
		MapOp mapOp;
		mapOp.opcode = 1;
		mapOp.arg1 = req->customer_id;
		mapOp.arg2 = req->last_order_num;
		smr_log.push_back(mapOp);
		last_index++;

		
		curstomer_record_mutex.lock();
		customer_record[mapOp.arg1] = mapOp.arg2;
		curstomer_record_mutex.unlock();
		committed_index++;

		// Replication protocol here
		//std::cout << "sending rep request\n";
		for (int i = 0; i < peer_count; i++)
		{
			if (livePeers[i] == false)
			{
				// std::cout<< i << " is failed peer, try to recon and send rep\n";
				// Failed backup, try to reconnect
				if (peerNodes[i].Init(peers[i].ip, peers[i].port) == 1)
				{
					peerNodes[i].SendIDMsg("PFA");
					livePeers[i] = true;
					// std::cout<<"live now, sending all:\n";
					// Send all replication requests
					sendAllRequests(i);
				}
			}
			else
			{
				// std::cout<< i << " is live peer, sending normal rep request\n";
				ReplicationRequest replicationReq(factory_id, committed_index, last_index, mapOp);
				std::string resp = peerNodes[i].SendReplicationReq(replicationReq);
				// std::cout << "resp back: " << resp << std::endl;
				if (resp == "FAILED")
				{
					livePeers[i] = false;
				}
			}
		}
		
		// Replication protocol ends here

		req->laptop.SetAdminId(id);
		req->prom.set_value(req->laptop);

		// printIndices();
	}
}

void LaptopFactory::updateCustomerRecord(MapOp oprn)
{
	curstomer_record_mutex.lock();
	customer_record[oprn.arg1] = oprn.arg2;
	curstomer_record_mutex.unlock();
	committed_index++;
}

void LaptopFactory::printIndices()
{
	std::cout << "Committed idx: " << committed_index << " Last Index: "
			  << last_index << std::endl;
}

void LaptopFactory::sendAllRequests(int peer_id)
{
	// std::cout<<"in all:\n";
	for (int i = 0; i < smr_log.size(); i++)
	{
		ReplicationRequest replicationReq(factory_id, i - 1, i, smr_log[i]);
		// replicationReq.print();
		std::string resp = peerNodes[peer_id].SendReplicationReq(replicationReq);
		// std::cout << "resp back: " << resp << std::endl;
	}
}

/*
int LaptopFactory::apply_txn_cust_log(){

	curstomer_record_mutex.lock();

	for (auto& tx : txnlogRecords)
	{

		std::cout << tx.txn_id // string (key)
				  << ':'
				  << tx.txn_operation // string's value
                  << ':'
				  << tx.customer_id 
                  << ':'
				  << tx.last_order_num 
                   << ':'
				  << tx.customer_opr 
				  << std::endl;
	
		if (customer_record.find(tx.customer_id) != customer_record.end()
					&& tx.last_order_num != customer_record[tx.customer_id] 
					//&& (tx.last_order_num > customer_record[tx.customer_id])
					&& tx.txn_operation == 1 && tx.customer_opr == 1
					) {
						
		customer_record[tx.customer_id] = tx.last_order_num;
						
		}
	}

	curstomer_record_mutex.unlock();
	return 1;
} */

void LaptopFactory::store_cust_record_txn()
{

	cust_write_file_mutex.lock();
	ofstream file;
	file.open("custRecFile.txt");

	if (!file)
	{
		std::cerr << "unable to open the file\n"
				  << std::endl;
		exit(1);
	}
	else
	{
		for (auto const &x : customer_record)
		{
			// std::cout << x.first // string (key)
			// 		  << ':'
			// 		  << x.second // string's value
			// 		  << std::endl;
			file << x.first << ' ' << x.second << "\n"; //write to the file
		}
	}

	file.close();
	cust_write_file_mutex.unlock();

	//empty the logFile.txt file
	return;
}

int LaptopFactory::empty_log_file()
{
	std::ofstream ofs;
	ofs.open("logFile.txt", std::ofstream::out | std::ofstream::trunc);
	ofs.close();
	return 1;
}

void LaptopFactory::store_txn_log_record(std::vector<TxnLogRequest> &txnLogReq)
{
	//std::cout<< "id " << id << "status" << status << std::endl;
	txn_write_file_mutex.lock();
	ofstream file;
	file.open("logFile.txt", std::ios_base::app);

	if (!file)
	{
		std::cerr << "unable to open the file\n"
				  << std::endl;
		exit(1);
	}
	else
	{

		for (auto &rec : txnLogReq)
		{
			// std::cout << rec.txn_id << ' ' << rec.txn_operation << ' ' << rec.customer_id << ' ' << rec.last_order_num << ' ' << rec.customer_opr << "\n"
			// 		  << std::endl;
			file << rec.txn_id << ' ' << rec.txn_operation << ' ' << rec.customer_id << ' ' << rec.last_order_num << ' ' << rec.customer_opr << "\n"; //write to the file
		}
	}

	file.close();
	txn_write_file_mutex.unlock();

	return;
}

int LaptopFactory::recovery_txn_log()
{
	ifstream fin;
	fin.open("logFile.txt");
	if (!fin)
	{
		cerr << "Error in opening the file" << endl;
		return 1; // if this is main
	}

	int txn_id;
	int operation;
	int cust_id;
	int last_index;
	int customer_opr;
	while (fin >> txn_id >> operation >> cust_id >> last_index >> customer_opr)
	{

		TxnLogRequest rec = {txn_id, operation, cust_id, last_index, customer_opr};

		// txnlogRecords.push_back(rec);

		int result = update_value(std::ref(txn_log), rec.txn_id, rec);

		//std::cout << "result in recovery_txn_log " << result << std::endl;
	}

	//  print the information you read in

	// for (auto &x : txn_log)
	// {

	// 	for (auto tx : x.second)
	// 	{
	// 		std::cout << tx.txn_id // string (key)
	// 				  << ':'
	// 				  << tx.txn_operation // string's value
	// 				  << ':'
	// 				  << tx.customer_id
	// 				  << ':'
	// 				  << tx.last_order_num
	// 				  << ':'
	// 				  << tx.customer_opr
	// 				  << std::endl;
	// 	}
	// }

	fin.close();
}
int LaptopFactory::recovery_cust_rec()
{

	ifstream fin;
	fin.open("custRecFile.txt");
	if (!fin)
	{
		cerr << "Error in opening the file" << endl;
		return 1; // if this is main
	}

	int cust_id;
	int last_index;
	while (fin >> cust_id >> last_index)
	{
		customer_record.insert(std::pair<int, int>(cust_id, last_index));
	}

	//  print the information you read in

	// for (auto const &x : customer_record)
	// {
	// 	std::cout << x.first // string (key)
	// 			  << ':'
	// 			  << x.second // string's value
	// 			  << std::endl;
	// }

	fin.close();
}
