#include "new_log_entry.hpp"

// ========== NewLogEntry class implementation ==========

// Setting up the term to -1 as this is the response to the message and the term of the server won't be of any use for the client
NewLogEntry::NewLogEntry(LogEntry log_entry) 
    : RPC(-1, RPC::RPC_TYPE::NEW_LOG_ENTRY), _log_entry(log_entry)
{}

NewLogEntry::NewLogEntry(const nlohmann::json& serialized_json) 
    : NewLogEntry(LogEntry(serialized_json["log_entry"]))
{}

NewLogEntry::NewLogEntry(const std::string& serialized) 
    : NewLogEntry(nlohmann::json::parse(serialized))
{}

nlohmann::json NewLogEntry::serialize_content() const
{
    nlohmann::json json_object;
    json_object["log_entry"] = this->_log_entry.serialize_content();
    return json_object;
}

// ========== NewLogEntryResponse class implementation ==========

// Setting up the term to -1 as this is the response to the message and the term of the server won't be of any use for the client
NewLogEntryResponse::NewLogEntryResponse(bool success) 
    : RPC(-1, RPC::RPC_TYPE::NEW_LOG_ENTRY_RESPONSE), _success(success)
{}

NewLogEntryResponse::NewLogEntryResponse(const nlohmann::json& serialized_json) 
    : NewLogEntryResponse(serialized_json["success"].get<bool>())
{}

NewLogEntryResponse::NewLogEntryResponse(const std::string& serialized) 
    : NewLogEntryResponse(nlohmann::json::parse(serialized))
{}

nlohmann::json NewLogEntryResponse::serialize_content() const
{
    nlohmann::json json_object;
    json_object["success"] = this->_success;
    return json_object;
}