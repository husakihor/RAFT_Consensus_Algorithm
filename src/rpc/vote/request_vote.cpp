#include "request_vote.hpp"

// ========== RequestVote class implementation ==========

VoteRequest::VoteRequest(int server_term, size_t candidate_rank, size_t last_log_index, int last_log_term)
    : RPC(server_term, RPC::RPC_TYPE::VOTE_REQUEST), 
      _candidate_rank(candidate_rank), 
      _last_log_index(last_log_index), 
      _last_log_term(last_log_term)
{}

VoteRequest::VoteRequest(int server_term, const nlohmann::json& serialized_json)
    : VoteRequest(server_term, serialized_json["candidate_rank"].get<size_t>(), 
                               serialized_json["last_log_index"].get<size_t>(), 
                               serialized_json["last_log_term"].get<int>())
{}

VoteRequest::VoteRequest(int server_term, const std::string& serialized) 
    : VoteRequest(server_term, nlohmann::json::parse(serialized))
{}

nlohmann::json VoteRequest::serialize_content() const
{
    nlohmann::json json_object;
    json_object["candidate_rank"] = this->_candidate_rank;
    json_object["last_log_index"] = this->_last_log_index;
    json_object["last_log_term"] = this->_last_log_term;
    return json_object;
}

// ========== RequestVoteResponse class implementation ==========

VoteResponse::VoteResponse(int term, bool vote)
    : RPC(term, RPC::RPC_TYPE::VOTE_RESPONSE), _vote(vote)
{}

VoteResponse::VoteResponse(int term, const nlohmann::json& serialized_json)
    : VoteResponse(term, serialized_json["vote"].get<bool>())
{}

VoteResponse::VoteResponse(int term, const std::string& serialized)
    : VoteResponse(term, nlohmann::json::parse(serialized))
{}

nlohmann::json VoteResponse::serialize_content() const
{
    nlohmann::json json_object;
    json_object["vote"] = this->_vote;
    return json_object;
}