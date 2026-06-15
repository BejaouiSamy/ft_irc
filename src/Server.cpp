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
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR,
			&opt, sizeof(int)) < 0)
		throw std::runtime_error("socketopt() failed");

	int flags = fcntl(_server_fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("fcntl() failed");
	if (fcntl(_server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl() failed");

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);
	if (bind(_server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		throw std::runtime_error("bind() failed");
	if (listen(_server_fd, SOMAXCONN) == -1)
		throw std::runtime_error("listen() failed");

	struct pollfd server_poll;
	server_poll.fd = _server_fd;
	server_poll.events = POLLIN;
	server_poll.revents = 0;
	_fds.push_back(server_poll);
}

Server::~Server()
{
	for (std::map<int, Client*>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();

	for (std::map<std::string, Channel*>::iterator it = _channels.begin();
		it != _channels.end(); ++it)
		delete it->second;
	_channels.clear();

	for (size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].fd >= 0)
			close(_fds[i].fd);
	}
	_fds.clear();
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
			int fd = _fds[i].fd;
			short revents = _fds[i].revents;

			if (revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				if (fd != _server_fd)
					removeClient(fd);
				continue ;
			}
			if (revents & POLLIN)
			{
				if (fd == _server_fd)
					acceptClient();
				else
					handleData(fd);
			}
			if (fd != _server_fd && _clients.find(fd) != _clients.end()
				&& (revents & POLLOUT))
				handleWrite(fd);
		}
	}
}
