#include "server.hpp"

// Include the file where all the communication functions are
#include "rpc/rpc_communication.hpp"

// ========== Constructor function ==========

Server::Server(int rank, int servers_count, int clients_count) 
    : _rank(rank), _status(ServerStatus::FOLLOWER), _current_term(0), _clock(Clock()), 
      _voted_for(0), _vote_count(0), _servers_count(servers_count), _clients_count(clients_count),  
      _commit_index(-1), _last_log_applied(-1)
{
    // Timeout initializations
    srand(time(NULL) + this->_rank);
    this->_election_timeout = rand() % 200 + 200;   // timeout from 200 to 400
    this->_heartbeat_timeout = 25;

    // Setting the stop variable to false
    this->_is_stopped = false;
    // Setting up the server speed
    this->_server_speed = ServerSpeed::HIGH;

    // Creating the log path of the server
    this->_log_filepath = "server_logs/logs_server_" + std::to_string(rank) + ".txt";

    // Initializing the vectors of the server for logs synchronization
    this->_next_log_index = std::vector(servers_count, 0);
    this->_log_index_match = std::vector(servers_count, -1);

    // Initializing the log file of the server
    std::ofstream logs_file;
    logs_file.open(this->_log_filepath);
    if (!logs_file.good())
    {
        std::cerr << "Unable to open server logs file !" << std::endl;
    }
    logs_file.close();
};

// ========== Status changes function ==========

void Server::set_as_follower()
{
    this->_status = ServerStatus::FOLLOWER;
    this->_voted_for = 0;
    this->_vote_count = 0;
    this->_clock.reset();
}

void Server::set_as_candidate()
{
    // Setting up the status for the election
    this->_status = ServerStatus::CANDIDATE;
    
    // Term is monotonically increasing, here we are electing the leader of this term  
    this->_current_term += 1;

    // Voting for himself
    this->_voted_for = this->_rank;
    this->_vote_count = 1;

    // Getting the term of the last log of this server
    // Setting it to -1 if there is no logs to make it clear that this is the first one
    const int last_log_term = this->_server_log.empty() ? -1 : this->_server_log.back()._term;

    // Request for vote
    VoteRequest request = VoteRequest(this->_current_term, this->_rank, this->_server_log.size() - 1, last_log_term);
    send_to_all_processes(this->_rank, this->_servers_count, this->_clients_count + 1, request, 0);

    // Reset the clock timeout
    this->_clock.reset();
}

void Server::set_as_leader() 
{
    this->_status = ServerStatus::LEADER;
    this->_clock.reset();

    // Setting up the new next log index for all the servers as this one is the new leader
    // Also setting up the match log index to -1 as we don't know if any of the log match the leader logs
    const int new_log_index = this->_server_log.size();
    for (int server_rank = 0; server_rank < this->_servers_count; server_rank++)
    {
        this->_next_log_index.at(server_rank) = new_log_index;
        this->_log_index_match.at(server_rank) = -1;
    }

    // Send a first heartbeat as the new leader 
    int offset = this->_clients_count + 1; 
    for (int server_rank = 0; server_rank < this->_servers_count; server_rank++)
    {
        // Only used to send the request (as we have a offset with the clients)
        int destination_rank = offset + server_rank;
        if (destination_rank != this->_rank)
        {
            // Getting the previous log index and log term for the Heartbeat Query
            int prev_log_index = this->_next_log_index.at(server_rank) - 1;
            int prev_log_term = (prev_log_index >= 0) && (prev_log_index < (int)this->_server_log.size()) ? this->_server_log.at(prev_log_index)._term : -1;

            Heartbeat start_heartbeat = Heartbeat(this->_current_term, this->_rank, prev_log_index, prev_log_term, this->_commit_index);
            send_message(start_heartbeat, destination_rank, 0);
        }
    }
}

// ========== Routines function ==========

void Server::candidate_routine(const std::vector<Query>& queries)
{
    // Getting all the queries that the server received and treating the ones needed for the candidate
    for (const auto& query : queries)
    {
        if (query._type == RPC::RPC_TYPE::VOTE_RESPONSE)
        {
            const VoteResponse vote_response = std::get<VoteResponse>(query._content);
            this->_vote_count += vote_response._vote ? 1 : 0;
        }
        // If the candidate receive an entry from the leader, set as follower
        if (query._type == RPC::RPC_TYPE::APPEND_ENTRIES || query._type == RPC::RPC_TYPE::HEARTBEAT)
        {
            // While handling the Append Entries query, if the term is superior to the server term, set it as follower
            if (query._term >= this->_current_term)
            {
                this->set_as_follower();
                return;
            }
        }
    }

    // Check if the server received enougth votes to be the leader, set him as candidate again if not
    if ((int)this->_vote_count > this->_servers_count / 2)
    {
        this->set_as_leader();
    }
    else if (this->_clock.check() > this->_election_timeout)
    {
        this->set_as_candidate();
    }
}

