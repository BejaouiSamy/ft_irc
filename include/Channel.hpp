#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <vector>
# include "Client.hpp"

class Channel
{
private:
  std::string				_name;
  std::string				_topic;
  std::string				_password;
  std::vector<Client*>	_members;
  std::vector<Client*>	_operators;
  bool					_inviteOnly;
  bool					_topicRestricted;
  int						_userLimit;

public:
  Channel();
  Channel(const std::string& name);
  Channel(const Channel& other);
  Channel& operator=(const Channel& other);
  ~Channel();

  std::string	getName() const;
  std::string	getTopic() const;
  size_t		getMemberCount() const;
  const std::vector<Client*>&	getMembers() const;

  void		addMember(Client *client);
  void		removeMember(Client *client);
  bool		isMember(Client *client) const;

  void		addOperator(Client *client);
  void		removeOperator(Client *client);
  bool		isOperator(Client *client) const;

  std::string	getNamesList() const;
  void		broadcast(const std::string& message, Client *except = NULL);
};

#endif