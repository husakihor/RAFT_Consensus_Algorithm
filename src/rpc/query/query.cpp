#include "query.hpp"

// ========== Query Class implementation ==========

Query::Query(size_t source_rank, RPC::RPC_TYPE type, int term, request_content content)
    : _source_rank(source_rank), 
      _type(type), _term(term), 
      _content(content)
{}