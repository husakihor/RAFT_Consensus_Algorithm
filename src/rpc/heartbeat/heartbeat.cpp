#include "heartbeat.hpp"

// ========== Heartbeat class implementation ==========

Heartbeat::Heartbeat(int term, size_t leader_rank, int prev_log_index, int prev_log_term, int leader_commit)
    : RPC(term, RPC::RPC_TYPE::HEARTBEAT), 
      _leader_rank(leader_rank),
      _prev_log_index(prev_log_index), 
      _prev_log_term(prev_log_term), 
      _leader_commit(leader_commit)
{}

Heartbeat::Heartbeat(int term, const nlohmann::json& serialized_json)
    : RPC(term, RPC::RPC_TYPE::HEARTBEAT), 
      _leader_rank(serialized_json["leader_rank"]),
      _prev_log_index(serialized_json["prev_log_index"]), 
      _prev_log_term(serialized_json["prev_log_term"]), 
      _leader_commit(serialized_json["leader_commit"])
{}

Heartbeat::Heartbeat(int term, const std::string& serialized) 
    : Heartbeat(term, nlohmann::json::parse(serialized))
{}

nlohmann::json Heartbeat::serialize_content() const
{
    nlohmann::json json_object;
    json_object["leader_rank"] = this->_leader_rank;
    json_object["prev_log_index"] = this->_prev_log_index;
    json_object["prev_log_term"] = this->_prev_log_term;
    json_object["leader_commit"] = this->_leader_commit;
    return json_object;
}