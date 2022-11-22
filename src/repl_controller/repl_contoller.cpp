#include "repl_contoller.hpp"

ReplController::ReplController(size_t nb_servers, size_t nb_clients):
    _timeout(500), _nb_servers(nb_servers), _nb_clients(nb_clients)
{}

// ========== Display functions ==========

void ReplController::display_help()
{
    std::cerr << "====================================== HELP ======================================\n" << std::endl;
    std::cerr << "This is the help in case if you forgot how the commands are working.\n" << std::endl;
    std::cerr << "- help => this function displays this text to help you know how to use all the commands.\n" << std::endl;
    std::cerr << "- process_informations => this function displays the ranks for all the processes that are running.\n" << std::endl;
    std::cerr << "- set_speed {client_rank} {speed} => this command is used to change the speed of the process. His parameters are : " << std::endl;
    std::cerr << "\t- low (500 milliseconds of delay)" << std::endl;
    std::cerr << "\t- medium (250 milliseconds of delay)" << std::endl;
    std::cerr << "\t- high (0 milliseconds of delay)\n" << std::endl;
    std::cerr << "- start_client {client_rank} => this command is used to start a client (as they have the status \"DEAD\" at the beggining).\n" << std::endl;
    std::cerr << "- crash_process {process_rank} => this command is used to make crash a process. By doing that it will only receive and treat commands from the repl controller.\n" << std::endl;
    std::cerr << "- recover_process {process_rank} => very similar to the last command, this command is used to make a process receive commands from other process again and catch up the missing logs." << std::endl;
    std::cerr << "- add_log_entry {client_rank} {log_entry} => this command is used to send a new log entry to the client that will after send it to the servers. This log entry is the line that will be added at the end of his queue." << std::endl;
    std::cerr << "- add_files_entries {client_rank} {files_list} => this function add all the passed files (you need to pass their filepath depending on where you are executing the program) into the given client log entries." << std::endl;
    std::cerr << "- timeout_server {server_rank} => this command is used to force a server to timeout. This will force an election and a change of leader in the servers." << std::endl;
    std::cerr << "- stop_process {process_rank} => this command is used to stop a process. It will not be able to receive any commands after this one." << std::endl;
    std::cerr << "- display_process {process_rank} => this command is used to display the informations of the process.\n" << std::endl;
    std::cerr << "- stop_all => this command is used to stop all the processes that are currently running.\n" << std::endl;
}

void ReplController::display_processes_informations()
{
    std::cerr << "================================= PROCESSES INFORMATIONS =================================\n" << std::endl;
    std::cerr << "Repl Controller rank is : 0 (this is the process that you are using to send commands)." << std::endl;
    std::cerr << "The clients ranks are from 1 to " << this->_nb_clients << "." << std::endl;
    std::cerr << "The servers ranks are from " << this->_nb_clients + 1 << " to " << this->_nb_clients + this->_nb_servers << std::endl;
}

// ========== Send functions ==========

void ReplController::send_and_wait(int destination, const Message::MESSAGE_TYPE messageType, std::string command)
{
    // Generating the message to send
    Message message = Message(messageType, command);

    // Sending the message and reseting the clock to avoid a false timeout
    send_message(message, destination, 0);
    this->_clock.reset();

    // Waiting for the timeout to run out
    while (this->_clock.check() < this->_timeout)
    {
        // Trying to receive the response
        std::optional<Query> query = receive_message(destination, 0);
        if (query.has_value())
        {
            const MessageResponse& response = std::get<MessageResponse>(query.value()._content);
            if (response._success)
            {
                std::cerr << "Command has been successfuly executed." << std::endl;
            }
            else 
            {
                std::cerr << "Error while trying to send the message, please try again" << std::endl;
            }
            return;
        }
    }

    // We have timeout so we return a nullopt
    std::cerr << "Error while trying to send the message, please try again" << std::endl;
}

void ReplController::send_set_speed(const std::vector<std::string>& command)
{
    if (command.size() == 3)
    {
        // Parsing the rank of the process
        int process_rank = std::stoi(command[1]);
        // Checking if the indicated rank is a server or a client (and not off limits)
        if ((process_rank >= 1) && (process_rank <= (this->_nb_clients + this->_nb_servers)))
        {
            // Checking if the speed indicated is a good one
            std::string process_speed = command[2];
            if (process_speed == "low" || process_speed == "medium" || process_speed == "high")
            {
                // Sending the message
                this->send_and_wait(process_rank, Message::MESSAGE_TYPE::PROCESS_SET_SPEED, process_speed);
            }
            else 
            {
                std::cerr << "Warning : invalid speed : " << command[2] << std::endl;
            }
        }
        else
        {
            std::cerr << "Warning : invalid rank : " << command[1] << std::endl;
        }        
    }
    else 
    {
        std::cerr << "Warning : invalid number of arguments" << std::endl;
    }
}

