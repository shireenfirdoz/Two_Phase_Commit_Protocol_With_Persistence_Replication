#include <array>
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <random>

#include "ClientSocket.h"
#include "ClientThread.h"
#include "Messages.h"
#include "ClientStub.h"

void experiment(std::string tmip,
                int tmport,
                std::string rmip,
                int rmport,
                int num_customers,
                int num_orders,
				int curr_customer,
				int &output) {


	std::vector<CustomerRecord> readRecords; 
	std::mutex writeRecordLock;
	std::vector<CustomerOrder> writeRecords;

	//std::cout<<"chosen curr: " << curr_customer <<std::endl;

	{
		std::vector<std::shared_ptr<ClientThreadClass>> client_vector;
		std::vector<std::thread> thread_vector;

		//1st read requests
		//std::cin >> request_type >> ip >> port >> num_customers >> num_orders;

		//std::cout << 3 << " " << rmip << " " << rmport << " " << num_customers << " " << num_orders << std::endl; 

        auto client_cls = std::shared_ptr<ClientThreadClass>(new ClientThreadClass());
        std::thread client_thread(&ClientThreadClass::ThreadBodyReadRequestSingle, client_cls,
                rmip, rmport, 1, curr_customer, 3,
                std::ref(readRecords),
                std::ref(writeRecordLock), 
                std::ref(writeRecords));

        client_vector.push_back(std::move(client_cls));
        thread_vector.push_back(std::move(client_thread));
		for (auto& th : thread_vector) {
			th.join();
		}
	}

	{
		std::vector<std::shared_ptr<ClientThreadClass>> client_vector;
		std::vector<std::thread> thread_vector;

		//2nd write requests
		//std::cin >> request_type >> ip >> port >> num_customers >> num_orders;

		//std::cout << 1 << " " << tmip << " " << tmport << " " << num_customers << " " << num_orders << std::endl; 
		
        auto client_cls = std::shared_ptr<ClientThreadClass>(new ClientThreadClass());
        std::thread client_thread(&ClientThreadClass::ThreadBodyWriteRequest, client_cls,
                tmip, tmport, curr_customer, num_orders, 1,
                std::ref(readRecords),
                std::ref(writeRecordLock), 
                std::ref(writeRecords));

        client_vector.push_back(std::move(client_cls));
        thread_vector.push_back(std::move(client_thread));
		
		for (auto& th : thread_vector) {
			th.join();
		}
	}

	Transaction txn(readRecords, writeRecords);
	//txn.print();
	ClientStub clientStub;
	clientStub.Init(tmip, tmport);
	//std::cout << "sending txn prepare" << std::endl;
	txn.setLenReadRecords(readRecords.size());
	txn.setLenWriteRecords(writeRecords.size());
	//std::cout<<"read size: "<<readRecords.size()<<std::endl;
	//std::cout<<"write size: "<<writeRecords.size()<<std::endl;
	std::string result = clientStub.SendTxnPrepareRequest(txn);
	//std::cout<<"result: " << result << std::endl;

	if(result.compare("Failed") == 0) {
		output = 0;
	}
	else {
		output = 1;
	}
}

int main(int argc, char *argv[]) {
    // int num_customers = atoi(argv[2]);
    // int num_orders = atoi(argv[3]);
	std::string tmip = argv[1];
	int tmport = atoi(argv[2]);
	std::string rmip = argv[3];
	int rmport = atoi(argv[4]);
	int num_threads = atoi(argv[5]);
	int num_customers = 5;
    int num_orders = 1;
	int aborts = 0;
	srand(time(NULL));
	for (int i = 0; i < 10; i++) {
		std::vector<std::thread> thread_vector;
		std::vector<int> results_vector(num_threads);
		for (int i = 0; i < num_threads; i++) {
			int curr_customer = rand() % num_customers;
			std::thread clienti(experiment, 
								tmip, 
								tmport, 
								rmip,
								rmport,
								num_customers,
								num_orders,
								curr_customer,
								std::ref(results_vector[i]));
			thread_vector.push_back(std::move(clienti));
		}
		for (auto &th : thread_vector) {
			th.join();
		}
		for (int &result : results_vector) {
			if (result == 0) {
				aborts++;
			}
		}
	}

	std::cout<<"Abort rate: " << (double) aborts / (10 * num_threads) * 100 << "%"<< std::endl;
	return 1;
}
