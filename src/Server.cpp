#include "Server.hpp"

#include <iostream>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdexcept>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
//#include <cctype>

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

Server::~Server()
{
	for (std::map<int, Client*>::iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		delete it->second;
	}
	_clients.clear();

	for (std::map<std::string, Channel*>::iterator it = _channels.begin();
		it != _channels.end(); ++it)
	{
		delete it->second;
	}
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
	_clients[client_fd] = new Client(client_fd);
	std::cout << "Client connected with fd " << client_fd << std::endl;
}

void Server::handleData(int fd)
{
	char	buffer[1024];
	int	bytes;
	Client		*client;
	std::string	message;

	bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes <= 0)
	{
		std::cout << "Client disconnected fd " << fd << std::endl;
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
		std::cout << "Full IRC command from fd " << fd << ": "
			<< message << std::endl;
		dispatchCommand(client, message);
	}
}

void Server::handlePass(Client *client, const std::string& message)
{
	(void)client;
	std::cout << "handlePass called with message: " << message << std::endl;
}

void Server::handleNick(Client *client, const std::string& message)
{
	(void)client;
	std::cout << "handleNick called with message: " << message << std::endl;
}

void Server::handleUser(Client *client, const std::string& message)
{
	(void)client;
	std::cout << "handleUser called with message: " << message << std::endl;
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
	if (_clients.find(fd) != _clients.end())
	{
		delete _clients[fd];
		_clients.erase(fd);
	}
}

std::string Server::getCommandName(const std::string& message)
{
	size_t pos;

	pos = message.find(' ');
	if (pos == std::string::npos)
		return (message);
	return (message.substr(0, pos));
}
/*
std::string Server::toUpperCommand(const std::string& command)
{
	std::string upper;

	upper = command;
	for (size_t i = 0; i < upper.length(); ++i)
		upper[i] = std::toupper(static_cast<unsigned char>(upper[i]));
	return (upper);
}
*/
Server::e_CommandType Server::getCommandType(const std::string& command)
{
	if (command == "PASS")
		return (CMD_PASS);
	if (command == "NICK")
		return (CMD_NICK);
	if (command == "USER")
		return (CMD_USER);
	if (command == "JOIN")
		return (CMD_JOIN);
	if (command == "PRIVMSG")
		return (CMD_PRIVMSG);
	if (command == "KICK")
		return (CMD_KICK);
	if (command == "INVITE")
		return (CMD_INVITE);
	if (command == "TOPIC")
		return (CMD_TOPIC);
	if (command == "MODE")
		return (CMD_MODE);
	return (CMD_UNKNOWN);
}

void Server::dispatchCommand(Client *client, const std::string& message)
{
	std::string		command;
	e_CommandType	type;

	command = getCommandName(message);
	type = getCommandType(command);

	std::cout << "Command detected from fd " << client->getFd()
		<< ": " << command << std::endl;

	switch (type)
	{
		case CMD_PASS:
			handlePass(client, message);
			break ;
		case CMD_NICK:
			handleNick(client, message);
			break ;
		case CMD_USER:
			handleUser(client, message);
			break ;
		case CMD_JOIN:
			std::cout << "JOIN command will be handled here" << std::endl;
			break ;
		case CMD_PRIVMSG:
			std::cout << "PRIVMSG command will be handled here" << std::endl;
			break ;
		case CMD_KICK:
			std::cout << "KICK command will be handled here" << std::endl;
			break ;
		case CMD_INVITE:
			std::cout << "INVITE command will be handled here" << std::endl;
			break ;
		case CMD_TOPIC:
			std::cout << "TOPIC command will be handled here" << std::endl;
			break ;
		case CMD_MODE:
			std::cout << "MODE command will be handled here" << std::endl;
			break ;
		default:
			std::cout << "Unknown command: " << command << std::endl;
			break ;
	}
}