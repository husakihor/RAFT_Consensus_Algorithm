#include <exception>
#include <iostream>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#include "mpi.h"

#include "server/server.hpp"
#include "client/client.hpp"
#include "repl_controller/repl_contoller.hpp"

void parse_args(std::unordered_map<std::string, int>& args, int argc, char** argv);

int main(int argc, char **argv)
{
    std::unordered_map<std::string, int> args;
    parse_args(args, argc, argv);

    // Parsing the servers number
    int serv_num = 1;
    if (args.find("servers") != args.end())
    {
        serv_num = args["servers"];

        // Checking for errors
        if (serv_num < 0)
        {
            std::cerr << "Invalid number of servers (the number of servers must be positive) : " << serv_num << std::endl;
            std::cerr << "The program must be called with : ./algorep --servers nb_servers --clients nb_clients" << std::endl;
            return -1;
        }
    }

    // Parsing the clients number
    int clients_num = 1;
    if (args.find("clients") != args.end())
    {
        clients_num = args["clients"];

        // Checking for errors
        if (clients_num < 0)
        {
            std::cerr << "Invalid number of clients (the number of clients must be positive) : " << clients_num << std::endl;
            std::cerr << "The program must be called with : ./algorep --servers nb_servers --clients nb_clients" << std::endl;
            return -1;
        }
    }
    
    // Start the MPI instances
    int rank;
    int size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check that the size is coherent with the parameters
    if (size != serv_num + clients_num + 1)
    {
        std::cerr << "Usage: mpirun -np size ./algorep --servers nb_server --clients nb_client" << std::endl
                  << "where size == n_client + n_server + 1" << std::endl;
        return 1;
    }

    // Start running the controller for the rank 0
    if (rank == 0)
    {
        ReplController repl_controller = ReplController(serv_num, clients_num);
        repl_controller.run_repl_controller();
    }
    // Start running a client if the rank is between 1 and the number of client
    else if (rank <= clients_num)
    {
        // Try to create the directory for the clients commands (should be here but if not, create it) 
        // If the directory is already here, then it won't do anything
        std::filesystem::create_directories("client_commands");
        Client client = Client(rank, serv_num, clients_num);
        client.run_client();
    }
    // For any other instances, run a server
    else
    {
        // Try to create the directory for the server logs 
        // If the directory is already here, then it won't do anything
        std::filesystem::create_directories("server_logs");
        Server server = Server(rank, serv_num, clients_num);
        server.run_server();
    }

    MPI_Finalize();
    return 0;
}

void parse_args(std::unordered_map<std::string, int>& args, int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (std::strncmp(argv[i], "--", 2) == 0 && argv[i][2])
        {
            const char* param = argv[i];

            int count = 0;
            if ((i + 1) < argc && std::strncmp(argv[i + 1], "--", 2) != 0 && argv[i + 1][0])
            {
                count = std::atoi(argv[i + 1]);
                i++;
            }

            args[&param[2]] = count;
        }
    }
}