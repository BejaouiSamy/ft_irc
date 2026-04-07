#include "Server.hpp"

int main(int argc, char **argv)
{
    if(argc == 3)
    {
        int port = atoi(argv[1]);
        std::string password = std::string(argv[2]);
        Server server(port, password);
        server.run();
        return 0;
    }
    else
    {
        std::cerr << "Usage: ./IRC port password" << std::endl;
        return 1;
    }
}