void Server::leader_routine(const std::vector<Query>& queries)
{
    // As for MPI, we initialize the controler and then the clients, the offset for the servers is the number of clients
    // We are setting up this value here as we will use it for the heartbeat timeout and the queries parsing
    int offset = this->_clients_count + 1; 
    if (this->_clock.check() > this->_heartbeat_timeout)
    {
        for (int server_rank = 0; server_rank < this->_servers_count; server_rank++)
        {
            // Only used to send the request (as we have a offset with the clients)
            int destination_rank = offset + server_rank;
            if (destination_rank != this->_rank)
            {
                // Getting the previous log index and log term for the Append Entries Query
                int prev_log_index = this->_next_log_index.at(server_rank) - 1;
                int prev_log_term = (prev_log_index >= 0) && (prev_log_index < (int)this->_server_log.size()) ? this->_server_log.at(prev_log_index)._term : -1;
                
                // Check if the size of the logs of the server is superior or equal to the index of the next logs to send to the destination server
                // If it is not the case, then we don't have any logs to send so we don't need to do anything
                if ((int)this->_server_log.size() - 1 >= this->_next_log_index.at(server_rank))
                {
                    // Getting all the logs that we need to send 
                    auto start = this->_server_log.begin() + this->_next_log_index.at(server_rank);
                    auto end = this->_server_log.end();
                    std::vector<LogEntry> entries_to_send(start, end);

                    AppendEntries append_entry = AppendEntries(this->_current_term, this->_rank, prev_log_index, prev_log_term, entries_to_send, this->_commit_index);
                    send_message(append_entry, destination_rank, 0);
                }
                else
                {
                    // Sending a Heartbeat to the destination_rank server 
                    Heartbeat heartbeat = Heartbeat(this->_current_term, this->_rank, prev_log_index, prev_log_term, this->_commit_index);
                    send_message(heartbeat, destination_rank, 0);
                }
            }
        }

        // Reset the clock as we send a query
        this->_clock.reset();
    }

    // Parsing the queries that need to be parsed by the leader
    for (const Query& query : queries)
    {
        if (query._type == RPC::RPC_TYPE::NEW_LOG_ENTRY)
        {
            const NewLogEntry& new_entry = std::get<NewLogEntry>(query._content);
            this->_server_log.emplace_back(this->_current_term, new_entry._log_entry._command);
            this->_entries_queue.emplace(query);
        }
        else if (query._type == RPC::RPC_TYPE::APPEND_ENTRIES_RESPONSE)
        {
            const AppendEntriesResponse& response = std::get<AppendEntriesResponse>(query._content);
            size_t source_rank = query._source_rank - offset;

            // If the response is success, then update the match and next log indexes
            if (response._success)
            {
                this->_log_index_match.at(source_rank) += 1;

                if (this->_log_index_match.at(source_rank) >= this->_next_log_index.at(source_rank))
                {
                    this->_next_log_index.at(source_rank) += 1;
                }
            }
            // If not, then decrease the source rank next log index
            else
            {
                this->_next_log_index.at(source_rank) -= 1;
            }
        }
    }
    
    // Updating the commit index of the leader 
    int new_commit_index = this->_commit_index + 1;
    size_t updated_commit_count = 1; // Set at 1 because there is the leader

    // Checking if the commit index is the same for the server and counting them
    for (size_t server_rank = 0; server_rank < this->_servers_count; server_rank++)
    {
        if (this->_rank != (server_rank + offset))
        {
            if (this->_log_index_match.at(server_rank) >= new_commit_index)
            {
                updated_commit_count += 1;
            }
        }
    }

    // Then if the majority (so more than the half of the servers) are up to date with th commit index, 
    // Update the leader's commit index
    if ((updated_commit_count > (this->_servers_count / 2)))
    {
        if (this->_server_log.at(new_commit_index)._term == this->_current_term)
        {
            this->_commit_index = new_commit_index;
        }
    }
}

// ========== Queries handling functions ==========

