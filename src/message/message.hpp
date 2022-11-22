#pragma once

#include "rpc/rpc.hpp"

class Message : public RPC
{
public:
    // Enum for the type of the message
    enum MESSAGE_TYPE
    {
        CLIENT_CREATE_NEW_ENTRY,
        CLIENT_NEW_FILE_ENTRY,
        SERVER_TIMEOUT,
        PROCESS_DISPLAY,
        PROCESS_SET_SPEED,
        PROCESS_CRASH,
        PROCESS_RECOVER,
        PROCESS_STOP,
        PROCESS_START,
    };

    Message(MESSAGE_TYPE message_type, std::string message_content);
    Message(const nlohmann::json& serialized_json);
    Message(const std::string& serialized);

    nlohmann::json serialize_content() const override;

    // Type of the message
    const MESSAGE_TYPE _type;
    // Content of the message
    const std::string _content;
};

class MessageResponse : public RPC
{
public:
    MessageResponse(bool success);
    MessageResponse(const nlohmann::json& serialized_json);
    MessageResponse(const std::string& serialized);

    nlohmann::json serialize_content() const override;

    // Response to the message depending if the actions that the message asked to do is done
    bool _success;
};