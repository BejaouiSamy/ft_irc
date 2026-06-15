#include "Server.hpp"

void Server::sendReply(Client *client, const std::string& message)
{
	std::string response;

	response = message + "\r\n";
	client->appendOutput(response);
	enablePollOut(client->getFd());
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
