#pragma once

#include <string>
#include <utility>

#include "utils/json.hpp"

// ========== RCP TypeClass ==========

class RPC
{
public:
    // Enum used to determine the type of RPC used 
    enum RPC_TYPE
    {
        HEARTBEAT,
        VOTE_REQUEST, VOTE_RESPONSE,
        APPEND_ENTRIES, APPEND_ENTRIES_RESPONSE,
        NEW_LOG_ENTRY, NEW_LOG_ENTRY_RESPONSE,
        SEARCH_LEADER, SEARCH_LEADER_RESPONSE,
        MESSAGE, MESSAGE_RESPONSE,
        STATE_REQUEST, STATE_RESPONSE,
    };

    // RPC Constructor with the term of the server and the type of RPC
    RPC(int term, RPC_TYPE rpc_type);

    // Function used to serialize the RPC to a string ready to be sent to other servers
    std::string serialize() const;

    // Virtual class that the inheritant classes will have to implement to parse their content
    virtual nlohmann::json serialize_content() const = 0;

    // term of the RPC
    const int _term;
    // Type of the rpc
    const RPC_TYPE _rpc_type;
};