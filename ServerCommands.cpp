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
	if (command == "CAP")
		return (CMD_CAP);
	if (command == "PING")
		return (CMD_PING);
	if (command == "USERHOST")
		return (CMD_USERHOST);
	return (CMD_UNKNOWN);
}

std::string	Server::trimSpaces(const std::string& str) const
{
	size_t	start;
	size_t	end;

	start = 0;
	while (start < str.length() && str[start] == ' ')
		start++;
	if (start == str.length())
		return ("");
	end = str.length() - 1;
	while (end > start && str[end] == ' ')
		end--;
	return (str.substr(start, end - start + 1));
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
	if (commandNeedsRegistration(type) && !client->isRegistered())
	{
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
		case CMD_TOPIC:
			handleTopic(client, message);
			break ;
		case CMD_KICK:
			handleKick(client, message);
			break ;
		case CMD_INVITE:
			handleInvite(client, message);
			break ;
		case CMD_MODE:
			handleMode(client, message);
			break ;
		case CMD_CAP:
			handleCap(client, message);
			break ;
		case CMD_PING:
			handlePing(client, message);
			break ;
		case CMD_USERHOST:
			handleUserhost(client, message);
			break ;
		default:
			sendUnknownCommand(client, command);
			break ;
	}
}
void	Server::handleCap(Client *client, const std::string& message)
{
	std::string	args;
	std::string	target;

	args = getCommandArgs(message);
	if (client->getNickname().empty())
		target = "*";
	else
		target = client->getNickname();
	if (args.find("LS") == 0)
		sendReply(client, ":ircserv CAP " + target + " LS :");
}

void	Server::handlePing(Client *client, const std::string& message)
{
	std::string	token;

	token = getCommandArgs(message);
	if (token.empty())
	{
		sendNumeric(client, "409", ":No origin specified");
		return ;
	}
	if (token[0] == ':')
		token = token.substr(1);
	sendReply(client, ":ircserv PONG ircserv :" + token);
}


void	Server::handleUserhost(Client *client, const std::string& message)
{
	std::string	nick;
	Client		*target;

	nick = getCommandArgs(message);
	if (nick.empty())
		nick = client->getNickname();
	if (nick.empty() == false && nick[0] == ':')
		nick = nick.substr(1);
	target = findClientByNickname(nick);
	if (target == NULL)
	{
		sendNumeric(client, "302", ":");
		return ;
	}
	sendNumeric(client, "302", ":" + target->getNickname()
		+ "=+" + target->getUsername() + "@localhost");
}