#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>

class Client
{
	private:
		int			_fd;
		std::string	_nickname;
		std::string	_username;
		std::string	_realname;
		std::string	_buffer;
		bool		_passOk;
		bool		_registered;

	public:
		Client();
		Client(int fd);
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();

		int			getFd() const;
		std::string	getNickname() const;
		std::string	getUsername() const;
		std::string	getRealname() const;
		bool		isPassOk() const;
		bool		isRegistered() const;

		void		setNickname(const std::string& nickname);
		void		setUsername(const std::string& username);
		void		setRealname(const std::string& realname);
		void		setPassOk(bool value);
		void		setRegistered(bool value);

		void		appendBuffer(const std::string& data);
		std::string	extractMessage();
};

#endif