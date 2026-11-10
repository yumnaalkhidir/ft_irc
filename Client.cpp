#include "Includes/Irc.hpp"
#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Channel.hpp"

Client::Client() 
        : fd(-1), 
          buffer(""), 
          nickname(""), 
          username(""),
          state(CONNECTED), 
          passAccepted(false),
          userSet(false),
          sendBuffer(""),
          quitReason(""),
          hasQuitReason(false)
{}

Client::Client(int fd)
        : fd(fd),
          buffer(""),
          nickname(""), 
          username(""),
          state(CONNECTED),
          passAccepted(false),
          userSet(false),
          sendBuffer(""),
          quitReason(""),
          hasQuitReason(false)
{}

Client::~Client() {}

int Client::getFd() const 
{
    return fd;
}

const std::string &Client::getBuffer() const 
{
    return buffer;
}

std::string &Client::getBuffer() 
{
    return buffer;
}

const std::string &Client::getNickname() const 
{
    return nickname;
}

ClientState Client::getState() const 
{
    return state;
}

void Client::setNickname(const std::string &nickname) 
{
    this->nickname = nickname;
}

void Client::setState(ClientState state) 
{
    this->state = state;
}

bool Client::isRegistered() const 
{
    return state == REGISTERED;
}

bool Client::isPassAccepted() const 
{
    return passAccepted;
}

bool Client::isUserSet() const 
{
    return userSet;
}

void Client::setPassAccepted(bool value) 
{
    passAccepted = value;
}

void Client::setUserSet(bool value) 
{
    userSet = value;
}

void Client::setUsername(const std::string &username) 
{
    this->username = username;
}

std::string Client::getUsername() const 
{
    return username;
}

void Client::appendToBuffer(const std::string &data) 
{
    buffer += data;
}

void Client::queueMessage(const std::string &msg)
{
    sendBuffer += msg;
}

std::string &Client::getSendBuffer()
{
    return sendBuffer;
}

void Client::clearBuffer() 
{
    buffer.clear();
}

void Client::leaveAllChannels(Irc *irc) 
{
    std::set<Channel*> copy = channels;
    std::set<int> notified;
    std::string reason;
    if(getHasQuitReason())
        reason = getQuitReason();
    else
        reason = "Client Disconnected";
    std::string quitMsg = 
            ":" + nickname + "!" + username + 
            "@irc.server.com QUIT :" + reason + "\r\n";

    for(std::set<Channel*>::iterator it = copy.begin(); it != copy.end(); ++it)
    {
        Channel *channel = *it;

        const std::map<int, Client*> &members = channel->getMembers();
        for(std::map<int, Client*>::const_iterator mit = members.begin();
            mit != members.end(); ++mit)
        {
            Client *other = mit->second;
            if(other == this)
                continue;
            if(notified.insert(other->getFd()).second)
                irc->sendToClient(*other, quitMsg);
           
        }
        channel->removeClient(this);
        removeChannel(channel);
        if(channel->getMembers().empty())
            irc->getServer()->removeChannel(channel->getName());
    }
    channels.clear();
}

void Client::partAllChannels(Irc *irc)
{
    std::set<Channel*> copy = channels;

    for(std::set<Channel*>::iterator it = copy.begin(); it != copy.end(); ++it)
    {
        Channel *channel = *it;
        std::string partMsg = 
            ":" + nickname + "!" + username + 
            "@irc.server.com PART " + channel->getName() + "\r\n";

        const std::map<int, Client*> &members = channel->getMembers();
        for(std::map<int, Client*>::const_iterator mit = members.begin();
            mit != members.end(); ++mit)
        {
            Client *other = mit->second;
            if(other == this)
                continue;
            irc->sendToClient(*other, partMsg);
        }
        channel->removeClient(this);
        removeChannel(channel);
        if(channel->getMembers().empty())
            irc->getServer()->removeChannel(channel->getName());
    }
    channels.clear();
}

void Client::addChannel(Channel *channel)
{
    if(channel)
        channels.insert(channel);
}

void Client::removeChannel(Channel *channel)
{
    if(channel)
        channels.erase(channel);
}

const std::set<Channel*> &Client::getChannels() const
{
    return channels;
}

void Client::setQuitReason(const std::string &reason)
{
    quitReason = reason;
}

void Client::setHasQuitReason(bool value)
{
    hasQuitReason = value;
}

const std::string &Client::getQuitReason() const
{
    return quitReason;
}

bool Client::getHasQuitReason() const
{
    return hasQuitReason;
}