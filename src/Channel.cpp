#include "Channel.hpp"

Channel::Channel()
{
	_name = "";
	_topic = "";
	_password = "";
	_inviteOnly = false;
	_topicRestricted = false;
	_userLimit = 0;
}

Channel::Channel(const std::string& name)
{
	_name = name;
	_topic = "";
	_password = "";
	_inviteOnly = false;
	_topicRestricted = false;
	_userLimit = 0;
}

Channel::Channel(const Channel& other)
{
	*this = other;
}

Channel& Channel::operator=(const Channel& other)
{
	if (this != &other)
	{
		_name = other._name;
		_topic = other._topic;
		_password = other._password;
		_members = other._members;
		_operators = other._operators;
		_inviteOnly = other._inviteOnly;
		_topicRestricted = other._topicRestricted;
		_userLimit = other._userLimit;
        _invited = other._invited;
	}
	return (*this);
}

Channel::~Channel()
{
}

std::string Channel::getName() const
{
	return (_name);
}

std::string Channel::getTopic() const
{
	return (_topic);
}

size_t Channel::getMemberCount() const
{
	return (_members.size());
}

void Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

bool Channel::hasTopic() const
{
	return (_topic.empty() == false);
}

const std::vector<Client*>& Channel::getMembers() const
{
	return (_members);
}

void Channel::addMember(Client *client)
{
	if (isMember(client) == false)
		_members.push_back(client);
}

void Channel::removeMember(Client *client)
{
	std::vector<Client*>::iterator it;

	for (it = _members.begin(); it != _members.end(); ++it)
	{
		if (*it == client)
		{
			_members.erase(it);
			break ;
		}
	}
	removeOperator(client);
	removeInvited(client);
}

bool Channel::isMember(Client *client) const
{
	std::vector<Client*>::const_iterator it;

	for (it = _members.begin(); it != _members.end(); ++it)
	{
		if (*it == client)
			return (true);
	}
	return (false);
}

void Channel::addOperator(Client *client)
{
	if (isOperator(client) == false)
		_operators.push_back(client);
}

void Channel::removeOperator(Client *client)
{
	std::vector<Client*>::iterator it;

	for (it = _operators.begin(); it != _operators.end(); ++it)
	{
		if (*it == client)
		{
			_operators.erase(it);
			break ;
		}
	}
}

bool Channel::isOperator(Client *client) const
{
	std::vector<Client*>::const_iterator it;

	for (it = _operators.begin(); it != _operators.end(); ++it)
	{
		if (*it == client)
			return (true);
	}
	return (false);
}

std::string Channel::getNamesList() const
{
	std::vector<Client*>::const_iterator	it;
	std::string							names;

	for (it = _members.begin(); it != _members.end(); ++it)
	{
		if (names.empty() == false)
			names += " ";
		if (isOperator(*it))
			names += "@";
		names += (*it)->getNickname();
	}
	return (names);
}

void Channel::broadcast(const std::string& message, Client *except)
{
	(void)message;
	(void)except;
}

void Channel::addInvited(Client *client)
{
	if (isInvited(client) == false)
		_invited.push_back(client);
}

void Channel::removeInvited(Client *client)
{
	for (std::vector<Client*>::iterator it = _invited.begin();
		it != _invited.end(); ++it)
	{
		if (*it == client)
		{
			_invited.erase(it);
			return ;
		}
	}
}

bool Channel::isInvited(Client *client) const
{
	for (std::vector<Client*>::const_iterator it = _invited.begin();
		it != _invited.end(); ++it)
	{
		if (*it == client)
			return (true);
	}
	return (false);
}

void	Channel::setInviteOnly(bool value)
{
	_inviteOnly = value;
}

bool	Channel::isInviteOnly() const
{
	return (_inviteOnly);
}
