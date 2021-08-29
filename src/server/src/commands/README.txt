Commands directory
    ./command.cpp
        class ExecutableCommand contains a std::function and a name that you can assign to it
        upon being ->execute()'d, the function will be ran.
        You can use the getCommand() command to register a command
        Example:
        getCommand("myCommand", [&] {

            return cmd_status::OK;
        })->setExecutor(myExecutor);
        Note that you can use regular std::functions or functions, but I used a lambda in this
        example. cmd_status::ERROR; logs an error to the server's console.
    ./executor.cpp
        class Executor can hold multiple ExecutableCommand classes in a vector and iterate through them.
        a ExecutableCommand has a setExecutor(Executor*) function that pushes itself to the given executor.