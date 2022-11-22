#include "log_entry.hpp"

// ========== LogEntry class implementation ==========

LogEntry::LogEntry(int term, std::string command) 
    : _term(term), _command(command)
{}

LogEntry::LogEntry(const nlohmann::json& serialized_json) 
    : LogEntry(serialized_json["term"], serialized_json["command"])
{}

LogEntry::LogEntry(const std::string& serialized) 
    : LogEntry(nlohmann::json::parse(serialized))
{}

nlohmann::json LogEntry::serialize_content() const
{
    nlohmann::json json_object;
    json_object["term"] = this->_term;
    json_object["command"] = this->_command;
    return json_object;
}