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
	std::string args;

	args = getCommandArgs(message);
	if (client->isRegistered())
	{
		std::cout << "PASS error: client already registered" << std::endl;
		sendNumeric(client, "462", ":You may not reregister");
		return ;
	}
	if (args.empty())
	{
		std::cout << "PASS error: missing password" << std::endl;
		sendNumeric(client, "461", "PASS :Not enough parameters");
		return ;
	}
	if (args != _password)
	{
		std::cout << "PASS error: wrong password" << std::endl;
		sendNumeric(client, "464", ":Password incorrect");
		return ;
	}
	client->setPassOk(true);
	std::cout << "PASS accepted for fd " << client->getFd() << std::endl;
	checkRegistration(client);
}

void Server::handleNick(Client *client, const std::string& message)
{
	std::string nickname;

	nickname = getCommandArgs(message);
	if (nickname.empty())
	{
		std::cout << "NICK error: missing nickname" << std::endl;
		sendNumeric(client, "431", ":No nickname given");
		return ;
	}
	if (isNicknameTaken(nickname))
	{
		std::cout << "NICK error: nickname already in use" << std::endl;
		sendNumeric(client, "433", nickname + " :Nickname is already in use");
		return ;
	}
	client->setNickname(nickname);
	std::cout << "Nickname set for fd " << client->getFd()
		<< ": " << nickname << std::endl;
	checkRegistration(client);
}

void Server::handleUser(Client *client, const std::string& message)
{
	std::string	args;
	std::string	username;
	std::string	realname;
	size_t		space_pos;
	size_t		colon_pos;

    if (client->isRegistered())
	{
		std::cout << "USER error: client already registered" << std::endl;
		sendNumeric(client, "462", ":You may not reregister");
		return ;
	}
	args = getCommandArgs(message);
	if (args.empty())
	{
		std::cout << "USER error: missing parameters" << std::endl;
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	space_pos = args.find(' ');
	if (space_pos == std::string::npos)
	{
		std::cout << "USER error: not enough parameters" << std::endl;
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	username = args.substr(0, space_pos);
	colon_pos = args.find(':');
	if (colon_pos != std::string::npos)
		realname = args.substr(colon_pos + 1);
	else
		realname = username;
	client->setUsername(username);
	client->setRealname(realname);
	std::cout << "Username set for fd " << client->getFd()
		<< ": " << username << std::endl;
	checkRegistration(client);
}

void Server::handleJoin(Client *client, const std::string& message)
{
	std::string	channel_name;
	Channel		*channel;
	bool		created;

	channel_name = getCommandArgs(message);
	if (channel_name.empty())
	{
		sendNumeric(client, "461", "JOIN :Not enough parameters");
		return ;
	}
	if (isValidChannelName(channel_name) == false)
	{
		sendNumeric(client, "403", channel_name + " :No such channel");
		return ;
	}
	created = false;
	if (_channels.find(channel_name) == _channels.end())
	{
		_channels[channel_name] = new Channel(channel_name);
		created = true;
	}
	channel = _channels[channel_name];
	if (channel->isMember(client))
		return ;
	channel->addMember(client);
	if (created)
		channel->addOperator(client);
	sendToChannel(channel, ":" + client->getNickname() + "!"
	+ client->getUsername() + "@localhost JOIN " + channel_name, NULL);
	sendNumeric(client, "353", "= " + channel_name + " :"
		+ channel->getNamesList());
	sendNumeric(client, "366", channel_name + " :End of /NAMES list");
	std::cout << client->getNickname() << " joined "
		<< channel_name << std::endl;
}

void Server::sendReply(Client *client, const std::string& message)
{
	std::string response;

	response = message + "\r\n";
	send(client->getFd(), response.c_str(), response.length(), 0);
}

void Server::sendNumeric(Client *client, const std::string& code,
	const std::string& message)
{
	std::string target;
	std::string response;

	if (client->getNickname().empty())
		target = "*";
	else
		target = client->getNickname();
	response = ":ircserv " + code + " " + target + " " + message;
	sendReply(client, response);
}

void Server::sendUnknownCommand(Client *client, const std::string& command)
{
	sendNumeric(client, "421", command + " :Unknown command");
}

void Server::sendNotRegistered(Client *client)
{
	sendNumeric(client, "451", ":You have not registered");
}

bool Server::commandNeedsRegistration(e_CommandType type) const
{
	if (type == CMD_JOIN || type == CMD_PRIVMSG
		|| type == CMD_KICK || type == CMD_INVITE
		|| type == CMD_TOPIC || type == CMD_MODE)
		return (true);
	return (false);
}

void Server::sendToChannel(Channel *channel, const std::string& message,
	Client *except)
{
	const std::vector<Client*>&			members = channel->getMembers();
	std::vector<Client*>::const_iterator	it;

	for (it = members.begin(); it != members.end(); ++it)
	{
		if (*it != except)
			sendReply(*it, message);
	}
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

std::string Server::getCommandArgs(const std::string& message)
{
	size_t pos;

	pos = message.find(' ');
	if (pos == std::string::npos)
		return ("");
	return (message.substr(pos + 1));
}

bool Server::isNicknameTaken(const std::string& nickname) const
{
	std::map<int, Client*>::const_iterator it;

	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return (true);
	}
	return (false);
}

bool Server::isValidChannelName(const std::string& name) const
{
	if (name.empty())
		return (false);
	if (name[0] != '#')
		return (false);
	return (true);
}

void Server::checkRegistration(Client *client)
{
	if (client->isRegistered())
		return ;
	if (client->isPassOk() == true
		&& client->getNickname().empty() == false
		&& client->getUsername().empty() == false)
	{
		client->setRegistered(true);
		std::cout << "Client fd " << client->getFd()
			<< " is now registered as " << client->getNickname()
			<< std::endl;
		sendNumeric(client, "001", ":Welcome to the ft_irc server, "
			+ client->getNickname());
	}
}

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
	if (commandNeedsRegistration(type) && !client->isRegistered())
	{
		std::cout << "Command blocked: client is not registered" << std::endl;
		sendNotRegistered(client);
		return ;
	}

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
	        handleJoin(client, message);
	        break ;
		case CMD_PRIVMSG:
		case CMD_KICK:
		case CMD_INVITE:
		case CMD_TOPIC:
		case CMD_MODE:
	        std::cout << "Command recognized but not implemented yet: "
		        << command << std::endl;
			break ;
		default:
			std::cout << "Unknown command: " << command << std::endl;
            sendUnknownCommand(client, command);
			break ;
	}
}