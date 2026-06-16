#include "Server.hpp"

#include <iostream>
#include <vector>

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

void Server::handlePrivmsg(Client *client, const std::string& message)
{
	std::string	args;
	std::string	target;
	std::string	text;
	size_t		space_pos;
	Channel		*channel;
	Client		*receiver;

	args = getCommandArgs(message);
	if (args.empty())
	{
		sendNumeric(client, "411", ":No recipient given (PRIVMSG)");
		return ;
	}
	space_pos = args.find(' ');
	if (space_pos == std::string::npos)
	{
		sendNumeric(client, "412", ":No text to send");
		return ;
	}
	target = args.substr(0, space_pos);
	text = args.substr(space_pos + 1);
	if (text.empty())
	{
		sendNumeric(client, "412", ":No text to send");
		return ;
	}
	if (text[0] == ':')
		text.erase(0, 1);
	if (target[0] == '#')
	{
		if (_channels.find(target) == _channels.end())
		{
			sendNumeric(client, "403", target + " :No such channel");
			return ;
		}
		channel = _channels[target];
		if (channel->isMember(client) == false)
		{
			sendNumeric(client, "442", target + " :You're not on that channel");
			return ;
		}
		sendToChannel(channel, ":" + client->getNickname() + "!"
			+ client->getUsername() + "@localhost PRIVMSG "
			+ target + " :" + text, client);
		return ;
	}
	receiver = findClientByNickname(target);
	if (receiver == NULL)
	{
		sendNumeric(client, "401", target + " :No such nick/channel");
		return ;
	}
	sendReply(receiver, ":" + client->getNickname() + "!"
		+ client->getUsername() + "@localhost PRIVMSG "
		+ target + " :" + text);
}

bool Server::isValidChannelName(const std::string& name) const
{
	if (name.empty())
		return (false);
	if (name[0] != '#')
		return (false);
	return (true);
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

void Server::handleTopic(Client *client, const std::string& message)
{
	std::string	args;
	std::string	channel_name;
	std::string	topic;
	size_t		space_pos;
	size_t		colon_pos;
	Channel		*channel;

	args = getCommandArgs(message);
	if (args.empty())
	{
		sendNumeric(client, "461", "TOPIC :Not enough parameters");
		return ;
	}
	space_pos = args.find(' ');
	if (space_pos == std::string::npos)
		channel_name = args;
	else
		channel_name = args.substr(0, space_pos);
	if (_channels.find(channel_name) == _channels.end())
	{
		sendNumeric(client, "403", channel_name + " :No such channel");
		return ;
	}
	channel = _channels[channel_name];
	if (channel->isMember(client) == false)
	{
		sendNumeric(client, "442", channel_name + " :You're not on that channel");
		return ;
	}
	if (space_pos == std::string::npos)
	{
		if (channel->hasTopic())
			sendNumeric(client, "332", channel_name + " :" + channel->getTopic());
		else
			sendNumeric(client, "331", channel_name + " :No topic is set");
		return ;
	}
	colon_pos = args.find(':', space_pos);
	if (colon_pos == std::string::npos)
		topic = args.substr(space_pos + 1);
	else
		topic = args.substr(colon_pos + 1);
	channel->setTopic(topic);
	sendToChannel(channel, ":" + client->getNickname() + "!"
		+ client->getUsername() + "@localhost TOPIC "
		+ channel_name + " :" + topic, NULL);
}
