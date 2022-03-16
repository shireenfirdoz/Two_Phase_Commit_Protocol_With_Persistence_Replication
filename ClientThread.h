#ifndef __CLIENT_THREAD_H__
#define __CLIENT_THREAD_H__

#include <chrono>
#include <ctime>
#include <string>
#include <mutex>
#include <vector>

#include "ClientStub.h"
#include "ClientTimer.h"
#include "Messages.h"

class ClientThreadClass {
	int customer_id;
	int num_orders;
	int request_type;
	ClientStub stub;

	ClientTimer timer;
public:
	ClientThreadClass();
	void ThreadBody(std::string ip, int port, int id, int orders, int type,
					std::vector<CustomerRecord> &a,
					std::mutex &b,
					std::vector<CustomerOrder> &c);
	void ThreadBodyWriteRequest(std::string ip, int port, int id, int orders, int type,
					std::vector<CustomerRecord> &a,
					std::mutex &b,
					std::vector<CustomerOrder> &c);
	void ThreadBodyReadRequestSingle(std::string ip, int port, int id, int orders, int type,
									std::vector<CustomerRecord> &readRecords,
									std::mutex &writeRecordLock,
									std::vector<CustomerOrder> &writeRecords);

	ClientTimer GetTimer();
};


#endif // end of #ifndef __CLIENT_THREAD_H__
