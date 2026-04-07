#include "Server.hpp"

#include <sys/socket.h>
#include <fcntl.h>
#include <stdexcept>

Server::Server(int port, std::string password)
{
    _port = port;
    _password = password;
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd == -1)
        throw std::runtime_error("socket() failed");
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
        throw std::runtime_error("socketopt() failed");
    int flags = fcntl(_server_fd, F_GETFL, 0); // get the flags before modifying it
    if (flags == -1)
        throw std::runtime_error("fcntl() failed");
    if (fcntl(_server_fd, F_SETFL, flags | O_NONBLOCK) == -1) // its bit by bit operation, we modify only what we need. so we are just turning o_nonblock on
        throw std::runtime_error("fcntl() failed");
    // o_nonblock avoid the program to block the execution to wait. So he gonna answer with data or no.
}