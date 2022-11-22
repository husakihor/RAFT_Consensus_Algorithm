#pragma once

#include "rpc/rpc.hpp"

class SearchLeader : public RPC
{
public:
    SearchLeader(const int leader_rank);
    SearchLeader(const nlohmann::json& serialized_json);
    SearchLeader(const std::string& serialized);

    // Function used to serialize the class as a json to be sent later as a string
    nlohmann::json serialize_content() const override;

    // Rank of the last known leader before sending that query
    int _leader_rank;
};

class SearchLeaderResponse : public RPC
{
public:
    SearchLeaderResponse(const int leader_rank);
    SearchLeaderResponse(const nlohmann::json& serialized_json);
    SearchLeaderResponse(const std::string& serialized);

    // Function used to serialize the class as a json to be sent later as a string
    nlohmann::json serialize_content() const override;

    // Rank of the current leader
    const int _leader_rank;
};