#include "client.hpp"

// Include the file where all the communication functions are
#include "rpc/rpc_communication.hpp"

// ========== Constructor function ==========

Client::Client(int rank, int server_count, int client_count) :
    _rank(rank),
    _is_stopped(false),
    _leader_rank(0),
    _entries_to_send(),
    _entry_sent(true),
    _entry_clock(),
    _client_count(client_count),
    _server_count(server_count),
    _has_started(false)
{
    // Setting the client status as DEAD as this is his initializing
    this->_status = ClientStatus::DEAD;
    // Setting the client speed as MEDIUM
    this->_client_speed = Speed::MEDIUM;
    // Setting the timeout as 100
    this->_timeout = 100;

    // Setting up the two clocks of the client
    this->_leader_clock = Clock();
    this->_entry_clock = Clock();

    // Creating the log path of the server
    this->_commands_filepath = "client_commands/commands_client_" + std::to_string(rank) + ".txt";

    // Initializing the log file of the server
    std::ifstream commands_file;
    commands_file.open(this->_commands_filepath);
    if (!commands_file.good())
    {
        std::cerr << "Unable to open client commands file !" << std::endl;
    }
    if (!(commands_file.peek() == std::ifstream::traits_type::eof()))
    {
        // Parsing the commands of the client file
        parse_commands_file();
    }
    else
    {
        std::cerr << "Client " << this->_rank << " commands file is empty" << std::endl;
    }
    commands_file.close();
}

// ========== Parsing functions ==========

void Client::parse_commands_file()
{
    // Opening the client file and reading the logs inside
    std::ifstream commands_file(this->_commands_filepath);

    if (commands_file.is_open())
    {
        for (std::string line; std::getline(commands_file, line);)
        {
            this->_entries_to_send.emplace(LogEntry(-1, line));
        }
    }
}

void Client::parse_new_command_file(std::string filename) 
{
    // Opening the given file and reading the logs inside
    std::ifstream commands_file(filename);

    if (commands_file.is_open())
    {
        for (std::string line; std::getline(commands_file, line);)
        {
            this->_entries_to_send.emplace(LogEntry(-1, line));
        }
    }
}

// ========== Handling queries functions ==========

void Client::handle_queries(const std::vector<Query>& queries)
{
    for (const Query& query : queries)
    {
        switch (query._type)
        {
            case RPC::RPC_TYPE::MESSAGE:
            {
                // Calling the function to handle the messages depending on their type
                this->handle_message(query);
                break;
            } 
            case RPC::RPC_TYPE::SEARCH_LEADER_RESPONSE:
            {
                const SearchLeaderResponse& leader_response = std::get<SearchLeaderResponse>(query._content);
                // If we don't already have a leader, then set the new leader's rank
                if (this->_leader_rank == 0)
                {
                    this->_leader_rank = leader_response._leader_rank;
                    this->_leader_clock.reset();
                    // Also reset the entry timeout to make sure that it won't reset the leader instantly
                    this->_entry_clock.reset();
                }
                break;
            }
            case RPC::RPC_TYPE::NEW_LOG_ENTRY_RESPONSE:
            {
                const NewLogEntryResponse& entriesResponse = std::get<NewLogEntryResponse>(query._content);
                // Checking if the new log entries has been received correctly
                if (entriesResponse._success)
                {
                    this->_entries_to_send.pop();
                }
                // If not and the leader is not set up, then reset it
                else if (this->_leader_rank != 0)
                {
                    this->_leader_rank = 0;
                    this->_leader_clock.reset();
                }
                // Set up the entries as sent 
                this->_entry_sent = true;
                this->_entry_clock.reset();
                break;
            }
            default:
                break;
        }
    }
}

