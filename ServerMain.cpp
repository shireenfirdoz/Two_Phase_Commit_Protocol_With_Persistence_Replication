#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "ServerSocket.h"
#include "ServerThread.h"

int main(int argc, char *argv[]) {
	int port;
	int unique_id;
	int peer_count;
	int engineer_cnt = 0;
	ServerSocket socket;
	std::unique_ptr<ServerSocket> new_socket;
	std::vector<std::thread> thread_vector;

	if (argc < 4) {
		std::cout << "not enough arguments" << std::endl;
		// std::cout << argv[0] << "[port #] [# experts]" << std::endl;
		return 0;
	}
	port = atoi(argv[1]);
	unique_id = atoi(argv[2]);
	peer_count = atoi(argv[3]);
	int arg_index = 4;
	std::vector<peer_servers_info> peers;
	for (int i = 0; i < peer_count; i++) {
		peer_servers_info peer;
		peer.id = atoi(argv[arg_index++]);
		peer.ip = argv[arg_index++];
		peer.port = atoi(argv[arg_index++]);
		peers.push_back(peer);
	}

	LaptopFactory factory(peers, peer_count);
	factory.setFactoryId(unique_id);

	std::thread admin_thread(&LaptopFactory::AdminThread,
			&factory, engineer_cnt++);
	thread_vector.push_back(std::move(admin_thread));

	if (!socket.Init(port)) {
		std::cout << "Socket initialization failed" << std::endl;
		return 0;
	}

	while ((new_socket = socket.Accept())) {
		// std::cout<< "creating new engIFA thread\n";
		std::thread engineer_thread(&LaptopFactory::EngineerIFAThread,
				&factory, std::move(new_socket),
				engineer_cnt++);
		thread_vector.push_back(std::move(engineer_thread));
	}
	return 0;
}
