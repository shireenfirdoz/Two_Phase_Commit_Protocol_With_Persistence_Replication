#include <array>
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>

#include "ClientSocket.h"
#include "ClientThread.h"
#include "ClientTimer.h"
#include "Messages.h"
#include "ClientStub.h"

int main(int argc, char *argv[]) {
	std::string ip;
	int port;
	int num_customers;
	int num_orders;
	int request_type;
	ClientTimer timer;

	std::vector<CustomerRecord> readRecords; 
	std::mutex writeRecordLock;
	std::vector<CustomerOrder> writeRecords;

	{
		std::vector<std::shared_ptr<ClientThreadClass>> client_vector;
		std::vector<std::thread> thread_vector;

		//1st read requests
		std::cin >> request_type >> ip >> port >> num_customers >> num_orders;

		//std::cout << request_type << " " << ip << " " << port << " " << num_customers << " " << num_orders << std::endl; 

		timer.Start();
		for (int i = 0; i < num_customers; i++) {
			auto client_cls = std::shared_ptr<ClientThreadClass>(new ClientThreadClass());
			std::thread client_thread(&ClientThreadClass::ThreadBody, client_cls,
					ip, port, i, num_orders, request_type,
					std::ref(readRecords),
					std::ref(writeRecordLock), 
					std::ref(writeRecords));

			client_vector.push_back(std::move(client_cls));
			thread_vector.push_back(std::move(client_thread));
		}
		for (auto& th : thread_vector) {
			th.join();
		}
		timer.End();

		for (auto& cls : client_vector) {
			timer.Merge(cls->GetTimer());
		}
		timer.PrintStats();
	}

	{
		std::vector<std::shared_ptr<ClientThreadClass>> client_vector;
		std::vector<std::thread> thread_vector;

		//2nd write requests
		std::cin >> request_type >> ip >> port >> num_customers >> num_orders;

		//std::cout << request_type << " " << ip << " " << port << " " << num_customers << " " << num_orders << std::endl; 

		timer.Start();
		for (int i = 0; i < num_customers; i++) {
			auto client_cls = std::shared_ptr<ClientThreadClass>(new ClientThreadClass());
			std::thread client_thread(&ClientThreadClass::ThreadBodyWriteRequest, client_cls,
					ip, port, i, num_orders, request_type,
					std::ref(readRecords),
					std::ref(writeRecordLock), 
					std::ref(writeRecords));

			client_vector.push_back(std::move(client_cls));
			thread_vector.push_back(std::move(client_thread));
		}
		for (auto& th : thread_vector) {
			th.join();
		}
		timer.End();

		for (auto& cls : client_vector) {
			timer.Merge(cls->GetTimer());
		}
		timer.PrintStats();
	}

	Transaction txn(readRecords, writeRecords);
	//txn.print();
	ClientStub clientStub;
	clientStub.Init(ip, port);
	//std::cout << "sending txn prepare" << std::endl;
	txn.setLenReadRecords(readRecords.size());
	txn.setLenWriteRecords(writeRecords.size());
	std::cout << "Transaction result and stats: \n";
	//std::cout<<"read size: "<<readRecords.size()<<std::endl;
	//std::cout<<"write size: "<<writeRecords.size()<<std::endl;
	timer.Start();
	std::cout<< clientStub.SendTxnPrepareRequest(txn) << std::endl;
	timer.End();
	timer.PrintStats();
	return 1;
}
