#include "ClientThread.h"
#include "Messages.h"
#include <thread>
#include <iostream>

ClientThreadClass::ClientThreadClass() {}

void ClientThreadClass::ThreadBody(std::string ip, int port, int id, int orders, int type,
									std::vector<CustomerRecord> &readRecords,
									std::mutex &writeRecordLock,
									std::vector<CustomerOrder> &writeRecords) {
	customer_id = id;
	num_orders = orders;
	request_type = type;
	if (!stub.Init(ip, port)) {
		std::cout << "Thread " << customer_id << " failed to connect" << std::endl;
		return;
	}
	std::string msg = "CLIENT";
	if (stub.SendIDMsg(msg) <= 0) {
		std::cout << "Error sending id msg\n";
		return;
	}
	for (int i = 0; i < num_orders; i++) {
		CustomerOrder order;
		LaptopInfo laptop;
		CustomerRecord record;

		timer.Start();
		if (request_type == 3) {
			order.SetOrder(i, -1, 2);
			record = stub.ReadRecord(order);
			// save read record for transaction, no need for lock as there will only be 1 thread
			readRecords.push_back(record);
			if (record.getCustomerId() == -1) {
				break;
			}
			record.print();
		}
		else {
			std::cout << "Invalid request type " << request_type << std::endl;
			break;
		}
		timer.EndAndMerge();
	}
}

void ClientThreadClass::ThreadBodyWriteRequest(std::string ip, int port, int id, int orders, int type,
									std::vector<CustomerRecord> &readRecords,
									std::mutex &writeRecordLock,
									std::vector<CustomerOrder> &writeRecords) {
	customer_id = id;
	num_orders = orders;
	request_type = type;

	int setLastOrder = 0;
	if (customer_id < readRecords.size() && readRecords[customer_id].getLastOrder() > 0) {
		int temp = readRecords[customer_id].getLastOrder();
	
        setLastOrder = 	temp;
	
	}
	//std::cout<< "cust id: " << customer_id << " last order: "<<setLastOrder<<std::endl;

	for (int i = 1; i <= num_orders; i++) {
		CustomerOrder order;
		timer.Start();

		order.SetOrder(customer_id, i+setLastOrder, request_type);
		// save write order for transaction
		// need to execute atomically as multiple threads may try to write
		writeRecordLock.lock();
		writeRecords.push_back(order);
		writeRecordLock.unlock();
		
		timer.EndAndMerge();
	}
}

void ClientThreadClass::ThreadBodyReadRequestSingle(std::string ip, int port, int id, int orders, int type,
									std::vector<CustomerRecord> &readRecords,
									std::mutex &writeRecordLock,
									std::vector<CustomerOrder> &writeRecords) {
	customer_id = id;
	num_orders = orders;
	request_type = type;
	if (!stub.Init(ip, port)) {
		std::cout << "Thread " << customer_id << " failed to connect" << std::endl;
		return;
	}
	std::string msg = "CLIENT";
	if (stub.SendIDMsg(msg) <= 0) {
		std::cout << "Error sending id msg\n";
		return;
	}
	CustomerOrder order;
	LaptopInfo laptop;
	CustomerRecord record;

	timer.Start();
	if (request_type == 3) {
		order.SetOrder(num_orders, -1, 2);
		record = stub.ReadRecord(order);
		// save read record for transaction, no need for lock as there will only be 1 thread
		readRecords.push_back(record);
		//record.print();
	}
	else {
		std::cout << "Invalid request type " << request_type << std::endl;
	}
	timer.EndAndMerge();
}

ClientTimer ClientThreadClass::GetTimer() {
	return timer;
}
