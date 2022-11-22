#pragma once

#include <iostream>
#include <optional>

#include "mpi.h"
#include "query/query.hpp"
#include "clock/clock.hpp"

// ========== Communication functions implementation ==========

// Send functions
void send_message(const RPC& rpc_message, size_t destination, int tag);
void send_to_all_processes(size_t source, size_t n_servers, size_t clients_offset, const RPC& rpc_message, int tag);

// Generate Query 
std::optional<Query> generate_query(const size_t source, const nlohmann::json& json_response);

// Receive functions
std::optional<Query> receive_message(size_t source, int tag);
void receive_all_messages(size_t server_rank, size_t n_servers, size_t clients_offset, std::vector<Query>& queries, int tag);