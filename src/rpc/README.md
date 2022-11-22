# The RPC classes

In this folder, you can find all the necessary classes for the RPC (Remote Procedure Call) to work correctly.
There is many different classes so we will briefly describe them : 

* The RPC class is the base class from which all the other classes will inherit from. This is mainly use to simplify the communication by only using the RPC class in the communication functions and being able to parse all the other classes from it.
* The RPC communications functions are in the file ``rpc_communication.cpp``. In this file, there is all the functions used to send and receive queries from all the other processes.
* The other folders contains many classes that are used in the project (for the servers elections, or append new logs for example) are : 
    * `AppendEntries` and `AppendEntriesResponse`
    * `LogEntry`
    * `NewLogEntry` and `NewLogEntryResponse`
    * `Heartbeat`
    * `SearchLeader` and `SearchLeaderResponse`
    * `Query`
    * `VoteRequest` and `VoteResponse`

