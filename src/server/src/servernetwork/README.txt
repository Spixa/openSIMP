ServerNetwork directory
    ./servernetwork.cpp:
        contains the main functionality of the server.
        function ::init(unsigned short); initalizes the server on a port
        function ::run(); runs the server, fails if the server isn't initalized
        function ::Get()-> gets the instance of the server. note that the ServerNetwork
        class is a singleton, meaning you cannot use it constructor, nor copy it, you
        can only access it by getting its instance.