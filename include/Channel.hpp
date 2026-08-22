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
  std::vector<Client*>		_members;
  std::vector<Client*>		_operators;
  std::vector<Client*>		_invited;
  bool					_inviteOnly;
  bool					_topicRestricted;
  int						_userLimit;

public:
  Channel();
  Channel(const std::string& name);
  Channel(const Channel& other);
  Channel& operator=(const Channel& other);
  ~Channel();

  std::string					getName() const;
  std::string					getTopic() const;
  size_t						getMemberCount() const;
  const std::vector<Client*>&	getMembers() const;

  void						setTopic(const std::string& topic);
  bool						hasTopic() const;
  void	          setInviteOnly(bool value);
  bool	          isInviteOnly() const;
  void	          setTopicRestricted(bool value);
  bool	          isTopicRestricted() const;

  void		        setPassword(const std::string& password);
  std::string	    getPassword() const;
  bool		        hasPassword() const;
  void		        clearPassword();

  void	          setUserLimit(int limit);
  int		          getUserLimit() const;
  bool	          hasUserLimit() const;
  void	          clearUserLimit();

  void						addMember(Client *client);
  void						removeMember(Client *client);
  bool						isMember(Client *client) const;

  void						addOperator(Client *client);
  void						removeOperator(Client *client);
  bool						isOperator(Client *client) const;

  void						addInvited(Client *client);
  void						removeInvited(Client *client);
  bool						isInvited(Client *client) const;

  std::string					getNamesList() const;
  void						broadcast(const std::string& message,
                                                                  Client *except = NULL);
};

#endif