#include "ServerStub.h"
#include <iostream>

ServerStub::ServerStub() {}

void ServerStub::Init(std::unique_ptr<ServerSocket> socket) {
	this->socket = std::move(socket);
}

CustomerOrder ServerStub::ReceiveRequest() {
	void *marshalledOrder = new char[sizeof(CustomerOrder)];
	CustomerOrder *order = NULL;
	if (socket->Recv(marshalledOrder, sizeof(CustomerOrder)) > 0) {
		order = static_cast<CustomerOrder *>(marshalledOrder);
		return *order;
	}
	// indicate close of connection by client by sending empty order to engineer thread
	return CustomerOrder();	
}

int ServerStub::SendLaptop(LaptopInfo info) {
	void *marshalledLaptop = &info;
	return socket->Send(marshalledLaptop, sizeof(LaptopInfo));
}

int ServerStub::ReturnRecord(CustomerRecord record) {
	void *marshalledRecord = &record;
	return socket->Send(marshalledRecord, sizeof(CustomerRecord));
}

std::string ServerStub::IdentificationMsg() {
	char message[10];
	socket->Recv(message, 10);
	return message;
}

ReplicationRequest ServerStub::ReceiveReplicationReq() {
	void *marshalledRepReq = new char[sizeof(ReplicationRequest)];
	ReplicationRequest *req;
	if (socket->Recv(marshalledRepReq, sizeof(ReplicationRequest)) > 0) {
		req = static_cast<ReplicationRequest *>(marshalledRepReq);
		return *req;
	}
	// indicate close of connection by PFA by sending empty order to engineer thread
	return ReplicationRequest();	
}

// Called SendResponseReplicationReq, but is a generic method used to send a small string
// over the socket
int ServerStub::SendResponseReplicationReq(char *msg) {
	return socket->Send(msg, 10, 0);
} 

Transaction ServerStub::ReceiveTxnRequest() {
	//std::cout<<"in ReceiveTxnRequest\n";
	char* buffer = (char *) malloc(200000);
	if(!buffer) {  
		std::cout<<"Malloc failed\n";
		return Transaction();
	}
	Transaction req;
	int recevdbytes = socket->Recv((void *)buffer, 200000);
	if (recevdbytes > 0) {
		//req = static_cast<Transaction *>(marshalledTxnReq);
		//std::cout<<"recevdbytes: "<<recevdbytes<<"\n";
		req.Unmarshal(buffer);
		return req;
	}
	std::cout<<"error in recv txn request\n";
}

int ServerStub::ReceiveTxnId() {
	return socket->readInt();
}