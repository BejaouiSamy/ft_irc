#include "Server.hpp"

#include <iostream>

void Server::handlePass(Client *client, const std::string& message)
{
	std::string args;

	args = trimSpaces(getCommandArgs(message));
	if (client->isRegistered())
	{
		sendNumeric(client, "462", ":You may not reregister");
		return ;
	}
	if (args.empty())
	{
		sendNumeric(client, "461", "PASS :Not enough parameters");
		return ;
	}
	if (args != _password)
	{
		sendNumeric(client, "464", ":Password incorrect");
		return ;
	}
	client->setPassOk(true);
	checkRegistration(client);
}

void Server::handleNick(Client *client, const std::string& message)
{
	std::string nickname;

	nickname = getCommandArgs(message);
	if (nickname.empty())
	{
		sendNumeric(client, "431", ":No nickname given");
		return ;
	}
	if (isNicknameTaken(nickname))
	{
		sendNumeric(client, "433", nickname + " :Nickname is already in use");
		return ;
	}
	client->setNickname(nickname);
	checkRegistration(client);
}

void	Server::handleUser(Client *client, const std::string& message)
{
	std::string	args;
	std::string	username;
	std::string	realname;
	size_t		firstSpace;
	size_t		secondSpace;
	size_t		thirdSpace;
	size_t		colonPos;

	if (client->isRegistered())
	{
		sendNumeric(client, "462", ":You may not reregister");
		return ;
	}
	args = trimSpaces(getCommandArgs(message));
	if (args.empty())
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	firstSpace = args.find(' ');
	if (firstSpace == std::string::npos)
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	secondSpace = args.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	thirdSpace = args.find(' ', secondSpace + 1);
	if (thirdSpace == std::string::npos)
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	username = args.substr(0, firstSpace);
	colonPos = args.find(':', thirdSpace + 1);
	if (colonPos == std::string::npos)
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	realname = args.substr(colonPos + 1);
	if (username.empty() || realname.empty())
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	client->setUsername(username);
	client->setRealname(realname);
	checkRegistration(client);
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
		sendNumeric(client, "001", ":Welcome to the ft_irc server, "
			+ client->getNickname());
	}
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

Client *Server::findClientByNickname(const std::string& nickname) const
{
	std::map<int, Client*>::const_iterator it;

	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return (it->second);
	}
	return (NULL);
}
