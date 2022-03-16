#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "Messages.h"
#include "Socket.h"
#include "ServerStub.h"
#include "ServerSocket.h"
#include "ClientStub.h"
#include "ClientSocket.h"

int transaction_id = 0;

void transaction_manager(std::unique_ptr<ServerSocket> socket, int rms_count,
                            std::vector<peer_servers_info> rms,
                            std::vector<ClientStub> rmNodes) {
	// std::cout << "rms size: " << rms.size() << "rmNodes size: " << rmNodes.size()
	// 		  << "rms_count: " << rms_count << std::endl;
    bool allOk = true;
    ServerStub stub;
	stub.Init(std::move(socket));
    Transaction txn = stub.ReceiveTxnRequest();
    txn.setTxnId(transaction_id++);
	//txn.print();
    for (int i = 0; i < rms_count; i++) {
        rmNodes[i] = ClientStub();
        if (rmNodes[i].Init(rms[i].ip, rms[i].port) == 0) {
            if (rmNodes[i].Init(rms[i].backup_ip, rms[i].backup_port) == 0) {
                stub.SendResponseReplicationReq("Failed");
                return;
            }
        }
    }
    for (int i = 0; i < rms_count; i++) {
        //send Txn prepare to all rms
		//std::cout << "before sendIDMsg\n";
        rmNodes[i].SendIDMsg("TMPrepare");
		//std::cout << "before sendTxn... \n";
        std::string response = rmNodes[i].SendTxnPrepareRequest(txn);
        //std::cout<<"response from rm: "<<response<<std::endl;
        if (response == "No") {
            allOk = false;
            break;
        }
        //get response, if any of them says No, abort, if OK continue processing
    }
    // if all are OK, send commit msg, else abort msg
    for (int i = 0; i < rms_count; i++) {
        if (allOk) {
            rmNodes[i].SendIDMsg("TMCommit");
        }
        else {
            rmNodes[i].SendIDMsg("TMAbort");
        }
        // std::cout << "response from rms after sending commit/abort: " << 
        rmNodes[i].SendTxnId(txn.getTxnId());
    }
    if (allOk) {
      stub.SendResponseReplicationReq("Success");
    }
    else {
      stub.SendResponseReplicationReq("Failed");
    }
}

int main(int argc, char *argv[]) {
	int port;
	int rms_count;
	ServerSocket socket;
	std::unique_ptr<ServerSocket> new_socket;
	std::vector<std::thread> thread_vector;

	if (argc < 4) {
		std::cout << "not enough arguments" << std::endl;
		return 0;
	}
	port = atoi(argv[1]);
	rms_count = atoi(argv[2]);
	int arg_index = 3;
	std::vector<peer_servers_info> rms;
    std::vector<ClientStub> rmNodes;
    for (int i = 0; i < rms_count; i++) {
        peer_servers_info peer;
        peer.id = atoi(argv[arg_index++]);
        peer.ip = argv[arg_index++];
        peer.port = atoi(argv[arg_index++]);
        peer.backup_ip = argv[arg_index++];
        peer.backup_port = atoi(argv[arg_index++]);
        rms.push_back(peer);
        rmNodes.push_back(ClientStub());
    }

	if (!socket.Init(port)) {
		std::cout << "Socket initialization failed" << std::endl;
		return 0;
	}

	while ((new_socket = socket.Accept())) {
		transaction_manager(std::move(new_socket), rms_count, rms, rmNodes);
	}
	return 0;
}
