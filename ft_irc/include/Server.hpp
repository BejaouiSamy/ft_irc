#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <poll.h>
#include <cstdlib>
#include "Client.hpp"
#include "Channel.hpp"

class Server
{
    private:
        int                             _port;
        int                             _server_fd; // Listening fd
        std::string                     _password;
        std::vector<struct pollfd>      _fds; // Every checked fds
        std::map<int, Client*>          _clients; // fd -> client
        std::map<std::string, Channel*> _channels; // #general -> channel

    public:
        Server(int port, std::string password);
        void run(); //principal loop
        void acceptClient(); // new client -> accet()
        void handleData(int fd); // recv() + parser + dispatcher
        void removeClient(int fd); // clean logout
};

#endif