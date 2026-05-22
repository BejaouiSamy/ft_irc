#include "Server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdexcept>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>

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
    sockaddr_in addr;

    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);
    if (bind(_server_fd, (struct sockaddr *)&addr, sizeof(addr)) ==-1)
        throw std::runtime_error("bind() failed");
    if (listen(_server_fd, SOMAXCONN) == -1)
        throw std::runtime_error("listen() failed");
    struct pollfd server_poll;
    server_poll.fd = _server_fd;
    server_poll.events = POLLIN;
    server_poll.revents = 0;
    _fds.push_back(server_poll);
}

void Server::run()
{
    std::cout << "IRC server listening on port " << _port << std::endl;
    while (true)
    {
        int ret = poll(&_fds[0], _fds.size(), -1);
        if (ret == -1)
            throw std::runtime_error("poll() failed");
        for (size_t i = 0; i < _fds.size(); ++i)
        {
            if (_fds[i].revents & POLLIN)
            {
                if (_fds[i].fd == _server_fd)
                    acceptClient();
                else
                    handleData(_fds[i].fd);
            }
        }
    }
}

void Server::acceptClient()
{
	sockaddr_in client_addr;
	socklen_t client_len;
	int	client_fd;

	client_len = sizeof(client_addr);
	client_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd == -1)
		return;
	int	flags = fcntl(client_fd, F_GETFL, 0);
	if (flags == -1)
	{
		close(client_fd);
		return ;
	}
	if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		close(client_fd);
		return ;
	}
	struct pollfd client_poll;
	client_poll.fd = client_fd;
	client_poll.events = POLLIN;
	client_poll.revents = 0;
	_fds.push_back(client_poll);
	std::cout << "Client connected with fd " << client_fd << std::endl;
}

void Server::handleData(int fd)
{
	char	buffer[1024];
	int	bytes;

	bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes <= 0)
	{
		std::cout << "Client disconnected fd " << fd << std::endl;
		removeClient(fd);
		return ;
	}
	buffer[bytes] = '\0';
	std::cout << "Data received from fd " << fd << ": " << buffer << std::endl;
}

void Server::removeClient(int fd)
{
    for (size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].fd == fd)
		{
			close(_fds[i].fd);
			_fds.erase(_fds.begin() + i);
			break ;
		}
	}
}