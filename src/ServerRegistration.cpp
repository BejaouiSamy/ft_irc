#include "Server.hpp"

#include <iostream>

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
