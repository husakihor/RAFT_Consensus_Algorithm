#pragma once

#include <vector>

#include "rpc/rpc.hpp"

class Heartbeat : public RPC
{
public:
    Heartbeat(int term, size_t leader_rank, int prev_log_index, int prev_log_term, int leader_commit);
    Heartbeat(int term, const nlohmann::json& serialized_json);
    Heartbeat(int term, const std::string& serialized);

    // Function used to serialize the class as a json to be sent later as a string
    nlohmann::json serialize_content() const override;

    // This is the same data as AppendEntries (we just have no entries)
    // This is to make sure that the server receiving the heartbeat is up to date
    const size_t _leader_rank;
    const int _prev_log_index;
    const int _prev_log_term;
    const int _leader_commit;
};