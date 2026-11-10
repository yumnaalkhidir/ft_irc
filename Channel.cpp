#include "Includes/Irc.hpp"
#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Channel.hpp"

Channel::Channel(const std::string &name)
    : name(name),
      topic(""),
      inviteOnly(false),
      topicRestricted(false),
      key(""),
      userLimit(0)
{
}

Channel::~Channel()
{
}

bool Channel::isInviteOnly() const
{
    return (inviteOnly);
}

bool Channel::isTopicRestricted() const
{
    return (topicRestricted);
}

bool Channel::hasLimit() const
{
    if (userLimit == 0)
        return (false);
    return (true);
}

bool Channel::hasKey() const
{
    if (key.empty())
        return (false);
    return (true);
}

const std::string &Channel::getKey() const
{
    return (key);
}

const size_t &Channel::getUserLimit() const
{
    return (userLimit);
}

void Channel::setTopic(const std::string &topic)
{
    this->topic = topic;
}
const std::string Channel::getTopic() const
{
    return topic;
}

const std::string &Channel::getName() const
{
    return name;
}

const std::map<int, Client *> &Channel::getMembers() const
{
    return members;
}

void Channel::addClient(Client *client)
{
    if (!client)
        return;
    members[client->getFd()] = client;
}

void Channel::removeClient(Client *client)
{
    if (!client)
        return;
    members.erase(client->getFd());
    if (isOperator(client))
        removeOperator(client);
}

bool Channel::hasClient(Client *client) const
{
    if (!client)
        return false;
    return members.find(client->getFd()) != members.end();
}

void Channel::addOperator(Client *client)
{
    if (!client)
        return;
    operators.insert(client);
}

void Channel::removeOperator(Client *client)
{
    if (!client)
        return;
    operators.erase(client);
}

bool Channel::isOperator(Client *client) const
{
    if (!client)
        return (false);
    return (operators.find(client) != operators.end());
}

void Channel::inviteClient(Client *client)
{
    if (!client)
        return;
    invitedClients.insert(client);
}

void Channel::removeInvite(Client *client)
{
    if (!client)
        return;
    invitedClients.erase(client);
}

bool Channel::isInvited(Client *client) const
{
    if (!client)
        return (false);
    return (invitedClients.find(client) != invitedClients.end());
}

void Channel::setInviteOnly(bool state)
{
    this->inviteOnly = state;
}

void Channel::setTopicRestricted(bool state)
{
    this->topicRestricted = state;
}

void Channel::setChannelKey(std::string &pass_key)
{
    this->key = pass_key;
}

void Channel::removeChannelKey()
{
    this->key = "";
}

void Channel::setLimit(int limit)
{
    this->userLimit = limit;
}

void Channel::removeLimit()
{
    this->userLimit = 0;
}

void Channel::broadcast(const std::string &msg, Client *sender, Irc *irc)
{
    for (std::map<int, Client *>::iterator it = members.begin(); it != members.end(); ++it)
    {
        if (sender && it->second == sender)
            continue;
        irc->sendToClient(*(it->second), msg);
    }
}
