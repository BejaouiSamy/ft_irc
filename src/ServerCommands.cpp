#include "Server.hpp"

#include <iostream>

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

bool Server::commandNeedsRegistration(e_CommandType type) const
{
	if (type == CMD_JOIN || type == CMD_PRIVMSG
		|| type == CMD_KICK || type == CMD_INVITE
		|| type == CMD_TOPIC || type == CMD_MODE)
		return (true);
	return (false);
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
			handlePrivmsg(client, message);
			break ;
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
