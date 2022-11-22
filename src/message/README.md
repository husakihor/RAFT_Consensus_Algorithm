# Message and MessageResponse

* Implementation of the Message and MessageResponse classes.
* Messages are defined by a `MESSAGE_TYPE` and its `content`.
* Messages can have different types : 
    * `CLIENT_CREATE_NEW_ENTRY`
    * `CLIENT_START`
    * `CLIENT_WAIT_FINISH`
    * `SERVER_TIMEOUT`
    * `SERVER_PRINT_LOCAL_LOG`
    * `PROCESS_SET_SPEED`
    * `PROCESS_CRASH`
    * `PROCESS_RECOVER`
    * `PROCESS_STOP`
* Messages also have a `speed` value that can be :
    * `HIGH`
    * `MEDIUM`
    * `LOW`
* The `speed` value of the Message defines the delay of the message

Thoses classes are mostly used to communicate and send REPL commands from the ReplController class to the Clients and Servers.