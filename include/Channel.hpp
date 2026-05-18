#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <vector>
#include "Client.hpp"

class Channel
{
    private:
        std::string             _name; // #general
        std::string             _topic;
        std::string             _password; // mode +k
        std::vector<Client*>    _member;
        std::vector<Client*>    _operator;
        bool                    _inviteOnly; // mode +i
        bool                    _topicRestricted; // mode+t
        int                     _userLimit; // mode +l, 0 = no limite

    public:
        void addMember(Client *c);
        void removeMember(Client *c);
        bool isOperator(Client *c) const;
        void broadcast(const std::string& msg, Client* except = NULL);
};

#endif