void Server::handle_vote_request(const Query& query) 
{
    const VoteRequest& vote_request = std::get<VoteRequest>(query._content);

    // If the term of the request is inferior to the one of the server, then deny the request
    if (vote_request._term < this->_current_term)
    {
        send_message(VoteResponse(this->_current_term, false), vote_request._candidate_rank, 0);
    }
    // If the server did not voted yet (as 0 is the controler, there must not be any vote for him)
    else if (this->_voted_for == 0 || this->_voted_for == vote_request._candidate_rank)
    {
        // If the logs of the candidate are empty and the logs of the server are empty
        if ((vote_request._last_log_index == -1) && (this->_server_log.empty()))
        {
            send_message(VoteResponse(vote_request._term, true), vote_request._candidate_rank, 0);
            this->_voted_for = vote_request._candidate_rank;
            // std::cerr << "Server " << this->_rank << " voted for " << vote_request._candidate_rank << std::endl;
            return;
        }
        // If the candidate last log and the server last log terms are the same 
        // Then check if the server logs are as updated as the server logs 
        if (this->_server_log.at(vote_request._last_log_index)._term == vote_request._last_log_term)
        {
            if (vote_request._last_log_index >= this->_server_log.size())
            {
                send_message(VoteResponse(vote_request._term, true), vote_request._candidate_rank, 0);
                this->_voted_for = vote_request._candidate_rank;
                return;
            }
            else
            {
                send_message(VoteResponse(vote_request._term, false), vote_request._candidate_rank, 0);
            }
        }
        // If the request last log term is greater than the server last log term then vote for the candidate
        else if (this->_server_log.at(vote_request._last_log_index)._term < vote_request._last_log_term)
        {
            send_message(VoteResponse(vote_request._term, true), vote_request._candidate_rank, 0);
            this->_voted_for = vote_request._candidate_rank;
            return;
        }
        else
        {
            send_message(VoteResponse(vote_request._term, false), vote_request._candidate_rank, 0);
        }
    }
    // If the conditions are not met, then return false to the vote request
    else
    {
        send_message(VoteResponse(vote_request._term, false), vote_request._candidate_rank, 0);
    }
}

void Server::handle_new_entries(const Query& query)
{
    // Parsing our query to get our new entries
    const AppendEntries& new_entries = std::get<AppendEntries>(query._content);

    // If the query term is inferior to the server term, then deny query
    if (new_entries._term < this->_current_term)
    {
        send_message(AppendEntriesResponse(this->_current_term, false), new_entries._leader_rank, 0);
        return;
    }
    // Check if there is entries to append to the logs
    // Normaly, should not happend as we have a dedicated class for the Heartbeat (but checked for safety)
    else if (!new_entries._entries.empty())
    {
        // Check if there is a previous log index
        if (new_entries._prev_log_index != -1)
        {
            // Check if the server logs contains a LogEntry at the previous log index of the query
            // Then check if the LogEntry contained at the previous index log of the query has the same term as the server
            // If those conditions are not met, then deny the request
            if ((new_entries._prev_log_index >= (int)this->_server_log.size()) || 
                (this->_server_log.at(new_entries._prev_log_index)._term != new_entries._prev_log_term))
            {
                send_message(AppendEntriesResponse(new_entries._term, false), new_entries._leader_rank, 0);
                return;
            }
        }

        // Reseting the clock as we don't have any reasons to deny the query now
        this->_clock.reset();

        // Starting and ending iterators to copy the logs that will not change
        const int previousLogIndex = new_entries._prev_log_index;
        auto start = this->_server_log.begin();
        auto end = this->_server_log.begin() + previousLogIndex + 1;
        std::vector<LogEntry> new_logs(start, end);

        // Setting up variables for the loop
        int i = previousLogIndex + 1;

        // Then now we are going to copy the rest of the new entries in our logs
        // Defining a variable to determine if there is any conflict during the copy of the new log entries
        bool conflict = false;
        for (; i < (int)new_entries._entries.size() + previousLogIndex + 1;  i++)
        {
            const LogEntry& new_entry = new_entries._entries.at(i - (previousLogIndex + 1));

            if (i < (int)this->_server_log.size() && !conflict)
            {
                const LogEntry& old_entry = this->_server_log.at(i);

                // Checking if the new and old entry have the same term,  then there is a conflict
                conflict = old_entry._term != new_entry._term;

                // If there is no conflict, then add the old_entry
                if (!conflict)
                {
                    new_logs.push_back(old_entry);
                }
            }
            // If there is a conflict, ignore the rest of the old logs and add the new entries
            else
            {
                new_logs.push_back(new_entry);
            }
        }

        // Replacing the old logs by the new ones
        this->_server_log.swap(new_logs);

        // If the leader commit index is superior to the server commit index
        // Then set the commit index to the minimum between the leader's one and the index of last new entry
        if (new_entries._leader_commit > this->_commit_index)
        {
            this->_commit_index = std::min(new_entries._leader_commit, (int)this->_server_log.size() - 1);
        }

        // Send the response saying that the queries has been appened correctly
        send_message(AppendEntriesResponse(new_entries._term, true), new_entries._leader_rank, 0);
    }
}

