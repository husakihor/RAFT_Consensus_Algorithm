#pragma once

#include <string>
#include <utility>

#include "utils/json.hpp"
#include "rpc/rpc.hpp"

class LogEntry
{
public:
    LogEntry(int term, std::string command);
    LogEntry(const nlohmann::json& serialized_json);
    LogEntry(const std::string& serialized);

    nlohmann::json serialize_content() const;

    // The term of the server when handling the log entry
    const int _term;
    // The command of the log entry
    const std::string _command;
};