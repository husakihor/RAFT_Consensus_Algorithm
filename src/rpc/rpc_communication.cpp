#include "rpc_communication.hpp"

// ========== Communication functions implementation ==========

// ========== SEND FUNCTIONS ==========

// Function used to send a single RPC to the destination server
void send_message(const RPC& rpc_message, size_t destination, int tag)
{
    MPI_Request request;
    const std::string& serialized_message = rpc_message.serialize();
    MPI_Isend(serialized_message.c_str(), serialized_message.size(), MPI_CHAR, destination, tag, MPI_COMM_WORLD, &request);
    MPI_Request_free(&request);
}

// Here there are some little things to catch : 
// This function is used to send queries to all the n_process processes since the offset
// The source is the rank of the process sending the queries
// The n_process is here to determine the number of ranks from which we want to send the queries
// The offset is here to determine the start rank from which we send the queries
void send_to_all_processes(size_t source, size_t n_process, size_t offset, const RPC& rpc_message, int tag)
{
    for (size_t rank = 0; rank < n_process; rank++)
    {
        // Setting up the real destination rank (as the client_offset is the rank of the last client)
        size_t dest_rank = rank + offset;
        if (source != dest_rank)
        {
            send_message(rpc_message, dest_rank, tag);
        }
    }
}

// ========== RECEIVE FUNCTIONS ==========

// Function used to generate the received query
std::optional<Query> generate_query(const size_t source, const nlohmann::json& json_response) 
{
    RPC::RPC_TYPE message_type = json_response["message_type"].get<RPC::RPC_TYPE>();
    size_t term = json_response["term"].get<size_t>();
    // Migth be std::string for some so be carreful with this one ! (to maybe use in auto)
    nlohmann::json message_content = json_response["message_content"];

    switch (message_type)
    {
        case RPC::RPC_TYPE::HEARTBEAT:
            return std::make_optional<Query>(source, message_type, term, Heartbeat(term, message_content));

        case RPC::RPC_TYPE::VOTE_REQUEST:
            return std::make_optional<Query>(source, message_type, term, VoteRequest(term, message_content));

        case RPC::RPC_TYPE::VOTE_RESPONSE:
            return std::make_optional<Query>(source, message_type, term, VoteResponse(term, message_content));

        case RPC::RPC_TYPE::APPEND_ENTRIES: 
            return std::make_optional<Query>(source, message_type, term, AppendEntries(term, message_content));

        case RPC::RPC_TYPE::APPEND_ENTRIES_RESPONSE: 
            return std::make_optional<Query>(source, message_type, term, AppendEntriesResponse(term, message_content));

        // From this part, the next type of RPC won't have any term type as they are made to be used by the clients

        case RPC::RPC_TYPE::NEW_LOG_ENTRY: 
            return std::make_optional<Query>(source, message_type, term, NewLogEntry(message_content));

        case RPC::RPC_TYPE::NEW_LOG_ENTRY_RESPONSE: 
            return std::make_optional<Query>(source, message_type, term, NewLogEntryResponse(message_content));

        case RPC::RPC_TYPE::SEARCH_LEADER: 
            return std::make_optional<Query>(source, message_type, term, SearchLeader(message_content));

        case RPC::RPC_TYPE::SEARCH_LEADER_RESPONSE: 
            return std::make_optional<Query>(source, message_type, term, SearchLeaderResponse(message_content));

        case RPC::RPC_TYPE::MESSAGE:
            return std::make_optional<Query>(source, message_type, term, Message(message_content));

        case RPC::RPC_TYPE::MESSAGE_RESPONSE:
            return std::make_optional<Query>(source, message_type, term, MessageResponse(message_content));

        default:
            return std::nullopt;
    }
}

// Function used to receive a single message from the source server
std::optional<Query> receive_message(size_t source, int tag)
{
    MPI_Status mpi_status;

    int flag;
    MPI_Iprobe(source, tag, MPI_COMM_WORLD, &flag, &mpi_status);

    if (!flag)
    {
        return std::nullopt;
    }

    int buffer_size = 0;
    MPI_Get_count(&mpi_status, MPI_CHAR, &buffer_size);

    std::vector<char> buffer(buffer_size);
    MPI_Recv(buffer.data(), buffer_size, MPI_CHAR, source, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::string serialized_response(buffer.begin(), buffer.end());

    try 
    {
        nlohmann::json json_response = nlohmann::json::parse(serialized_response);
        return generate_query(source, json_response);
    }
    catch (...)
    {
        return std::nullopt;
    }
}

// Here there are some little things to catch : 
// This function is used to receive queries to all the n_process processes since the offset
// The process_rank is the rank of the process receiving the queries
// The n_process is here to determine the number of ranks from which we want to receive the queries
// The offset is here to determine the start rank from which we receive the queries
void receive_all_messages(size_t process_rank, size_t n_process, size_t offset, std::vector<Query>& queries, int tag)
{
    for (size_t rank = 0; rank < n_process; rank++)
    {
        // Setting up the real source rank (as the client_offset is the rank of the last client)
        size_t source_rank = rank + offset;
        // If the source is the same as the server rank, then skip
        if (source_rank == process_rank)
            continue;

        // Loop used to receive all the message from each server
        while (true)
        {
            std::optional<Query> query = receive_message(source_rank, tag);
            if (query.has_value())
            {
                queries.emplace_back(query.value());
            }
            else
            {
                break;
            }
        }
    }
}