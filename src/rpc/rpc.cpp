#include "rpc.hpp"

// ========== RCP TypeClass implementation ==========

RPC::RPC(int term, RPC_TYPE rpc_type)
    : _term(term)
    , _rpc_type(rpc_type)
{}

std::string RPC::serialize() const
{
    nlohmann::json json_object;
    json_object["message_type"] = this->_rpc_type;
    json_object["term"] = this->_term;
    json_object["message_content"] = this->serialize_content();
    return json_object.dump();
}