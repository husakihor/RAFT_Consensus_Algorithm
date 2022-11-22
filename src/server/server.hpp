#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <fstream>
#include <queue>
#include <vector>

#include "mpi.h"
#include "clock/clock.hpp"
#include "rpc/entries/log_entry.hpp"
#include "rpc/query/query.hpp"
#include "message/message.hpp"

enum class ServerStatus { FOLLOWER, CANDIDATE, LEADER, DEAD };
enum class ServerSpeed 
{
    // The speed is linked to the delay that the server will wait before each update
    // So the more the server is fast, the less the delay is high
    HIGH = 0,
    MEDIUM = 250,
    LOW = 500
};

class Server
{
public:
    // Constructor
    Server(int rank, int servers_count, int clients_count);

    // Core functions
    void run_server();

    // Operator to print the server data easily
    friend std::ostream& operator<< (std::ostream& out, const Server& server);

private:

    // Status changes and the needed operations for it
    void set_as_follower();
    void set_as_candidate();
    void set_as_leader();

    // Candidate and leader routines (follower is done in the update function)
    void candidate_routine(const std::vector<Query>& queries);
    void leader_routine(const std::vector<Query>& queries);

    // Queries handling functions
    void handle_vote_request(const Query& query);
    void handle_new_entries(const Query& query);
    void handle_message(const Query& query);
    void handle_queries(std::vector<Query> received_queries);

    // Update function (to update the server status, send and receive queries)
    void update();

    // ===== Server class privates variables =====

    // Rank of the server
    int _rank;
    // Status of the server
    ServerStatus _status;
    // Current term of the server (initialized to 1)
    int _current_term;
    // Filepath of the log of the server
    std::string _log_filepath;

    // Server speed (the time that the server will wait between each updata)
    ServerSpeed _server_speed;

    // Clock to detect the timeout
    Clock _clock;
    // Timeout for the election of the node (switch to candidate and start election)
    float _election_timeout;
    // Timeout for the heartbeat of the node
    float _heartbeat_timeout;

    // Vote of the server for the leader election
    size_t _voted_for;
    // Current vote count of the server (number of times it has voted)
    size_t _vote_count;

    // Servers count
    size_t _servers_count;
    // Clients count
    size_t _clients_count;

    // Value saying if the server is completely stop or not
    bool _is_stopped;

    // Queue of the entries of the server
    std::queue<Query> _entries_queue;
    
    // Log entries of the server
    // Each entry contains command for state machine, and the term when this log entry was received by leader (the first index is 1)
    std::vector<LogEntry> _server_log;
    // Index of highest log entry known to be committed (initialized to 0, increase monotonically)
    int _commit_index;
    // Index of highest log entry applied to state machine (initialized to 0, increase monotonically)
    int _last_log_applied;
    // For each server, indicate the index of the next log entry to send to that server
    std::vector<int> _next_log_index;
    // For each server, indicate the index of the highest log entry known to be replicated on a the server of the index (initialized to 0, increase monotonically)
    std::vector<int> _log_index_match;
};