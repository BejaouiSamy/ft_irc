#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

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

    public:
        // getter / setter
        int getFd() const;
        std::string getNickname() const;
        bool isRegistered() const;

        // Buffer handling
        void appendBuffer(const std::string& data);
        std::string extractMessage(); // extract until /r/n

};

#endif

/* _buffer is very important because if recv() gave incomplete message
    buffer get data until we hit /r/n complet
*/