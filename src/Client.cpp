#include "Client.hpp"

Client::Client()
{
  _fd = -1;
  _nickname = "";
  _username = "";
  _realname = "";
  _buffer = "";
  _passOk = false;
  _registered = false;
}

Client::Client(int fd)
{
  _fd = fd;
  _nickname = "";
  _username = "";
  _realname = "";
  _buffer = "";
  _passOk = false;
  _registered = false;
}

Client::Client(const Client& other)
{
  *this = other;
}

Client& Client::operator=(const Client& other)
{
  if (this != &other)
  {
    _fd = other._fd;
    _nickname = other._nickname;
    _username = other._username;
    _realname = other._realname;
    _buffer = other._buffer;
    _passOk = other._passOk;
    _registered = other._registered;
  }
  return (*this);
}

Client::~Client()
{
}

int Client::getFd() const
{
	return (_fd);
}

std::string Client::getNickname() const
{
	return (_nickname);
}

bool Client::isRegistered() const
{
	return (_registered);
}

void Client::appendBuffer(const std::string& data)
{
	_buffer += data;
}

std::string Client::extractMessage()
{
	size_t		pos;
	std::string	message;

	pos = _buffer.find("\r\n");
	if (pos == std::string::npos)
		return ("");
	message = _buffer.substr(0, pos);
	_buffer.erase(0, pos + 2);
	return (message);
}
