# Our tests 

Here is the list of all our test during the development of the project.

We wanted to create a function test suite that run all the tests in one command but ran out of time for that. Instead, we choose to mark here all the results of our tests with all the commands that are used. We wille be using for that, the static logs files of the clients and some other files.

* Test 1: 
    Parameters :
    > Client number : 2
    > Server number : 5
    
    Commands : 
    > display_process x (in order to find the leader)

    > set_speed {leader} low

    > display_process x (in order to find the leader)

    Result : 
    > SUCCESS : Leader changed because of FOLLOWER's timeout.

* Test 2: 
    Parameters :
    > Client number : 2
    > Server number : 5
    
    Commands : 
    > display_process x (in order to find the leader)

    > start_client 1

    > set_speed {leader} low

    > display_process x (in order to find the leader)

    Result : 
    > FAILURE : The leader did not change and the CANDIDATE is locked in CANDIDATE status (may be due to a condition on the logs for the election)

* Test 3: 
    Parameters :
    > Client number : 2

    > Server number : 5
    
    Commands : 
    > start_client 1

    > start_client 2

    Result : 
    > SUCCESS : All the servers logs contain all the entries sent by all the clients.

* Test 4: 
    Parameters :
    > Client number : 2

    > Server number : 5
    
    Commands : 
    > start_client 1

    > add_log_entry 1 testing a new log entry

    > crash_process 1

    > add_log_entry 1 testing a new log entry

    Result : 
    > SUCCESS : the second add_log_entry was not written in the server logs

* Test 5: 
    Parameters :
    > Client number : 2

    > Server number : 5
    
    Commands : 
    > start_client 1

    > display_process x => follower

    > crash_process {follower}

    > start_client 2

    Result : 
    > SUCCESS : All the running servers have the correct logs but the crashed server only has the logs of client 1

* Test 6: 
    Parameters :
    > Client number : 2

    > Server number : 5
    
    Commands : 
    > start_client 1

    > add_log_entry 1 testing a new log entry

    > crash_process 1
    
    > add_log_entry 1 testing a new log entry while crashed

    > recover_process 1

    > add_log_entry 1 testing a new log entry after recovery

    Result : 
    > SUCCESS : All the logs sent durring the crash have been written before the log after recovery.

* Test 7: 
    Parameters :
    > Client number : 2
    
    > Server number : 5
    
    Commands : 
    > start_client 1

    > display_process x to find the leader

    > crash_process {leader}

    > display_process x to find new leader

    Result : 
    > FAILURE : No new LEADER stuck at status CANDIDATE

* Test 8: 
    Parameters :
    > Client number : 2
    
    > Server number : 5
    
    Commands : 
    > display_process x to find the leader

    > crash_process {leader}

    > display_process x to find new leader

    > start_client 1

    > process_recover {old_leader}

    Result : 
    > SUCCESS : The OLD LEADER has the same logs as all the other servers.

* Test 9: 
    Parameters :
    > Client number : 2
    
    > Server number : 5
    
    Commands : 
    > start_client 1

    > add_files_entries 1 client_commands/new_log_entries_files/new_single_log_entry.txt        
    client_commands/new_log_entries_files/new_empty_log_entry.txt client_commands/new_log_entries_files/new_multiple_log_entries.txt => 

    Result : 
    > SUCCESS : All the logs contains the entries of the given files. 
    `(Might have an error while parsing the empty one)`

* Test 8: 
    Parameters :
    > Client number : 2
    
    > Server number : 5
    
    Commands : 
    > display_process x to find the leader

    > timeout_server {any_non_leader}

    Result : 
    > SUCCESS : The timeout server is now the `LEADER` and the found `LEADER` is now a `FOLLOWER` 

* Test 8: 
    Parameters :
    > Client number : 2
    
    > Server number : 5
    
    Commands : 
    > display_process x to find the leader

    > stop_process {leader}

    > display_process x to find the new leader

    > start_client 1

    > start_client 2

    > stop_process 1

    > add_log_entries 1 testing stopped client

    > add_log_entries 2 testing stopped client

    Result : 
    > SUCCESS : All the servers contains all the client's static logs and the entry of client 2, except for the old leader which has no logs at all.

`END OF OUR TESTS`