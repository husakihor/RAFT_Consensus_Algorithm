#include "search_leader.hpp"

// ========== SearchLeader class implementation ==========

// Setting the term to -1 as this is not a request that will be used by servers
SearchLeader::SearchLeader(const int leader_rank) 
    : RPC(-1, RPC::RPC_TYPE::SEARCH_LEADER), _leader_rank(leader_rank) 
{}

SearchLeader::SearchLeader(const nlohmann::json& serialized_json) 
    : SearchLeader(serialized_json["leader_rank"].get<int>())
{}

SearchLeader::SearchLeader(const std::string& serialized) 
    : SearchLeader(nlohmann::json::parse(serialized))
{}

nlohmann::json SearchLeader::serialize_content() const
{
    nlohmann::json json_object;
    json_object["leader_rank"] = this->_leader_rank;
    return json_object;
}

// ========== SearchLeaderResponse class implementation ==========

// Setting the term to -1 as this is not a request that will be used by servers
SearchLeaderResponse::SearchLeaderResponse(const int leader_rank) 
    : RPC(-1, RPC::RPC_TYPE::SEARCH_LEADER_RESPONSE), _leader_rank(leader_rank)
{}

SearchLeaderResponse::SearchLeaderResponse(const nlohmann::json& serialized_json) 
    : SearchLeaderResponse(serialized_json["leader_rank"].get<int>())
{}

SearchLeaderResponse::SearchLeaderResponse(const std::string& serialized) 
    : SearchLeaderResponse(nlohmann::json::parse(serialized))
{}

nlohmann::json SearchLeaderResponse::serialize_content() const
{
    nlohmann::json json_object;
    json_object["leader_rank"] = this->_leader_rank;
    return json_object;
}