void Server::handle_message(const Query& query) 
{
    // Boolean describing the status of the parsing (success or failure)
    bool parsing_message_status = true;

    const Message& message = std::get<Message>(query._content);
    switch (message._type)
    {
        case Message::MESSAGE_TYPE::PROCESS_SET_SPEED:
        {
            // Setting up the server speed (delay during what the server will way before each update)
            if (message._content == "low")
            {
                this->_server_speed = ServerSpeed::LOW;
            } 
            else if (message._content == "medium")
            {
                this->_server_speed = ServerSpeed::MEDIUM;
            }
            // Not checking the speed here, as we will set it as HIGH by default if it is not LOW or MEDIUM
            else 
            {
                this->_server_speed = ServerSpeed::HIGH;
            }
            break;
        }
        case Message::MESSAGE_TYPE::PROCESS_STOP:
        {
            // Stopping the server, it won't be working anymore
            this->_is_stopped = true;
            break;
        }
        case Message::MESSAGE_TYPE::PROCESS_CRASH:
        {
            if (this->_status != ServerStatus::DEAD)
            {
                // Changing the server status to DEAD to simulate a server crash
                // Reseting his settings to make sure that it won't have the same when recovering (to avoid confusion)
                this->_status = ServerStatus::DEAD;
                this->_vote_count = 0;
                this->_entries_queue = std::queue<Query>();
                this->_current_term = 0;
                this->_voted_for = 0;
                this->_next_log_index = std::vector<int>(this->_servers_count, 0);
                this->_log_index_match = std::vector<int>(this->_servers_count, -1);
            }
            else
            {
                std::cout << "Server " << this->_rank << " is already DEAD." << std::endl;
                parsing_message_status = false;
            }
            break;
        }
        case Message::MESSAGE_TYPE::PROCESS_RECOVER:
        {
            if (this->_status == ServerStatus::DEAD)
            {
                // Recover a server to set him as follower (a server with DEAD status)
                this->set_as_follower();
            }
            else
            {
                std::cout << "Server " << this->_rank << " has not crashed and so cannot be recovered." << std::endl;
                parsing_message_status = false;
            }
            break;
        }
        case Message::MESSAGE_TYPE::SERVER_TIMEOUT:
        {
            // Forcing a new election if the server is a follower as we make it timeout
            if (this->_status == ServerStatus::FOLLOWER)
            {
                this->set_as_candidate();
            }
            else 
            {
                std::cout << "Server " << this->_rank << " is dead, already is a leader or candidate, and cannot start another election." << std::endl;
                parsing_message_status = false;
            }
            break;
        }
        case Message::MESSAGE_TYPE::PROCESS_DISPLAY:
        {
            std::map<int, std::string> status_map {{0, "FOLLOWER"}, {1, "CANDIDATE"}, {2, "LEADER"}, {3, "DEAD"}};
            std::map<int, std::string> speed_map {{0, "HIGH"}, {250, "MEDIUM"}, {500, "LOW"}};
            std::cerr << "Server " << this->_rank << " has the status " << status_map.at((int)this->_status) << " and his speed is " << speed_map.at((int)this->_server_speed) << std::endl;
            break;
        }
        default:
            // In the case where we don't know the message type, return false as the response
            parsing_message_status = false;
            break;
    }
    // Sending the message response with his status
    MessageResponse message_response = MessageResponse(parsing_message_status);
    send_message(message_response, query._source_rank, 0);
}

