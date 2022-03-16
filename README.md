# cs7610-project

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

- ./experiment 10.200.125.58 16000 10.200.125.56 16000 1    
              [TM IP] [TM port] [RM IP] [RM port] [# of client threads to create]

## Citations
- StackOverflow
- Beej's Guide to Network Programming
- GeeksForGeeks
- C++ TutorialsPoint
- cplusplus.com
