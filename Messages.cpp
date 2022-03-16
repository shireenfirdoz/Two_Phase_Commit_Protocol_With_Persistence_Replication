#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include "Messages.h"

CustomerOrder::CustomerOrder() {
	customer_id = -1;
	order_number = -1;
	request_type = -1;
}

void CustomerOrder::SetOrder(int id, int number, int type) {
	customer_id = id;
	order_number = number;
	request_type = type;
}

int CustomerOrder::GetCustomerId() { return customer_id; }
int CustomerOrder::GetOrderNumber() { return order_number; }
int CustomerOrder::GetRequestType() { return request_type; }

int CustomerOrder::Size() {
	return sizeof(customer_id) + sizeof(order_number) + sizeof(request_type);
}

void CustomerOrder::Marshal(char *buffer) {
	int net_customer_id = htonl(customer_id);
	int net_order_number = htonl(order_number);
	int net_request_type = htonl(request_type);
	int offset = 0;
	memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(buffer + offset, &net_request_type, sizeof(net_request_type));
}

void CustomerOrder::Unmarshal(char *buffer) {
	int net_customer_id;
	int net_order_number;
	int net_request_type;
	int offset = 0;
	memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(&net_order_number, buffer + offset, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(&net_request_type, buffer + offset, sizeof(net_request_type));

	customer_id = ntohl(net_customer_id);
	order_number = ntohl(net_order_number);
	request_type = ntohl(net_request_type);
}

bool CustomerOrder::IsValid() {
	return (customer_id != -1);
}

void CustomerOrder::Print() {
	std::cout << "id " << customer_id << " ";
	std::cout << "num " << order_number << " ";
	std::cout << "type " << request_type << std::endl;
}

LaptopInfo::LaptopInfo() {
	customer_id = -1;
	order_number = -1;
	request_type = -1;
	engineer_id = -1;
	admin_id = -1;
}

void LaptopInfo::SetInfo(int id, int number, int type, int engid, int expid) {
	customer_id = id;
	order_number = number;
	request_type = type;
	engineer_id = engid;
	admin_id = expid;
}

void LaptopInfo::CopyOrder(CustomerOrder order) {
	customer_id = order.GetCustomerId();
	order_number = order.GetOrderNumber();
	request_type = order.GetRequestType();
}
void LaptopInfo::SetEngineerId(int id) { engineer_id = id; }
void LaptopInfo::SetAdminId(int id) { admin_id = id; }

int LaptopInfo::GetCustomerId() { return customer_id; }
int LaptopInfo::GetOrderNumber() { return order_number; }
int LaptopInfo::GetRequestType() { return request_type; }
int LaptopInfo::GetEngineerId() { return engineer_id; }
int LaptopInfo::GetAdminId() { return admin_id; }

int LaptopInfo::Size() {
	return sizeof(customer_id) + sizeof(order_number) + sizeof(request_type)
		+ sizeof(engineer_id) + sizeof(admin_id);
}

void LaptopInfo::Marshal(char *buffer) {
	int net_customer_id = htonl(customer_id);
	int net_order_number = htonl(order_number);
	int net_request_type = htonl(request_type);
	int net_engineer_id = htonl(engineer_id);
	int net_admin_id = htonl(admin_id);
	int offset = 0;

	memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(buffer + offset, &net_request_type, sizeof(net_request_type));
	offset += sizeof(net_request_type);
	memcpy(buffer + offset, &net_engineer_id, sizeof(net_engineer_id));
	offset += sizeof(net_engineer_id);
	memcpy(buffer + offset, &net_admin_id, sizeof(net_admin_id));

}

void LaptopInfo::Unmarshal(char *buffer) {
	int net_customer_id;
	int net_order_number;
	int net_request_type;
	int net_engineer_id;
	int net_admin_id;
	int offset = 0;

	memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
	offset += sizeof(net_customer_id);
	memcpy(&net_order_number, buffer + offset, sizeof(net_order_number));
	offset += sizeof(net_order_number);
	memcpy(&net_request_type, buffer + offset, sizeof(net_request_type));
	offset += sizeof(net_request_type);
	memcpy(&net_engineer_id, buffer + offset, sizeof(net_engineer_id));
	offset += sizeof(net_engineer_id);
	memcpy(&net_admin_id, buffer + offset, sizeof(net_admin_id));

	customer_id = ntohl(net_customer_id);
	order_number = ntohl(net_order_number);
	request_type = ntohl(net_request_type);
	engineer_id = ntohl(net_engineer_id);
	admin_id = ntohl(net_admin_id);
}

bool LaptopInfo::IsValid() {
	return (customer_id != -1);
}

void LaptopInfo::Print() {
	std::cout << "id " << customer_id << " ";
	std::cout << "num " << order_number << " ";
	std::cout << "type " << request_type << " ";
	std::cout << "engid " << engineer_id << " ";
	std::cout << "expid " << admin_id << std::endl;
}

void CustomerRecord::setCustomerId(int cid) {
	customer_id = cid;
}

void CustomerRecord::setLastOrder(int num) {
	last_order = num;
}

void CustomerRecord::print() {
	std::cout << "cust id " << customer_id << "\t";
	std::cout << "last order " << last_order << "\n";
}

int CustomerRecord::getCustomerId() {
	return customer_id;
}

int CustomerRecord::getLastOrder() {
	return last_order;
}

int CustomerRecord::Size() {
	return sizeof(customer_id) + sizeof(last_order);
}


ReplicationRequest::ReplicationRequest(int fid, int cidx, int lidx, MapOp op) {
	factory_id = fid;
	committed_index = cidx;
	last_index = lidx;
	oprn = op;
}

ReplicationRequest::ReplicationRequest() {
	factory_id = -1;
	committed_index = -1;
	last_index = -1;
}

int ReplicationRequest::GetFactoryId() {
	return factory_id;
}

int ReplicationRequest::GetCommittedIndex() {
	return committed_index;
}

int ReplicationRequest::GetLastIndex() {
	return last_index;
}

MapOp ReplicationRequest::getMapOp() {
	return oprn;
}

void ReplicationRequest::print() {
	std::cout<<" Fact id "<< factory_id <<" cidx " << committed_index<<" la idx "<<
		last_index<<std::endl;
}

Transaction::Transaction() {
}

Transaction::Transaction(std::vector<CustomerRecord> a,
						std::vector<CustomerOrder> b) {
	this->readRecords = a;
	this->writeRecords = b;
}

std::vector<CustomerRecord> Transaction::getReadRecords() {
	return readRecords;
}

std::vector<CustomerOrder> Transaction::getWriteRecords() {
	return writeRecords;
}

int Transaction::getTxnId(){
   return txn_id;
}

void Transaction::setTxnId(int id){
  txn_id=id;
}

void Transaction::print() {
	for (auto &a : readRecords) {
		a.print();
	}
	for (auto &a : writeRecords) {
		a.Print();
	}
}

int Transaction::Size()
{
	return sizeof(txn_id) + sizeof(lenReadRecords) + sizeof(lenWriteRecords)
		+ sizeof(CustomerRecord) * lenReadRecords + sizeof(CustomerOrder) * lenWriteRecords;
}

void Transaction::Marshal(char *buffer)
{
	int offset = 0;
	int net_txn_id = htonl(txn_id);
	int net_lenReadRecords = htonl(lenReadRecords);
	int net_lenWriteRecords = htonl(lenWriteRecords);

	memcpy(buffer + offset, &net_txn_id, sizeof(net_txn_id));
	offset += sizeof(net_txn_id);

	memcpy(buffer + offset, &net_lenReadRecords, sizeof(net_lenReadRecords));
	offset += sizeof(net_lenReadRecords);

	memcpy(buffer + offset, &net_lenWriteRecords, sizeof(net_lenWriteRecords));
	offset += sizeof(net_lenWriteRecords);

	for (auto &record : readRecords)
	{
		int net_customer_id_1 = htonl(record.getCustomerId());
		int net_last_order = htonl(record.getLastOrder());
		memcpy(buffer + offset, &net_customer_id_1, sizeof(net_customer_id_1));
		offset += sizeof(net_customer_id_1);
		memcpy(buffer + offset, &net_last_order, sizeof(net_last_order));
		offset += sizeof(net_last_order);
	}

	for (auto &record : writeRecords)
	{
		int net_customer_id = htonl(record.GetCustomerId());
		int net_order_number = htonl(record.GetOrderNumber());
		int net_request_type = htonl(record.GetRequestType());

		memcpy(buffer + offset, &net_customer_id, sizeof(net_customer_id));
		offset += sizeof(net_customer_id);
		memcpy(buffer + offset, &net_order_number, sizeof(net_order_number));
		offset += sizeof(net_order_number);
		memcpy(buffer + offset, &net_request_type, sizeof(net_request_type));
		offset += sizeof(net_request_type);
	}
	//  subtract the last offset?
}

void Transaction::Unmarshal(char *buffer)
{

	int offset = 0;

	int net_txn_id;
	int net_lenReadRecords;
	int net_lenWriteRecords;

	memcpy(&net_txn_id, buffer + offset, sizeof(net_txn_id));
	offset += sizeof(net_txn_id);

	memcpy(&net_lenReadRecords, buffer + offset, sizeof(net_lenReadRecords));
	offset += sizeof(net_lenReadRecords);

	memcpy(&net_lenWriteRecords, buffer + offset, sizeof(net_lenWriteRecords));
	offset += sizeof(net_lenWriteRecords);

	txn_id = ntohl(net_txn_id);
	lenReadRecords = ntohl(net_lenReadRecords);
	lenWriteRecords = ntohl(net_lenWriteRecords);

	//std::cout<<"In Unmarshal: "<<txn_id << " "<<lenReadRecords<< " "<<lenWriteRecords<<"\n";

	for (int i = 0; i < lenReadRecords; i++)
	{

		int net_customer_id_1;
		int net_last_order;

		memcpy(&net_customer_id_1, buffer + offset, sizeof(net_customer_id_1));
		offset += sizeof(net_customer_id_1);
		memcpy(&net_last_order, buffer + offset, sizeof(net_last_order));
		offset += sizeof(net_last_order);

		CustomerRecord rec(ntohl(net_customer_id_1), ntohl(net_last_order));
		readRecords.push_back(rec);
	}

	for (int j = 0; j < lenWriteRecords; j++)
	{
		int net_customer_id;
		int net_order_number;
		int net_request_type;

		memcpy(&net_customer_id, buffer + offset, sizeof(net_customer_id));
		offset += sizeof(net_customer_id);
		memcpy(&net_order_number, buffer + offset, sizeof(net_order_number));
		offset += sizeof(net_order_number);
		memcpy(&net_request_type, buffer + offset, sizeof(net_request_type));
		offset += sizeof(net_request_type);

		CustomerOrder order;
		order.SetOrder(ntohl(net_customer_id), ntohl(net_order_number), ntohl(net_request_type));
		writeRecords.push_back(order);
	}
	//  subtract the last offset?
}

int Transaction::getLenReadRecords()
{
	return lenReadRecords;
}
void Transaction::setLenReadRecords(int len)
{
	lenReadRecords = len;
}
int Transaction::getLenWriteRecords()
{
	return lenWriteRecords;
}
void Transaction::setLenWriteRecords(int len)
{
	lenWriteRecords = len;
}
