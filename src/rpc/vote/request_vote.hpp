#pragma once

#include "rpc/rpc.hpp"

class VoteRequest : public RPC
{
public:
    VoteRequest(int server_term, size_t candidate_rank, size_t last_log_index, int last_log_term);
    VoteRequest(int server_term, const nlohmann::json& serialized_json);
    VoteRequest(int server_term, const std::string& serialized);

    // Function used to serialize the class as a json to be sent later as a string
    nlohmann::json serialize_content() const override;

    // Rank of the candidate requesting vote
    const size_t _candidate_rank;
    // Index of the last log of the candidate
    const size_t _last_log_index;
    // Term of the last log of the candidate
    const int _last_log_term;
};

class VoteResponse : public RPC
{
public:
    VoteResponse(int server_term, bool vote);
    VoteResponse(int server_term, const nlohmann::json& serialized_json);
    VoteResponse(int server_term, const std::string& serialized);

    // Function used to serialize the class as a json to be sent later as a string
    nlohmann::json serialize_content() const override;

    // If this is True, this means candidate gives the vote
    const bool _vote;
};