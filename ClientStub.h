#ifndef __CLIENT_STUB_H__
#define __CLIENT_STUB_H__

#include <string>

#include "ClientSocket.h"
#include "Messages.h"

class ClientStub {
private:
	ClientSocket socket;
public:
	ClientStub();
	int Init(std::string ip, int port);
	LaptopInfo Order(CustomerOrder order);
	CustomerRecord ReadRecord(CustomerOrder order);
	int SendIDMsg(std::string msg);
	std::string SendReplicationReq(ReplicationRequest req);
	std::string SendTxnPrepareRequest(Transaction txn);
	std::string SendTxnId(int id);
};


#endif // end of #ifndef __CLIENT_STUB_H__