void ReplController::send_client_start(const std::vector<std::string>& command)
{
    if (command.size() == 2)
    {
        // Parsing the rank of the process
        int process_rank = std::stoi(command[1]);
        // Checking if the process is a client
        if ((process_rank >= 1) && (process_rank <= this->_nb_clients))
        {
            // Send the start command to the client (sending empty content as we don't need one)
            this->send_and_wait(process_rank, Message::MESSAGE_TYPE::PROCESS_START, "");
        }
        else 
        {
            std::cerr << "Warning : invalid rank : " << command[1] << " (this must be the rank of a client)." << std::endl;
        }
    }
    else 
    {
        std::cerr << "Warning : invalid number of arguments" << std::endl;
    }
}

void ReplController::send_process_crash(const std::vector<std::string>& command)
{
    if (command.size() == 2)
    {
        // Parsing the rank of the process
        int process_rank = std::stoi(command[1]);
        // Checking if the indicated rank is a server or a client (and not off limits)
        if ((process_rank >= 1) && (process_rank <= (this->_nb_clients + this->_nb_servers)))
        {
            // Send the start command to the client (sending empty content as we don't need one)
            this->send_and_wait(process_rank, Message::MESSAGE_TYPE::PROCESS_CRASH, "");
        }
        else 
        {
            std::cerr << "Warning : invalid rank : " << command[1] << std::endl;
        }

    }
    else 
    {
        std::cerr << "Warning : invalid number of arguments" << std::endl;
    }
}

void ReplController::send_process_recover(const std::vector<std::string>& command)
{
    if (command.size() == 2)
    {
        // Parsing the rank of the process
        int process_rank = std::stoi(command[1]);
        // Checking if the indicated rank is a server or a client (and not off limits)
        if ((process_rank >= 1) && (process_rank <= (this->_nb_clients + this->_nb_servers)))
        {
            // Send the start command to the client (sending empty content as we don't need one)
            this->send_and_wait(process_rank, Message::MESSAGE_TYPE::PROCESS_RECOVER, "");
        }
        else 
        {
            std::cerr << "Warning : invalid rank : " << command[1] << std::endl;
        }
    }
    else
    {
        std::cerr << "Warning : invalid number of arguments" << std::endl;
    }
}

void ReplController::send_new_log_entry(const std::vector<std::string>& command)
{
    if (command.size() >= 3)
    {
        // Parsing the rank of the process
        int process_rank = std::stoi(command[1]);
        // Checking if the process is a client
        if ((process_rank >= 1) && (process_rank <= this->_nb_clients))
        {
            std::string new_command = "";
            for (int i = 2; i < command.size(); i++)
            {
                new_command += command.at(i);
                new_command += (i + 1) == command.size() ? "" : " ";
            }
            // Send the start command to the client (sending empty content as we don't need one)
            this->send_and_wait(process_rank, Message::MESSAGE_TYPE::CLIENT_CREATE_NEW_ENTRY, new_command);
        }
        else 
        {
            std::cerr << "Warning : invalid rank : " << command[1] << " (this must be the rank of a client)." << std::endl;
        }
    }
    else
    {
        std::cerr << "Warning : invalid number of arguments" << std::endl;
    }
}

void ReplController::send_new_files_entries(const std::vector<std::string>& command)
{
    if (command.size() >= 3)
    {
        // Parsing the rank of the process
        int process_rank = std::stoi(command[1]);
        // Checking if the process is a client
        if ((process_rank >= 1) && (process_rank <= this->_nb_clients))
        {
            for (int i = 2; i < command.size(); i++)
            {
                // Send the start command to the client (sending empty content as we don't need one)
                this->send_and_wait(process_rank, Message::MESSAGE_TYPE::CLIENT_NEW_FILE_ENTRY, command.at(i));
            }
        }
        else 
        {
            std::cerr << "Warning : invalid rank : " << command[1] << " (this must be the rank of a client)." << std::endl;
        }
    }
    else
    {
        std::cerr << "Warning : invalid number of arguments" << std::endl;
    }
}

