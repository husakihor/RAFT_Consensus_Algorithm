#pragma once

#include <variant>

#include "rpc/rpc.hpp"
#include "message/message.hpp"
#include "rpc/heartbeat/heartbeat.hpp"
#include "rpc/vote/request_vote.hpp"
#include "rpc/leader/search_leader.hpp"
#include "rpc/entries/append_entries.hpp"
#include "rpc/entries/new_log_entry.hpp"

// ========== Query Class ==========

class Query
{
public:
    // Using this type for easier reading 
    // Here we are using variant as the type used here will be one of those passed in parameters
    using request_content = std::variant<Heartbeat,
                                         VoteRequest, VoteResponse,
                                         AppendEntries, AppendEntriesResponse,
                                         NewLogEntry, NewLogEntryResponse,
                                         SearchLeader, SearchLeaderResponse,
                                         Message, MessageResponse
                                        >;
    
    Query(size_t source_rank, RPC::RPC_TYPE type, int term, request_content content);

    // Rank of the source server Source rank
    const size_t _source_rank;
    // Type of the Query
    const RPC::RPC_TYPE _type;
    // Term of server when sending the Query
    const int _term;
    // Content of the Query
    const request_content _content;
};