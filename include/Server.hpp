#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <poll.h>

# include "Client.hpp"
# include "Channel.hpp"

class Server
{
private:
  enum e_CommandType
  {
    CMD_PASS,
    CMD_NICK,
    CMD_USER,
    CMD_JOIN,
    CMD_PRIVMSG,
    CMD_KICK,
    CMD_INVITE,
    CMD_TOPIC,
    CMD_MODE,
    CMD_UNKNOWN
  };

  int								_port;
  int								_server_fd;
  std::string						_password;
  std::vector<struct pollfd>		_fds;
  std::map<int, Client*>			_clients;
  std::map<std::string, Channel*>	_channels;

  void			acceptClient();
  void			handleData(int fd);
  void			removeClient(int fd);

  void			dispatchCommand(Client *client, const std::string& message);
  std::string		getCommandName(const std::string& message);
  std::string		getCommandArgs(const std::string& message);
  e_CommandType	getCommandType(const std::string& command);

  bool			isNicknameTaken(const std::string& nickname) const;
  void			checkRegistration(Client *client);
  void			sendReply(Client *client, const std::string& message);
  void	        sendNumeric(Client *client, const std::string& code,
        const std::string& message);
  void	sendUnknownCommand(Client *client, const std::string& command);
  void	sendNotRegistered(Client *client);
  void	sendToChannel(Channel *channel, const std::string& message,
        Client *except);

  bool	commandNeedsRegistration(e_CommandType type) const;

  bool	isValidChannelName(const std::string& name) const;
  void	handleJoin(Client *client, const std::string& message);

  void			handlePass(Client *client, const std::string& message);
  void			handleNick(Client *client, const std::string& message);
  void			handleUser(Client *client, const std::string& message);

  Server();
  Server(const Server& other);
  Server& operator=(const Server& other);

public:
  Server(int port, std::string password);
  ~Server();

  void	run();
};

#endif