#include "message.hpp"

// ========== Message class implementation ==========

// Setting up the term to -1 as this is meant to be used to send message to the server for the clients
Message::Message(MESSAGE_TYPE message_type, std::string message_content) 
    : RPC(-1, RPC::RPC_TYPE::MESSAGE), _type(message_type), _content(message_content)
{}

Message::Message(const nlohmann::json& serialized_json)
    : Message(serialized_json["type"], serialized_json["content"])
{}

Message::Message(const std::string& serialized) 
    : Message(nlohmann::json::parse(serialized))
{}

nlohmann::json Message::serialize_content() const
{
    nlohmann::json json_object;
    json_object["type"] = this->_type;
    json_object["content"] = this->_content;
    return json_object;
}

// ========== MessageResponse class implementation ==========

// Setting up the term to -1 as this is the response to the message and the term of the server won't be of any use for the client
MessageResponse::MessageResponse(const bool success) 
    : RPC(-1, RPC::RPC_TYPE::MESSAGE_RESPONSE), _success(success)
{}

MessageResponse::MessageResponse(const nlohmann::json& serialized_json) 
    : MessageResponse(serialized_json["success"].get<bool>())
{}

MessageResponse::MessageResponse(const std::string& serialized) 
    : MessageResponse(nlohmann::json::parse(serialized))
{}

nlohmann::json MessageResponse::serialize_content() const
{
    nlohmann::json json_object;
    json_object["success"] = this->_success;
    return json_object;
}