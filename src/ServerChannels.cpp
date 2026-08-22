#include "Server.hpp"

#include <iostream>
#include <vector>
#include <sstream>

void Server::handleJoin(Client *client, const std::string& message)
{
	std::string	args;
	std::string	channel_name;
	std::string	key;
	size_t		space_pos;
	Channel		*channel;
	bool		created;

	args = getCommandArgs(message);
	if (args.empty())
	{
		sendNumeric(client, "461", "JOIN :Not enough parameters");
		return ;
	}
	space_pos = args.find(' ');
	if (space_pos == std::string::npos)
	{
		channel_name = args;
		key = "";
	}
	else
	{
		channel_name = args.substr(0, space_pos);
		key = args.substr(space_pos + 1);
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
	if (channel->isInviteOnly() && channel->isInvited(client) == false)
	{
		sendNumeric(client, "473", channel_name
			+ " :Cannot join channel (+i)");
		return ;
	}
	if (channel->hasPassword() && key != channel->getPassword())
	{
		sendNumeric(client, "475", channel_name
			+ " :Cannot join channel (+k)");
		return ;
	}
	if (channel->hasUserLimit()
		&& channel->getMemberCount() >= static_cast<size_t>(channel->getUserLimit()))
	{
		sendNumeric(client, "471", channel_name
			+ " :Cannot join channel (+l)");
		return ;
	}
	channel->addMember(client);
	channel->removeInvited(client);
	if (created)
		channel->addOperator(client);
	sendToChannel(channel, ":" + client->getNickname() + "!"
		+ client->getUsername() + "@localhost JOIN " + channel_name, NULL);
	sendNumeric(client, "353", "= " + channel_name + " :"
		+ channel->getNamesList());
	sendNumeric(client, "366", channel_name + " :End of /NAMES list");
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
	if (channel->isTopicRestricted() && channel->isOperator(client) == false)
	{
		sendNumeric(client, "482", channel_name
			+ " :You're not channel operator");
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

bool Server::parseKickArgs(const std::string& args,
	std::string& channelName, std::string& targetNick,
	std::string& reason, const std::string& defaultReason)
{
	size_t	firstSpace;
	size_t	secondSpace;
	size_t	colonPos;

	if (args.empty())
		return (false);
	firstSpace = args.find(' ');
	if (firstSpace == std::string::npos)
		return (false);
	channelName = args.substr(0, firstSpace);
	secondSpace = args.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
	{
		targetNick = args.substr(firstSpace + 1);
		reason = defaultReason;
	}
	else
	{
		targetNick = args.substr(firstSpace + 1,
				secondSpace - firstSpace - 1);
		colonPos = args.find(':', secondSpace);
		if (colonPos == std::string::npos)
			reason = args.substr(secondSpace + 1);
		else
			reason = args.substr(colonPos + 1);
	}
	if (targetNick.empty())
		return (false);
	return (true);
}


void Server::handleKick(Client *client, const std::string& message)
{
	std::string	args;
	std::string	channelName;
	std::string	targetNick;
	std::string	reason;
	Channel		*channel;
	Client		*target;

	args = getCommandArgs(message);
	if (parseKickArgs(args, channelName, targetNick, reason,
			client->getNickname()) == false)
	{
		sendNumeric(client, "461", "KICK :Not enough parameters");
		return ;
	}
	if (_channels.find(channelName) == _channels.end())
	{
		sendNumeric(client, "403", channelName + " :No such channel");
		return ;
	}
	channel = _channels[channelName];
	if (channel->isMember(client) == false)
	{
		sendNumeric(client, "442", channelName
			+ " :You're not on that channel");
		return ;
	}
	if (channel->isOperator(client) == false)
	{
		sendNumeric(client, "482", channelName
			+ " :You're not channel operator");
		return ;
	}
	target = findClientByNickname(targetNick);
	if (target == NULL || channel->isMember(target) == false)
	{
		sendNumeric(client, "441", targetNick + " " + channelName
			+ " :They aren't on that channel");
		return ;
	}
	sendToChannel(channel, ":" + client->getNickname() + "!"
		+ client->getUsername() + "@localhost KICK "
		+ channelName + " " + targetNick + " :" + reason, NULL);
	channel->removeMember(target);
}

bool	Server::parseInviteArgs(const std::string& args,
	std::string& targetNick, std::string& channelName)
{
	size_t	firstSpace;
	size_t	secondSpace;

	if (args.empty())
		return (false);
	firstSpace = args.find(' ');
	if (firstSpace == std::string::npos)
		return (false);
	targetNick = args.substr(0, firstSpace);
	secondSpace = args.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
		channelName = args.substr(firstSpace + 1);
	else
		channelName = args.substr(firstSpace + 1, secondSpace - firstSpace - 1);
	if (targetNick.empty() || channelName.empty())
		return (false);
	return (true);
}

void Server::handleInvite(Client *client, const std::string& message)
{
	std::string	args;
	std::string	targetNick;
	std::string	channelName;
	Channel		*channel;
	Client		*target;

	args = getCommandArgs(message);
	if (parseInviteArgs(args, targetNick, channelName) == false)
	{
		sendNumeric(client, "461", "INVITE :Not enough parameters");
		return ;
	}
	target = findClientByNickname(targetNick);
	if (target == NULL)
	{
		sendNumeric(client, "401", targetNick + " :No such nick/channel");
		return ;
	}
	if (_channels.find(channelName) == _channels.end())
	{
		sendNumeric(client, "403", channelName + " :No such channel");
		return ;
	}
	channel = _channels[channelName];
	if (channel->isMember(client) == false)
	{
		sendNumeric(client, "442", channelName
			+ " :You're not on that channel");
		return ;
	}
	if (channel->isOperator(client) == false)
	{
		sendNumeric(client, "482", channelName
			+ " :You're not channel operator");
		return ;
	}
	if (channel->isMember(target) == true)
	{
		sendNumeric(client, "443", targetNick + " " + channelName
			+ " :is already on channel");
		return ;
	}
	channel->addInvited(target);
	sendNumeric(client, "341", targetNick + " " + channelName);
	sendReply(target, ":" + client->getNickname() + "!"
		+ client->getUsername() + "@localhost INVITE "
		+ targetNick + " " + channelName);
}

bool	Server::parseModeArgs(const std::string& args,
	std::string& channelName, std::string& mode, std::string& modeArg)
{
	size_t	firstSpace;
	size_t	secondSpace;
	size_t	thirdSpace;

	if (args.empty())
		return (false);
	firstSpace = args.find(' ');
	if (firstSpace == std::string::npos)
	{
		channelName = args;
		mode = "";
		modeArg = "";
		return (true);
	}
	channelName = args.substr(0, firstSpace);
	secondSpace = args.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
	{
		mode = args.substr(firstSpace + 1);
		modeArg = "";
	}
	else
	{
		mode = args.substr(firstSpace + 1, secondSpace - firstSpace - 1);
		thirdSpace = args.find(' ', secondSpace + 1);
		if (thirdSpace == std::string::npos)
			modeArg = args.substr(secondSpace + 1);
		else
			modeArg = args.substr(secondSpace + 1,
					thirdSpace - secondSpace - 1);
	}
	if (channelName.empty())
		return (false);
	return (true);
}

std::string	Server::getChannelModes(Channel *channel) const
{
	std::string	modes;

	modes = "+";
	if (channel->isInviteOnly())
		modes += "i";
	if (channel->isTopicRestricted())
		modes += "t";
	if (channel->hasPassword())
		modes += "k";
	if (channel->hasUserLimit())
		modes += "l";
	return (modes);
}

bool	Server::applyChannelMode(Client *client,
	Channel *channel, const std::string& channelName,
	const std::string& mode, const std::string& modeArg)
{
	char	sign;
	bool	changed;
	Client	*target;

	sign = 0;
	changed = false;
	target = NULL;
	for (size_t i = 0; i < mode.length(); ++i)
	{
		switch (mode[i])
		{
			case '+':
			case '-':
				sign = mode[i];
				break ;

			case 'i':
				if (sign == '+')
					channel->setInviteOnly(true);
				else if (sign == '-')
					channel->setInviteOnly(false);
				else
				{
					sendNumeric(client, "472",
						"i :is unknown mode char to me");
					return (false);
				}
				changed = true;
				break ;

			case 't':
				if (sign == '+')
					channel->setTopicRestricted(true);
				else if (sign == '-')
					channel->setTopicRestricted(false);
				else
				{
					sendNumeric(client, "472",
						"t :is unknown mode char to me");
					return (false);
				}
				changed = true;
				break ;

			case 'k':
				if (sign == '+')
				{
					if (modeArg.empty())
					{
						sendNumeric(client, "461",
							"MODE :Not enough parameters");
						return (false);
					}
					channel->setPassword(modeArg);
				}
				else if (sign == '-')
					channel->clearPassword();
				else
				{
					sendNumeric(client, "472",
						"k :is unknown mode char to me");
					return (false);
				}
				changed = true;
				break ;

			case 'l':
				if (sign == '+')
				{
					int	limit;

					if (parsePositiveInt(modeArg, limit) == false)
					{
						sendNumeric(client, "461",
							"MODE :Not enough parameters");
						return (false);
					}
					channel->setUserLimit(limit);
				}
				else if (sign == '-')
					channel->clearUserLimit();
				else
				{
					sendNumeric(client, "472",
						"l :is unknown mode char to me");
					return (false);
				}
				changed = true;
				break ;

			case 'o':
				if (modeArg.empty())
				{
					sendNumeric(client, "461",
						"MODE :Not enough parameters");
					return (false);
				}
				target = findClientByNickname(modeArg);
				if (target == NULL)
				{
					sendNumeric(client, "401",
						modeArg + " :No such nick/channel");
					return (false);
				}
				if (channel->isMember(target) == false)
				{
					sendNumeric(client, "441", modeArg + " "
						+ channelName + " :They aren't on that channel");
					return (false);
				}
				if (sign == '+')
					channel->addOperator(target);
				else if (sign == '-')
					channel->removeOperator(target);
				else
				{
					sendNumeric(client, "472",
						"o :is unknown mode char to me");
					return (false);
				}
				changed = true;
				break ;

			default:
				sendNumeric(client, "472", std::string(1, mode[i])
					+ " :is unknown mode char to me");
				return (false);
		}
	}
	if (changed)
	{
		if ((mode.find("+k") != std::string::npos
				|| mode.find("+l") != std::string::npos
				|| mode.find("+o") != std::string::npos
				|| mode.find("-o") != std::string::npos)
			&& modeArg.empty() == false)
		{
			sendToChannel(channel, ":" + client->getNickname() + "!"
				+ client->getUsername() + "@localhost MODE "
				+ channelName + " " + mode + " " + modeArg, NULL);
		}
		else
		{
			sendToChannel(channel, ":" + client->getNickname() + "!"
				+ client->getUsername() + "@localhost MODE "
				+ channelName + " " + mode, NULL);
		}
	}
	return (true);
}

bool	Server::parsePositiveInt(const std::string& str, int& value) const
{
	std::stringstream	ss;
	int					number;

	if (str.empty())
		return (false);
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (str[i] < '0' || str[i] > '9')
			return (false);
	}
	ss << str;
	ss >> number;
	if (ss.fail() || number <= 0)
		return (false);
	value = number;
	return (true);
}

void	Server::handleMode(Client *client, const std::string& message)
{
	std::string	args;
	std::string	channelName;
	std::string	mode;
	std::string	modeArg;
	Channel		*channel;

	args = getCommandArgs(message);
	if (parseModeArgs(args, channelName, mode, modeArg) == false)
	{
		sendNumeric(client, "461", "MODE :Not enough parameters");
		return ;
	}
	if (_channels.find(channelName) == _channels.end())
	{
		sendNumeric(client, "403", channelName + " :No such channel");
		return ;
	}
	channel = _channels[channelName];
	if (channel->isMember(client) == false)
	{
		sendNumeric(client, "442", channelName
			+ " :You're not on that channel");
		return ;
	}
	if (mode.empty())
	{
		sendNumeric(client, "324", channelName + " "
			+ getChannelModes(channel));
		return ;
	}
	if (channel->isOperator(client) == false)
	{
		sendNumeric(client, "482", channelName
			+ " :You're not channel operator");
		return ;
	}
	applyChannelMode(client, channel, channelName, mode, modeArg);
}

void	Server::removeClientFromChannels(Client *client)
{
	std::map<std::string, Channel*>::iterator	it;
	std::map<std::string, Channel*>::iterator	tmp;
	Channel									*channel;

	it = _channels.begin();
	while (it != _channels.end())
	{
		channel = it->second;
		channel->removeInvited(client);
		if (channel->isMember(client))
			channel->removeMember(client);
		else
			channel->removeOperator(client);
		if (channel->getMemberCount() == 0)
		{
			tmp = it;
			++it;
			delete tmp->second;
			_channels.erase(tmp);
		}
		else
			++it;
	}
}