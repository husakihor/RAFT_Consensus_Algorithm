#include "append_entries.hpp"

// ========== AppendEntries class implementation ==========

AppendEntries::AppendEntries(int term, size_t leader_rank, int prev_log_index, int prev_log_term, std::vector<LogEntry> entries, int leader_commit)
    : RPC(term, RPC::RPC_TYPE::APPEND_ENTRIES), 
      _leader_rank(leader_rank),
      _prev_log_index(prev_log_index), 
      _prev_log_term(prev_log_term), 
      _entries(entries), 
      _leader_commit(leader_commit)
{}

AppendEntries::AppendEntries(int term, const nlohmann::json& serialized_json)
    : RPC(term, RPC::RPC_TYPE::APPEND_ENTRIES), 
      _leader_rank(serialized_json["leader_rank"]),
      _prev_log_index(serialized_json["prev_log_index"]), 
      _prev_log_term(serialized_json["prev_log_term"]), 
      _leader_commit(serialized_json["leader_commit"])
{
    for (const auto& entry : serialized_json["entries"])
        this->_entries.push_back(entry);
}

AppendEntries::AppendEntries(int term, const std::string& serialized) 
    : AppendEntries(term, nlohmann::json::parse(serialized))
{}

nlohmann::json AppendEntries::serialize_content() const
{
    nlohmann::json json_object;
    json_object["leader_rank"] = this->_leader_rank;
    json_object["prev_log_index"] = this->_prev_log_index;
    json_object["prev_log_term"] = this->_prev_log_term;
    json_object["entries"] = nlohmann::json::array();
    for (LogEntry entry : this->_entries)
    {
        json_object["entries"].push_back(entry.serialize_content());
    }
    json_object["leader_commit"] = this->_leader_commit;
    return json_object;
}

// ========== AppendEntriesResponse class implementation ==========

AppendEntriesResponse::AppendEntriesResponse(int term, bool success) 
    : RPC(term, RPC::RPC_TYPE::APPEND_ENTRIES_RESPONSE), _success(success)
{}

AppendEntriesResponse::AppendEntriesResponse(int term, const nlohmann::json& serialized_json)
    : AppendEntriesResponse(term, serialized_json["success"].get<bool>())
{}

AppendEntriesResponse::AppendEntriesResponse(int term, const std::string& serialized)
    : AppendEntriesResponse(term, nlohmann::json::parse(serialized))
{}

nlohmann::json AppendEntriesResponse::serialize_content() const
{
    nlohmann::json json_object;
    json_object["success"] = this->_success;
    return json_object;
}