void Server::handle_queries(std::vector<Query> received_queries) 
{
    // Applying all the logs that need to be applied to the server logs
    while (this->_commit_index > this->_last_log_applied)
    {
        this->_last_log_applied += 1;

        std::ofstream logs_file;
        logs_file.open(this->_log_filepath, std::ofstream::app);
        if (logs_file.good())
        {
            logs_file << this->_server_log.at(this->_last_log_applied)._command << std::endl;
        }
        else
        {
            std::cerr << "Server " << this->_rank << " is unable to open file : " << this->_log_filepath << std::endl;
        }
        logs_file.close();

        // If this is the leader, then send a Reponse saying that the entry has been applied corretly
        if (this->_status == ServerStatus::LEADER)
        {
            Query entry_query = this->_entries_queue.front();
            send_message(NewLogEntryResponse(true), entry_query._source_rank, 0);
            this->_entries_queue.pop();
        }
    }

    for (const Query& query : received_queries)
    {
        // Check the term of the query if the server is not dead in a first place to update it
        if (this->_status != ServerStatus::DEAD && query._term > this->_current_term)
        {
            this->_current_term = query._term;
            this->set_as_follower();
        }
        // Checking if the type of the query is a Message to handle it 
        if (query._type == RPC::RPC_TYPE::MESSAGE)
        {
            this->handle_message(query);
        }

        if (this->_status != ServerStatus::DEAD)
        {
            switch (query._type)
            {
                case RPC::RPC_TYPE::HEARTBEAT:
                {
                    // Updating the commit
                    const Heartbeat& heartbeat = std::get<Heartbeat>(query._content);
                    // Reset the leader to prevent from an infinite candidate
                    if (this->_voted_for != 0)
                    {
                        this->_voted_for = 0;
                    }
                    if (heartbeat._leader_commit > this->_commit_index)
                    {
                        this->_commit_index = std::min(heartbeat._leader_commit, (int)this->_server_log.size() - 1);
                    }
                    // Reseting the clock
                    this->_clock.reset();
                    break;
                }
                case RPC::RPC_TYPE::APPEND_ENTRIES:
                {
                    this->handle_new_entries(query);
                    break;
                }
                case RPC::RPC_TYPE::VOTE_REQUEST:
                {
                    handle_vote_request(query);
                    // Clock reset to make sure that it does not request to be candidate just after
                    this->_clock.reset();
                    break;
                }
                case RPC::RPC_TYPE::SEARCH_LEADER:
                {
                    if (this->_status == ServerStatus::LEADER)
                    {
                        SearchLeaderResponse leader_response = SearchLeaderResponse(this->_rank);
                        send_message(leader_response, query._source_rank, 0);
                    }
                    break;
                }
                case RPC::RPC_TYPE::NEW_LOG_ENTRY:
                {
                    // If this is not the leader, deny the query as only the leader can interact with clients
                    // We need here to answer the query as this will tell the clients that his leader rank is outdated
                    if (this->_status != ServerStatus::LEADER)
                    {
                        NewLogEntryResponse new_log_entry_response = NewLogEntryResponse(false);
                        send_message(new_log_entry_response, query._source_rank, 0);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

// ========== Update and run function ==========

void Server::update() 
{
    std::vector<Query> received_queries;
    receive_all_messages(this->_rank, (this->_servers_count + this->_clients_count + 1), 0, received_queries, 0);
    handle_queries(received_queries);

    switch (this->_status)
    {
        case ServerStatus::FOLLOWER:
        {
            // If the server reached it's timeout, then become candidate and start new election
            if (this->_clock.check() > this->_election_timeout) 
            {
                // Set as candidate and request for vote
                this->set_as_candidate();
            }
            break;   
        }
        case ServerStatus::CANDIDATE:
        {
            this->candidate_routine(received_queries);
            break;
        }
        case ServerStatus::LEADER:
        {
            this->leader_routine(received_queries);
            break;
        }
        default:
            break;
    }
}

void Server::run_server() 
{
    while (!this->_is_stopped)
    {
        // Waiting depending on the speed of the server
        this->_clock.wait((int)this->_server_speed);

        // Updating the server (handling the queries, elections, timeout...)
        this->update();
    }
}

// Inline operator to print server data
std::ostream& operator<< (std::ostream& out, const Server& server)
{
    std::map<int, std::string> status_map {{0, "FOLLOWER"}, {1, "CANDIDATE"}, {2, "LEADER"}, {3, "DEAD"}};
    std::map<int, std::string> speed_map {{0, "HIGH"}, {250, "MEDIUM"}, {500, "LOW"}};
    out << "Server rank : " << server._rank << ", Server status : " << status_map.at((int)server._status);
    out << ", Server timeout : " << server._election_timeout << ", Server term : " << server._current_term;
    out << ", Server speed : " << speed_map.at((int)server._server_speed); 
    return out;
}