#pragma once

#include <vector>

#include "rpc/rpc.hpp"
#include "rpc/entries/log_entry.hpp"

class AppendEntries : public RPC
{
public:
    AppendEntries(int term, size_t leader_rank, int prev_log_index, int prev_log_term, std::vector<LogEntry> entries, int leader_commit);
    AppendEntries(int term, const nlohmann::json& serialized_json);
    AppendEntries(int term, const std::string& serialized);

    // Function used to serialize the class as a json to be sent later as a string
    nlohmann::json serialize_content() const override;

    // so follower can redirect clients
    const size_t _leader_rank;
    // index of log entry immediately preceding new ones
    const int _prev_log_index;
    // term of prev_log_index entry
    const int _prev_log_term;
    // log entries to store (empty for heartbeat may send more than one for efficiency)
    std::vector<LogEntry> _entries;
    // leader's commit_index
    const int _leader_commit;
};

class AppendEntriesResponse : public RPC
{
public:
    AppendEntriesResponse(int term, bool success);
    AppendEntriesResponse(int term, const nlohmann::json& serialized_json);
    AppendEntriesResponse(int term, const std::string& serialized);

    // Function used to serialize the class as a json to be sent later as a string
    nlohmann::json serialize_content() const override;

    // True if the follower has a log index matching the prev_log_index of the leader and a term matchin the prev_log_term of the leader
    const bool _success;
};