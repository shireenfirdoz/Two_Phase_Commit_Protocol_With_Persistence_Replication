#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include <string>
#include <vector>

class CustomerOrder {
private:
	int customer_id;
	int order_number;
	int request_type;

public:
	CustomerOrder();
	void operator = (const CustomerOrder &order) {
		customer_id = order.customer_id;
		order_number = order.order_number;
		request_type = order.request_type;
	}
	void SetOrder(int cid, int order_num, int type);
	int GetCustomerId();
	int GetOrderNumber();
	int GetRequestType();

	int Size();

	void Marshal(char *buffer);
	void Unmarshal(char *buffer);

	bool IsValid();

	void Print();
};

class LaptopInfo {
private:
	int customer_id;
	int order_number;
	int request_type;
	int engineer_id;
	int admin_id;

public:
	LaptopInfo();
	void operator = (const LaptopInfo &info) {
		customer_id = info.customer_id;
		order_number = info.order_number;
		request_type = info.request_type;
		engineer_id = info.engineer_id;
		admin_id = info.admin_id;
	}
	void SetInfo(int cid, int order_num, int type, int engid, int expid);
	void CopyOrder(CustomerOrder order);
	void SetEngineerId(int id);
	void SetAdminId(int id);

	int GetCustomerId();
	int GetOrderNumber();
	int GetRequestType();
	int GetEngineerId();
	int GetAdminId();

	int Size();

	void Marshal(char *buffer);
	void Unmarshal(char *buffer);

	bool IsValid();

	void Print();
};

class CustomerRecord {
private:
	int customer_id;
	int last_order;

public:
	CustomerRecord() {
		this->customer_id = -1;
		this->last_order = -1;
	}

	CustomerRecord(int customer_id, int last_order) {
		this->customer_id = customer_id;
		this->last_order = last_order;
	}

	void setCustomerId(int cid);
	void setLastOrder(int num);
	void print();
	int getCustomerId();
	int getLastOrder();
	int Size();
};

struct peer_servers_info {
	int id;
	std::string ip;
	int port;
	std::string backup_ip;
	int backup_port;
};

struct MapOp {
	int opcode; // operation code: 1 - update value
	int arg1; // customer_id to apply the operation
	int arg2; // parameter for the operation
};

class ReplicationRequest {
private:
	int factory_id;
	int committed_index;
	int last_index;
	MapOp oprn;

public:
	ReplicationRequest();
	ReplicationRequest(int fid, int cidx, int lidx, MapOp op);
	int GetFactoryId();
	int GetCommittedIndex();
	int GetLastIndex();
	MapOp getMapOp();
	void print();
};

class Transaction { 
private:
	int txn_id;
	int lenReadRecords;
	int lenWriteRecords;
	std::vector<CustomerRecord> readRecords; 
	std::vector<CustomerOrder> writeRecords; 
	
public:
	Transaction();
	Transaction(std::vector<CustomerRecord> reads, std::vector<CustomerOrder> writes);
	std::vector<CustomerRecord> getReadRecords();
	std::vector<CustomerOrder> getWriteRecords();
	int getTxnId();
	void setTxnId(int id);
	void print();
	int Size();
	void Marshal(char *buffer);
	void Unmarshal(char *buffer);
	int getLenReadRecords();
	void setLenReadRecords(int id);
	int getLenWriteRecords();
	void setLenWriteRecords(int id);

}; 
#endif // #ifndef __MESSAGES_H__