void ReplController::send_timeout(const std::vector<std::string>& command)
{
    if (command.size() == 2)
    {
        // Parsing the rank of the process
        int process_rank = std::stoi(command[1]);
        // Checking if the indicated rank is a server (this must be a server)
        if ((process_rank >= this->_nb_clients + 1) && (process_rank <= (this->_nb_clients + this->_nb_servers)))
        {
            // Send the start command to the client (sending empty content as we don't need one)
            this->send_and_wait(process_rank, Message::MESSAGE_TYPE::SERVER_TIMEOUT, "");
        }
        else 
        {
            std::cerr << "Warning : invalid rank : " << command[1] << " (this must be a server rank)." << std::endl;
        }
    }
    else
    {
        std::cerr << "Warning : invalid number of arguments" << std::endl;
    }
}

void ReplController::send_stop_process(const std::vector<std::string>& command)
{
    if (command.size() == 2)
    {
        // Parsing the rank of the process
        int process_rank = std::stoi(command[1]);
        // Checking if the indicated rank is a server or a client (and not off limits)
        if ((process_rank >= 1) && (process_rank <= (this->_nb_clients + this->_nb_servers)))
        {
            // Send the stop command to the client or the server (sending empty content as we don't need one)
            this->send_and_wait(process_rank, Message::MESSAGE_TYPE::PROCESS_STOP, "");
        }
        else 
        {
            std::cerr << "Warning : invalid rank : " << command[1] << std::endl;
        }
    }
    else
    {
        std::cerr << "Warning : invalid number of arguments" << std::endl;
    }
}

void ReplController::send_display_process(const std::vector<std::string>& command)
{
    if (command.size() == 2)
    {
        // Parsing the rank of the process
        int process_rank = std::stoi(command[1]);
        // Checking if the indicated rank is a server or a client (and not off limits)
        if ((process_rank >= 1) && (process_rank <= (this->_nb_clients + this->_nb_servers)))
        {
            // Send the stop command to the client (sending empty content as we don't need one)
            this->send_and_wait(process_rank, Message::MESSAGE_TYPE::PROCESS_DISPLAY, "");
        }
        else 
        {
            std::cerr << "Warning : invalid rank : " << command[1] << std::endl;
        }
    }
    else
    {
        std::cerr << "Warning : invalid number of arguments" << std::endl;
    }
}

// ========== Parsing functions ==========

void ReplController::parse_user_entry(std::string input, std::vector<std::string>& command)
{
    std::stringstream input_stream(input);
    std::string segment;

    while (input_stream >> segment) 
    {
        command.push_back(segment);
    }
}

void ReplController::parse_command(std::vector<std::string>& command)
{
    if (command[0] == "set_speed")
    {
        this->send_set_speed(command);
    }
    else if (command[0] == "start_client")
    {
        this->send_client_start(command);
    }
    else if (command[0] == "crash_process")
    {
        this->send_process_crash(command);
    }
    else if (command[0] == "recover_process")
    {
        this->send_process_recover(command);
    }
    else if (command[0] == "add_log_entry")
    {
        this->send_new_log_entry(command);
    }
    else if (command[0] == "add_files_entries")
    {
        this->send_new_files_entries(command);
    }
    else if (command[0] == "timeout_server")
    {
        this->send_timeout(command);
    }
    else if (command[0] == "stop_process")
    {
        this->send_stop_process(command);
    }
    else if (command[0] == "display_process")
    {
        this->send_display_process(command);
    }
    else
    {
        this->display_help();
        std::cerr << "Error : command not found: " << command[0] << " !" << std::endl;
    }
}

// ========== Main functions ==========

void ReplController::stop_all_processes()
{
    for (int i = 1; i <= this->_nb_clients + this->_nb_servers; i++)
    {
        // Send the stop command to the client or the server (sending empty content as we don't need one)
        this->send_and_wait(i, Message::MESSAGE_TYPE::PROCESS_STOP, "");
    }
}

void ReplController::run_repl_controller()
{
    std::cout << "Enter your command : \n> ";

    for (std::string line; std::getline(std::cin, line);)
    {
        std::vector<std::string> commands_parameters;
        this->parse_user_entry(line, commands_parameters);

        if (!commands_parameters.empty())
        {
            // Checking the prints commands here
            if (commands_parameters[0] == "help")
            {
                this->display_help();
            }
            else if (commands_parameters[0] == "process_informations")
            {
                this->display_processes_informations();
            }
            else if (commands_parameters[0] == "stop_all")
            {
                this->stop_all_processes();
                return;
            }
            else 
            {
                this->parse_command(commands_parameters);
            }
        }

        std::cout << "> ";
    }
}