## Implementation of the RAFT Consensus Algorithm for Log Replication
Authors :
* Jules Dorbeau
* Benoist André
* Noé Jenn-Treyer
* Ihor Husak
* Erwann Harris

Supervised by :
* Etienne Renault
* Antoine Martin

### 1.Requirements
---
* `C++` compiler suporting `C++17`
* `libopenmpi-dev`
* `cmake`
* `make`

### 2.Build & Run
---
To use the program you should use the `./run.sh` script with the following arguments.

    chmod +x run.sh
    ./run.sh [number_of_clients] [number_of_servers]
`number_of_clients` : Number of client instances you want to create.

`number_of_servers` : Number of server instances you want to create.

This will open a Command Line Interface in which you will be able to interact with the processes.

---

If you want you can build the code yourself using the following commands

    mkdir build
    cd build
    cmake ..
    make
    cd ..
    mpirun -n {number_of_clients + number_of_servers + 1} .build/algorep {number_of_clients} {number_of_servers)

> 
### 3. Run
---

During the run of the program, you will be able to enter different REPL commands to the servers cluster. 

You will find them just behind here but you can also have them printed on your console output by sending the REPL command "help".

To stop the program, you just have to send the REPL command "stop_all". This command will close all the processes that were launched by MPI and end the program.

### 4. Controller commands
---
Find bellow the list of command that you can use :
* `help` : Displays the list of commands.
* `process_informations` : Displays the ranks for all the processes that are running.
* `set_speed {client_rank} {speed}` : Changes the speed of the process. Parameters being : 
    * `low` : 500ms delay.
    * `medium`: 250ms delay.
    * `high`: 0ms delay.
* `start_client {client_rank}` : Used to start a client, all clients having the `DEAD` status at the beginning.
* `crash_process {process_rank}` : Makes a process crash. The process will only receive and treat commands from the REPL controller.
* `recover_process {process_rank}` : Makes a process receive commands from other processes again and catch up the missing logs.
* `add_log_entry {client_rank} {log_entry}` : Sends a new log entry to the client that will after send it to the servers.
* `add_files_entries {client_rank} {files_list}` : Adds all the passed files into the given client log entries.
* `timeout_server {server_rank}` : Forces a server to timeout. This will force an election and a change of leader in the servers.
* `stop_process {process_rank}` : Stops a process. It will not be able to receive any commands after this one.
* `display_process {process_rank}` : Displays the informations of the process.
* `stop_all`: Stop all the processes that are currently running.
