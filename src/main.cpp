#include "Server.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <exception>
#include <sstream>

static bool	isOnlyDigits(const std::string& str)
{
	if (str.empty())
		return (false);
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (str[i] < '0' || str[i] > '9')
			return (false);
	}
	return (true);
}

static int	parsePort(const std::string& arg)
{
	std::stringstream	ss;
	int					port;

	if (isOnlyDigits(arg) == false)
		throw std::runtime_error("Invalid port: port must contain only digits");
	ss << arg;
	ss >> port;
	if (ss.fail())
		throw std::runtime_error("Invalid port");
	if (port <= 0 || port > 65535)
		throw std::runtime_error("Invalid port: port must be between 1 and 65535");
	return (port);
}

int	main(int argc, char **argv)
{
	int			port;
	std::string	password;

	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return (1);
	}
	try
	{
		port = parsePort(argv[1]);
		password = std::string(argv[2]);

		Server server(port, password);
		server.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}