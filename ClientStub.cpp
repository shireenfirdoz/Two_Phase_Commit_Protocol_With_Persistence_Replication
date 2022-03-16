#include "ClientStub.h"
#include <iostream>

ClientStub::ClientStub() {}

int ClientStub::Init(std::string ip, int port) {
	return socket.Init(ip, port);
}

LaptopInfo ClientStub::Order(CustomerOrder order) {
	void *marshalledOrder = &order;
	void *marshalledLaptop = new char[sizeof(LaptopInfo)];
	LaptopInfo *laptop = NULL;
	LaptopInfo createdLaptop;
	if (socket.Send(marshalledOrder, sizeof(CustomerOrder))) {
		if (socket.Recv(marshalledLaptop, sizeof(LaptopInfo))) {
			laptop = static_cast<LaptopInfo *>(marshalledLaptop);
		}
	}
	if (laptop)
		createdLaptop = *laptop;
	return createdLaptop;
}

CustomerRecord ClientStub::ReadRecord(CustomerOrder order) {
	void *marshalledOrder = &order;
	void *marshalledRecord = new char[sizeof(CustomerRecord)];
	CustomerRecord *customerRecord = NULL;
	CustomerRecord returnedCustomerRec;
	if (socket.Send(marshalledOrder, sizeof(CustomerOrder))) {
		if (socket.Recv(marshalledRecord, sizeof(CustomerRecord))) {
			customerRecord = static_cast<CustomerRecord *>(marshalledRecord);
		}
	}
	if (customerRecord)
		returnedCustomerRec = *customerRecord;
	return returnedCustomerRec;
}

int ClientStub::SendIDMsg(std::string message) {
	return socket.Send(&message[0], 10, 0);
}

std::string ClientStub::SendReplicationReq(ReplicationRequest req) {
	void *marshalledReq = &req;
	char buf[10];
	// If the socket was never initialized, it was a failed backup node, so ignore
	// and send only if it was initialized
	// std::cout<<"is sock init: "<<socket.getIsInitialized() << std::endl;
	if (socket.getIsInitialized() && socket.Send(marshalledReq, sizeof(ReplicationRequest))) {
	//if (socket.Send(marshalledReq, sizeof(ReplicationRequest))) {
		if (socket.Recv(buf, 10, 0)) {
			return std::string(buf);
		}
	}
	return std::string("FAILED");
}

std::string ClientStub::SendTxnPrepareRequest(Transaction req) {
	//std::cout<<"in SendTxnPrepareRequest\n";
	char* marshalledReq = (char *) malloc(200000);
	if(!marshalledReq) {  
		std::cout<<"Malloc failed\n";
		return std::string("malloc failed");
	}
	char buf[10];
	if(!socket.getIsInitialized()) {
		std::cout<<"socket not initialized\n";
		return "error";
	}
	//std::cout<<"before marshal\n";
	req.Marshal(marshalledReq);
	//std::cout<<"after marshal\n";
	if (socket.Send(marshalledReq, req.Size())) {
		//std::cout<<"sent successfully\n";
		if (socket.Recv((void *)buf, 10)) {
			return std::string(buf);
		}
	}
	return std::string("ErrorNo");
}

std::string ClientStub::SendTxnId(int id) {
	char buf[10];
	if (socket.writeInt(id)) {
		if (socket.Recv((void *)buf, 10)) {
			return std::string(buf);
		}
	}
}
