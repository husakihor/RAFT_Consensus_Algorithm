#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>


#include "mpi.h"
#include "message/message.hpp"
#include "clock/clock.hpp"
#include "rpc/query/query.hpp"
#include "rpc/rpc_communication.hpp"

class ReplController
{
public:
    ReplController(size_t nb_servers, size_t nb_clients);

    // Run function for the REPL controller (main loop and user entry)
    void run_repl_controller();

private:
    // Displays functions
    void display_help();
    void display_processes_informations();

    // Sending commands functions 
    void send_and_wait(int destination, const Message::MESSAGE_TYPE messageType, std::string command);
    void send_set_speed(const std::vector<std::string>& command);
    void send_client_start(const std::vector<std::string>& command);
    void send_process_crash(const std::vector<std::string>& command);
    void send_process_recover(const std::vector<std::string>& command);
    void send_new_log_entry(const std::vector<std::string>& command);
    void send_new_files_entries(const std::vector<std::string>& command);
    void send_timeout(const std::vector<std::string>& command);
    void send_stop_process(const std::vector<std::string>& command);
    void send_display_process(const std::vector<std::string>& command);

    // Parsing functions 
    void parse_user_entry(std::string input, std::vector<std::string>& command);
    void parse_command(std::vector<std::string>& command);

    // Stop function
    void stop_all_processes();

    // ===== ReplController class privates variables =====

    // ReplController usefull variables
    float _timeout;
    Clock _clock;
    size_t _nb_servers;
    size_t _nb_clients;
};