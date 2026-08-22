#include "Server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

void Server::acceptClient()
{
	sockaddr_in	client_addr;
	socklen_t	client_len;
	int			client_fd;

	client_len = sizeof(client_addr);
	client_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd == -1)
		return ;

	int flags = fcntl(client_fd, F_GETFL, 0);
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
	_clients[client_fd] = new Client(client_fd);
}

void Server::handleData(int fd)
{
	char		buffer[1024];
	int			bytes;
	Client		*client;
	std::string	message;

	bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes <= 0)
	{
		removeClient(fd);
		return ;
	}
	buffer[bytes] = '\0';
	client = _clients[fd];
	client->appendBuffer(std::string(buffer));
	while (true)
	{
		message = client->extractMessage();
		if (message.empty())
			break ;
		dispatchCommand(client, message);
	}
}

void Server::handleWrite(int fd)
{
	Client	*client;
	ssize_t	bytes;

	if (_clients.find(fd) == _clients.end())
		return ;
	client = _clients[fd];
	if (client->getOutputBuffer().empty())
	{
		disablePollOut(fd);
		return ;
	}
	bytes = send(fd, client->getOutputBuffer().c_str(),
			client->getOutputBuffer().length(), 0);
	if (bytes > 0)
	{
		client->eraseOutput(bytes);
		if (client->hasOutput() == false)
			disablePollOut(fd);
		return ;
	}
	if (bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK
			|| errno == EINTR))
		return ;
	removeClient(fd);
}

void	Server::removeClient(int fd)
{
	std::map<int, Client*>::iterator	clientIt;
	std::vector<struct pollfd>::iterator	fdIt;
	Client								*client;

	clientIt = _clients.find(fd);
	if (clientIt != _clients.end())
	{
		client = clientIt->second;
		removeClientFromChannels(client);
		delete client;
		_clients.erase(clientIt);
	}
	for (fdIt = _fds.begin(); fdIt != _fds.end(); ++fdIt)
	{
		if (fdIt->fd == fd)
		{
			_fds.erase(fdIt);
			break ;
		}
	}
	close(fd);
	std::cout << "Client disconnected with fd " << fd << std::endl;
}

void Server::enablePollOut(int fd)
{
	for (size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].fd == fd)
		{
			_fds[i].events |= POLLOUT;
			return ;
		}
	}
}

void Server::disablePollOut(int fd)
{
	for (size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].fd == fd)
		{
			_fds[i].events &= ~POLLOUT;
			return ;
		}
	}
}
