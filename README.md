# cs7610-project

## Project Goal :
1.	To implement the Two-phase commit (2PC) protocol on the laptop ordering system designed in the programming assignments. Thus, a user can first read the current orders for a set of customers and then place the orders (write) accordingly. 2PC will automatically commit/abort the transaction based on whether the data at the time of write is consistent with what was read. (Similar to 2PC implemented in last question of written assignment 2). 
2.	Persist customer record data and transaction logs in a file to ensure data is persisted across multiple runs of the server and not lost in case of whole system failure.

## Software Design :
![image](https://user-images.githubusercontent.com/79550954/158520220-aeaa70ca-fa32-44fb-aeef-9741804102aa.png)

## Implementation
•	Implement 2PC on laptop ordering system: Client program now runs in two stages: first a read and then write. Client issues a read request as specified in the README.md and looks at the current orders placed for a set of customers. It then issues a write request based on it. If another client has executed a transaction in the meanwhile, the current client’s read data is inconsistent and the transaction is aborted.
           
•	Persistence of customer record data and transaction logs in a file: We have implemented persistence module through file system. Customer records (customer_record) and transaction logs (txn_log) for each of the RM primary/backup are persisted locally on individual nodes in two separate files cusRecFile.txt and logFile.txt. CusRecFile.txt file maintains list of  CustomerRecord and logFile.txt maintains list of TxnLogRequest which has below parameters.
             txn_id: transaction id 
txn_operation: 1 for prepare phase, 2 for commit and 3 for abort.
customer_id: id of the customer whose record is writing
last_order_num : last order number of the customer
customer_opr : 1 for write operation

•	Replication of RM in backup nodes to increase resiliency: Replication is handled by implementing primary-backup protocol between RM’s and its replica through PFA and IFA roles during the write requests in the transaction. Logs are copied during prepare and commit phase which are applied to the backup node to synchronize the customer record and the transaction logs.

•	TM proactively handles an entire RM cluster failure and abort the transaction: Client issue a read request in the transaction and then the entire RM cluster fails then when client issue write request then the TM proactively handle this scenario by marking the transaction as failed and aborting it. This has been implemented in the TM which tries to connect with the primary first and then when receive the 0 from the socket Init method then tries to connect with the backup rm node. If backup rm node initialization also fails then it sends the failure message to the client.

## Handling Failure:
•	TM proactively process the transaction through rm backup node when primary rm fails in the cluster: When the primary RM node fails then if client sends’ read and write request TM proactively process the transaction through the RM backup node which now becomes the primary though IFA and PFA replication exchange of messages. TM automatically handles it as when it tries to connect primary and if the connection fails then then it connects the backup RM node. Therefore in the command line passing the backup RM node ip and port so that TM can connects it incase primary RM fails. 

•	Primary RM handles prepare request, goes down, and the commit is handled by backup RM: To implement this feature, have replicated the transaction log in the prepare phase of the primary RM node to the backup node and when primary goes down after the prepare phase the commit is handled by the backup RM. It’s difficult to make the primary down after successful completion of prepare phase. Hence we don’t have an evaluation experiment for it, but can check in the logFile.txt of the rm backup node that it contains an entry of TxnLogRequest with txn_operation as 1 for the correctness of the program.

## Recovery from Failure:
•	Recovery of the system from all node failures: Recovery of the RM system incase of all RM’s primary and backup cluster fails is through the persistence. When each of the individual RM (primary/backup) node comes up then it replays the data stored locally in the logFile.txt and custRecFile.txt into the txn_log and customer_records. 

## Achievements:
•	Implemented 2PC on laptop ordering system designed in the programming assignments.
•	Persistence of customer record data and transaction logs in a file. 
•	Additionally, achieved the stretch goal: replication of RM in backup nodes to increase resiliency.
•	Recovery of the system from all RM cluster node failures through persistence.
•	Primary RM handles prepare request and goes down, commit is handled by backup RM.
•	TM proactively handles an entire RM cluster failure and aborts the transaction.


## Commands to setup

### Resource Manager (primary)
- ./server 16000 0 1 1 10.200.125.57 16000    
           [port] [RM id] [# of peers (always 1)] [id of backup (always 1)] [backup's IP] [backup's port]

### Resource Manager (backup)
- ./server 16000 1 1 0 10.200.125.56 16000   
           [port] [RM id] [# of peers (always 1)] [id of primary (always 0)] [primary's IP] [primary's port]


### Transaction Manager
- ./tm 16000 1 0 10.200.125.56 16000 10.200.125.57 16000    
       [port] [# of RM clusters] [cluster id] [primary RM IP] [primary RM port] [backup RM IP] [backup RM port]
- Another e.g. with 2 RM clusters:       
  ./tm 16000 2 0 10.200.125.56 16000 10.200.125.57 16000 1 10.200.125.60 16000 10.200.125.61 16000

### Client
- ./client
    - 3 127.0.0.1 16000 1 128 (read order)      
      [request type (read)] [RM IP to read data from] [RM port] [# of threads (always 1)] [read data for first 128 customers]   
      (o/p displayed here)    
      (waiting for user prompt)    
    - 1 127.0.0.1 16002 2 10 (write order)        
      [request type (write)] [TM IP] [TM port] [write data of first 2 customers] [issue 10 laptop order requests]
    - Success/Failed (based on whether there was conflict)     
  Transaction request stats

- E.g. runs and output:    
```
    ./client
    3 10.200.125.56 16000 1 128
    cust id 0	last order 50
    cust id 1	last order 50
    cust id 2	last order 50
    cust id 3	last order 50
    cust id 4	last order 50
    216.286	125.550	516.987	1279.505	3907.761
    1 10.200.125.58 16000 5 10
    20.671	0.615	516.987	243523.385	225.851
    Transaction result and stats:
    Success
```

### Experimental client (for abort rate measurement)
              
transaction abort rate as the number of client threads increases. 

Here we are fixing the number of customer records we operate upon to 5 (cust id 0 – cust id 4). Each transaction consists of a single read and write operation on one of these customers that is chosen at random. The experiment is repeated 10 times in a for loop (see Experimental.cpp). As the number of client threads increases, we expect the abort rate to increase as chances of conflict is higher.  

Navigate to project folder and run make clean and make (applicable for all servers) 

ssh user@vdi-linux-056.ccs.neu.edu 

Run ./server 16000 0 1 1 10.200.125.57 16000 

ssh user@vdi-linux-057.ccs.neu.edu 

Run ./server 16000 1 1 0 10.200.125.56 16000 

ssh user@vdi-linux-058.ccs.neu.edu 

./tm 16000 1 0 10.200.125.56 16000 10.200.125.57 16000 

ssh user@vdi-linux-059.ccs.neu.edu 

Run ./experiment 10.200.125.58 16000 10.200.125.56 16000 1 (for 1 client thread) 

Run ./experiment 10.200.125.58 16000 10.200.125.56 16000 2 (for 2 client threads) 

Run ./experiment 10.200.125.58 16000 10.200.125.56 16000 3 (for 3 client threads) 

Measure the abort rate printed 

NOTE: This must be run multiple times (atleast 3-5) ignoring the outliers and taking the average of the rest of the readings. 


Below are readings and the graph plotted: 
![image](https://user-images.githubusercontent.com/79550954/158520444-1884dde5-03c0-42c8-9ea0-972fbd979bb2.png)

### Experiment 2: 
Variation of elapsed time as the number of RMs in the configuration increases from 1 to 5. 

Steps to reproduce for RM count = 2. Others can be setup similarly
•	Navigate to project folder and run make clean and make (applicable for all servers)
•	ssh user@vdi-linux-056.ccs.neu.edu
•	Run ./server 16000 0 1 1 10.200.125.57 16000
•	ssh user@vdi-linux-057.ccs.neu.edu
•	Run ./server 16000 1 1 0 10.200.125.56 16000
•	ssh user@vdi-linux-060.ccs.neu.edu
•	Run ./server 16000 0 1 1 10.200.125.61 16000
•	ssh user@vdi-linux-061.ccs.neu.edu
•	Run ./server 16000 1 1 0 10.200.125.60 16000
•	ssh user@vdi-linux-058.ccs.neu.edu
•	./tm 16000 2 0 10.200.125.56 16000 10.200.125.57 16000 1 10.200.125.60 16000 10.200.125.61 16000
•	ssh user@vdi-linux-059.ccs.neu.edu
Run ./client
3 10.200.125.56 16000 1 128
cust id 0	last order 10
cust id 1	last order 10
cust id 2	last order 10
cust id 3	last order 10
cust id 4	last order 10
396.965	107.159	1532.104	1881.384	2657.618
1 10.200.125.58 16000 5 10
37.417	0.610	1532.104	74644.691	736.824
Transaction result and stats:
Success
37.417	0.610	1532.104	1076.915	51071.802

•	Measure the elapsed time that is printed in the end (highlighted)
•	Repeat the experiment for RMs = 1 to RMs = 5 and record the measurement

Below are readings and the graph plotted:

![image](https://user-images.githubusercontent.com/79550954/158520536-a7e5f8a7-0adb-48ee-856e-41ff17527beb.png)


## Citations
- StackOverflow
- Beej's Guide to Network Programming
- GeeksForGeeks
- C++ TutorialsPoint
- cplusplus.com
