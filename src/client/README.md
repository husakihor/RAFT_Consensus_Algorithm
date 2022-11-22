# The Client class

* The client is one of the 3 main parts of the project
* The client is the process that is sending logs to the servers. It is doing that by reading a static file (that is "linked" to the server based on his rank and can be found at ``client_commands/commands_client_{rank}.txt``). You can also ask them to read other "dynamic" logs files and to send single log via a REPL command.
* You to interact with all the clients by using REPL commands in a Command Line Interafce (CLI) by making them start, crash and sending them new logs to send to the servers.
* You can find a list of all the REPL commands in the `README.md` file at the root of the repository.