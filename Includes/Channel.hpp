#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Irc.hpp"

class Channel
{
private:
    std::string name;
    std::string topic;

    std::map<int, Client *> members;
    std::set<Client *> operators;
    std::set<Client *> invitedClients;

    bool inviteOnly;
    bool topicRestricted;
    std::string key;
    size_t userLimit;

public:
    Channel(const std::string &name);
    ~Channel();

    void addClient(Client *client);
    void removeClient(Client *client);
    bool hasClient(Client *client) const;

    void addOperator(Client *client);
    void removeOperator(Client *client);
    bool isOperator(Client *client) const;

    void inviteClient(Client *client);
    void removeInvite(Client *client);
    bool isInvited(Client *client) const;

    void setTopic(const std::string &topic);

    const size_t &getUserLimit() const;
    const std::string &getKey() const;
    const std::string getTopic() const;
    const std::string &getName() const;
    const std::map<int, Client *> &getMembers() const;

    bool isInviteOnly() const;
    void setInviteOnly(bool state);

    bool isTopicRestricted() const;
    void setTopicRestricted(bool state);

    bool hasKey() const;
    void setChannelKey(std::string &pass_key);
    void removeChannelKey();

    bool hasLimit() const;
    void setLimit(int limit);
    void removeLimit();

    void broadcast(const std::string &msg, Client *sender, Irc *irc);
};

#endif