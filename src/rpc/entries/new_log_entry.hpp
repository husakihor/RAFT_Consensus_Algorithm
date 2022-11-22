#pragma once

#include "rpc/rpc.hpp"
#include "log_entry.hpp"

class NewLogEntry : public RPC
{
public:
    NewLogEntry(LogEntry entry);
    NewLogEntry(const nlohmann::json& serialized_json);
    NewLogEntry(const std::string& serialized);

    // Function used to serialize the class as a json to be sent later as a string
    nlohmann::json serialize_content() const override;

    // New LogEntry to add to the logs
    LogEntry _log_entry;
};

class NewLogEntryResponse : public RPC
{
public:
    NewLogEntryResponse(bool success);
    NewLogEntryResponse(const nlohmann::json& serialized_json);
    NewLogEntryResponse(const std::string& serialized);

    // Function used to serialize the class as a json to be sent later as a string
    nlohmann::json serialize_content() const override;

    // Reponse True if the entry has been added to the logs
    const bool _success;
};