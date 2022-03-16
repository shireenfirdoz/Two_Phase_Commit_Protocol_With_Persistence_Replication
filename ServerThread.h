#ifndef __SERVERTHREAD_H__
#define __SERVERTHREAD_H__

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <map>

#include "Messages.h"
#include "ServerSocket.h"
#include "ClientStub.h"
#include "ServerStub.h"

struct AdminRequest {
	LaptopInfo laptop;
	int customer_id; // customer id of the customer who made the requst
	int last_order_num; // last order number of the customer
	std::promise<LaptopInfo> prom;
};


struct TxnLogRequest {
	int txn_id;
	int txn_operation;
	int customer_id; // customer id of the customer who made the requst
	int last_order_num; // last order number of the customer
	int customer_opr;
};

class LaptopFactory {
private:
	std::queue<std::unique_ptr<AdminRequest>> arq;
	std::mutex arq_lock;
	std::condition_variable arq_cv;

	std::map<int, int> customer_record;
	std::vector<MapOp> smr_log; // state machine replication log
	int last_index;
	int committed_index;
	int primary_id;
	int factory_id;
	std::vector<peer_servers_info> peers;
	int peer_count;
	std::vector<ClientStub> peerNodes;
	std::vector<bool> livePeers;

 //added for persistence
	std::map<int, std::vector<TxnLogRequest>> txn_log; 
	//std::vector<TxnLogRequest> txnlogRecords;
	std::mutex cust_write_file_mutex;
	std::mutex txn_write_file_mutex;
	std::mutex txn_log_mutex;
	int recovery_cust_rec();
	int recovery_txn_log();
	int store_txn_log_in_txn_map(Transaction txn);
	int update_value(std::map<int, std::vector<TxnLogRequest>>& d, int key, TxnLogRequest new_val);


	LaptopInfo CreateRegularLaptop(CustomerOrder order, int engineer_id);
	LaptopInfo CreateCustomLaptop(CustomerOrder order, int engineer_id);

	std::mutex curstomer_record_mutex;

	void TMPrepare(ServerStub &stub, int engineerId);

//added for RM configuration 
    void TMCommit(ServerStub &stub,int engineerId);
    void TMAbort(ServerStub &stub, int engineerId);
	void PeerPrepare(ServerStub &stub, int engineerId);
    void CopyTranlogReplica(Transaction txn);

public:
	LaptopFactory(std::vector<peer_servers_info> peers_input, int peer_count_input);
	void EngineerIFAThread(std::unique_ptr<ServerSocket> socket, int engineerId);
	void AdminThread(int id);
	void setFactoryId(int id);
	void updateCustomerRecord(MapOp oprn);
	void EngineerFlow(ServerStub &stub, int id);
	void IFAFlow(ServerStub &stub, int id);
	void ReceiveIDMsgAndInitiateFlow(ServerStub &stub, int engineerId);
	void printIndices();
	void sendAllRequests(int peer_id);

 //added for RM configuration and persistence  
    void ReadEngineerFlow(ServerStub &stub);
	void store_cust_record_txn();
	//int apply_txn_cust_log();
	void store_txn_log_record(std::vector<TxnLogRequest> &txnLogReq);
	int empty_log_file();

};

#endif // end of #ifndef __SERVERTHREAD_H__