// For the messages handling the std::cerr will be commented only to have an easier time showing some examples
// In this case, we will uncomment them to show the REPL queries handling
void Client::handle_message(const Query& query)
{
    // Boolean describing the status of the parsing (success or failure)
    bool parsing_message_status = true;

    const Message& message = std::get<Message>(query._content);
    switch (message._type)
    {
        case Message::MESSAGE_TYPE::CLIENT_CREATE_NEW_ENTRY:
        {
            // Setting up the term as -1 as this will come from a client so it wont have a term
            // std::cout << "Client " << this->_rank << " received and added new entry." << std::endl;
            this->_entries_to_send.emplace(NewLogEntry(LogEntry(-1, message._content)));
            break;
        }
        case Message::MESSAGE_TYPE::CLIENT_NEW_FILE_ENTRY:
        {
            // std::cout << "Client " << this->_rank << " received a new entry file and added all the new entries." << std::endl;
            this->parse_new_command_file(message._content);
            break;
        }
        case Message::MESSAGE_TYPE::PROCESS_SET_SPEED:
        {
            // Setting up the client speed (delay during what the server will way before each update)
            if (message._content == "low")
            {
                this->_client_speed = Speed::LOW;
            } 
            else if (message._content == "medium")
            {
                this->_client_speed = Speed::MEDIUM;
            }
            // Not checking the speed here, as we will set it as HIGH by default if it is not LOW or MEDIUM
            else 
            {
                this->_client_speed = Speed::HIGH;
            }
            break;
        }
        case Message::MESSAGE_TYPE::PROCESS_RECOVER:
        {
            if (this->_status == ClientStatus::DEAD)
            {
                this->_status = ClientStatus::RUNNING;
                // Reseting the leader to send a search to have to current one
                if (this->_leader_rank != 0)
                {
                    this->_leader_rank = 0;
                    this->_leader_clock.reset();
                }
            }
            else 
            {
                std::cerr << "Client " << this->_rank << " is already up and running." << std::endl;
                parsing_message_status = false;
            }
            break;
        }
        case Message::MESSAGE_TYPE::PROCESS_START:
        {
            if (!this->_has_started)
            {
                // Setting the variable determining if we already started
                this->_has_started = true;
                this->_status = ClientStatus::RUNNING;
                // Init again just in case 
                this->_leader_rank = 0;
                this->_leader_clock.reset();
                this->_entry_clock.reset();
            }
            else 
            {                
                std::cerr << "Client " << this->_rank << " has already started. It migh have crashed. Use process recover to make it run again." << std::endl;
                parsing_message_status = false;
            }
            break;
        }
        case Message::MESSAGE_TYPE::PROCESS_CRASH:
        {
            if (this->_status == ClientStatus::RUNNING)
            {
                this->_status = ClientStatus::DEAD;
                // Reseting the variables 
                this->_leader_rank = 0;
                this->_leader_clock.reset();
                this->_entry_clock.reset();
            }
            else 
            {
                std::cerr << "Client " << this->_rank << " has already crashed. It cannot crash a second time." << std::endl;
                parsing_message_status = false;
            }
            break;
        }
        case Message::MESSAGE_TYPE::PROCESS_STOP:
        {
            // May make MPI crash, to test
            // std::cerr << "Client " << this->_rank << " has stopped. It will not receive any commands now." << std::endl;
            this->_is_stopped = true;
            this->_status = ClientStatus::DEAD;
            break;
        }
        case Message::MESSAGE_TYPE::PROCESS_DISPLAY:
        {
            std::map<int, std::string> status_map {{0, "RUNNING"}, {1, "DEAD"}};
            std::map<int, std::string> speed_map {{0, "HIGH"}, {250, "MEDIUM"}, {500, "LOW"}};
            std::cerr << "Client " << this->_rank << " has the status " << status_map.at((int)this->_status) << " and his speed is " << speed_map.at((int)this->_client_speed) << std::endl;
            break;
        }
        default:
            parsing_message_status = false;
            break;
    }
    // Sending the message response with his status
    MessageResponse messageResponse = MessageResponse(parsing_message_status);
    send_message(messageResponse, query._source_rank, 0);
}

// ========== Main functions ==========

void Client::update() 
{
    // Handle queries
    std::vector<Query> received_queries;
    // For the clients, as they can reveice queries from the controler, clients or servers, we receive from all the possible ranks
    receive_all_messages(this->_rank, (this->_server_count + this->_client_count + 1), 0, received_queries, 0);
    this->handle_queries(received_queries);

    if (this->_status != ClientStatus::DEAD)
    {
        // Search for leader or send next entry
        if (this->_leader_rank == 0)
        {
            // Check if the client has timeout
            if (this->_leader_clock.check() > this->_timeout) 
            {
                // Creating the query to get the leader and sending it to all the servers
                SearchLeader searchLeader = SearchLeader(this->_leader_rank);
                send_to_all_processes(this->_rank, this->_server_count, this->_client_count + 1, searchLeader, 0);
                this->_leader_clock.reset();
            }
        }
        // If we have a valid leader, then send the next entry if there are to send
        else if (this->_entry_sent && !(this->_entries_to_send.empty()))
        {
            // Getting the next entry to send
            const NewLogEntry& newLogEntry = this->_entries_to_send.front();
            send_message(newLogEntry, this->_leader_rank, 0);
            // Reseting the clock for entries and the verification 
            this->_entry_sent = false;
            this->_entry_clock.reset();
        }

        // Check if the new log entry has been sent correctly
        if (!this->_entry_sent && !(this->_entries_to_send.empty()))
        {
            if (this->_entry_clock.check() > this->_timeout)
            {
                // If the entry response took too much time to come, set it as sent to make the new leader get it again
                this->_entry_sent = true;
                // Then if the leader is not set, reset him
                if (this->_leader_rank != 0)
                {
                    this->_leader_rank = 0;
                    this->_leader_clock.reset();
                    this->_entry_clock.reset();
                }
            }
        }
    }
}

void Client::run_client()
{
    while (!this->_is_stopped)
    {
        // Waiting depending on the speed of the client
        Clock::wait((int)this->_client_speed);

        // Updating the client (handling the queries, leader search, timeout...)
        this->update();
    }
}

// Inline operator to print server data
std::ostream& operator<< (std::ostream& out, const Client& client)
{
    std::map<int, std::string> status_map {{0, "RUNNING"}, {1, "DEAD"}};
    std::map<int, std::string> speed_map {{0, "HIGH"}, {250, "MEDIUM"}, {500, "LOW"}};
    out << "Client rank : " << client._rank << ", client status : " << status_map.at((int)client._status);
    out << ", client timeout : " << client._timeout << ", client speed : " << speed_map.at((int)client._client_speed);
    return out;
}