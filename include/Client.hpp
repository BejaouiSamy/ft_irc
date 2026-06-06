#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
    private:
        int             _fd;
        std::string     _nickname;
        std::string     _username;
        std::string     _realname;
        std::string     _buffer; // incomplete data received
        bool            _passOk; // does he sent PASS ?
        bool            _registered; // does he did PASS+NICK+USER ?

		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setRealname(const std::string& realname);
		void setPassOk(bool value);
		void setRegistered(bool value);

		bool isPassOk() const;
		std::string getUsername() const;
		std::string getRealname() const;

    public:
		Client();
		Client(int fd);
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();

        int getFd() const;
        std::string getNickname() const;
        bool isRegistered() const;

        // Buffer handling
        void appendBuffer(const std::string& data);
        std::string extractMessage(); // extract until \r\n

};

#endif