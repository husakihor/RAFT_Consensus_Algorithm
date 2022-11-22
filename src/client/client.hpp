#pragma once

#include <mpi.h>
#include <string>
#include <vector>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <thread>
#include <map>
#include <queue>

#include "clock/clock.hpp"
#include "message/message.hpp"
#include "rpc/query/query.hpp"
#include "rpc/entries/append_entries.hpp"
#include "rpc/rpc_communication.hpp"

// Enum class describing the current client status
enum class ClientStatus { RUNNING, DEAD };

enum class Speed 
{
    // The speed is linked to the delay that the client will wait before each update
    // So the more the server is fast, the less the delay is high
    HIGH = 0,
    MEDIUM = 250,
    LOW = 500
};

class Client
{
public:
    Client(int rank, int client_count, int server_count);

    // Run functions
    void run_client();

    // Operator to print the client data easily
    friend std::ostream& operator<< (std::ostream& out, const Client& client);

private:
    // Parsing the list of commands of the client
    void parse_commands_file();
    void parse_new_command_file(std::string filename);

    // Queries handling functions
    void handle_message(const Query& query);
    void handle_queries(const std::vector<Query>& queries);
    
    // Update function to update the client status
    void update(); 
    
    // ===== Client class privates variables =====

    // Rank of the client 
    int _rank;
    // Set to true if the client has started
    ClientStatus _status;
    // Speed of the client
    Speed _client_speed;
    // Set to true if the client has stopped
    bool _is_stopped;
    // General timeout of the client
    float _timeout;

    // Filepath for the commands file of the client
    std::string _commands_filepath;

    // Rank of the current servers leader (if no leader, set to 0)
    size_t _leader_rank;
    // Clock to detect the timeout (to run servers leader search queries)
    Clock _leader_clock;
  
    // Queue of the entries to send to the servers leader
    std::queue<NewLogEntry> _entries_to_send;
    // To dertermine if the next entry in the queue is commited to the leader
    bool _entry_sent;
    // Entry clock, used to check if the leader is dead
    Clock _entry_clock;

    // Server and client counts to determine the range of their ranks for the communication
    size_t _client_count;
    size_t _server_count;
    // Variable determining if the client already started 
    bool _has_started;
};