#ifndef __SERVER_STUB_H__
#define __SERVER_STUB_H__

#include <memory>

#include "ServerSocket.h"
#include "Messages.h"

class ServerStub {
private:
	std::unique_ptr<ServerSocket> socket;
public:
	ServerStub();
	void Init(std::unique_ptr<ServerSocket> socket);
	CustomerOrder ReceiveRequest();
	int SendLaptop(LaptopInfo info);
	int ReturnRecord(CustomerRecord record);
	std::string IdentificationMsg();
	ReplicationRequest ReceiveReplicationReq();
	int SendResponseReplicationReq(char *msg);
	Transaction ReceiveTxnRequest();
	int ReceiveTxnId();
};

#endif // end of #ifndef __SERVER_STUB_H__
