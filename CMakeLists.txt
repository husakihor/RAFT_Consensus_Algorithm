cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(algorep CXX)
enable_language(C)
enable_language(Fortran)
find_package(MPI REQUIRED)
add_definitions(-DOMPI_SKIP_MPICXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")

include_directories(src ${MPI_INCLUDE_PATH})
include_directories(server_logs)
include_directories(client_commands)

add_executable(algorep
	src/main.cpp
	src/server/server.cpp
	src/client/client.cpp
	src/repl_controller/repl_contoller.cpp
	src/clock/clock.cpp
	src/message/message.cpp
	src/rpc/rpc.cpp
	src/rpc/rpc_communication.cpp
	src/rpc/query/query.cpp
	src/rpc/entries/append_entries.cpp
	src/rpc/entries/new_log_entry.cpp
	src/rpc/entries/log_entry.cpp
	src/rpc/vote/request_vote.cpp
	src/rpc/heartbeat/heartbeat.cpp
	src/rpc/leader/search_leader.cpp
	src/utils/json.hpp)

target_link_libraries(algorep ${MPI_LIBRARIES})
set_target_properties(algorep PROPERTIES
	VS_DEBUGGER_COMMAND "\$(MSMPI_BIN)mpiexec"
	VS_DEBUGGER_COMMAND_ARGUMENTS "-n 3 \"\$(TargetPath)\"")