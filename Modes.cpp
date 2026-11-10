#include "Includes/Irc.hpp"
#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Channel.hpp"

bool isNumber(const std::string &param)
{
    if (param.empty())
        return (false);
    for (size_t i = 0; i < param.length(); i++)
    {
        if (!std::isdigit(param[i]))
            return (false);
    }
    return (true);
}

std::string intToString(int n)
{
    std::stringstream ss;
    ss << n;
    return (ss.str());
}

bool Irc::modeK(bool state, size_t &paramIndex, const Message &msg, Client &client, Channel &channel)
{
    if (state)
    {
        if (paramIndex >= msg.params.size())
        {
            sendReply(client, "461", ERR_NEEDMOREPARAMS);
            return (false);
        }
        std::string chanKey = msg.params[paramIndex++];
        channel.setChannelKey(chanKey);
    }
    else
        channel.removeChannelKey();
    return (true);
}

bool Irc::modeO(bool state, size_t &paramIndex, const Message &msg, Client &client, Channel &channel)
{
    if (paramIndex >= msg.params.size())
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return (false);
    }
    std::string nick = msg.params[paramIndex++];
    Client *targetClient = server->getClientByNick(nick);
    if (!targetClient)
    {
    
        sendReplyWithTarget(client, "401", nick, ERR_NOSUCHNICK);
        return (false);
    }
    if (!channel.hasClient(targetClient))
    {
    
        sendReplyWithTarget(client, "441", nick, ERR_USERNOTINCHANNEL); 
        return (false);
    }
    if (state)
        channel.addOperator(targetClient);
    else
        channel.removeOperator(targetClient);
    return (true);
}

bool Irc::modeL(bool state, size_t &paramIndex, const Message &msg, Client &client, Channel &channel)
{
    if (state)
    {
        if (paramIndex >= msg.params.size())
        {
            sendReply(client, "461", ERR_NEEDMOREPARAMS);
            return (false);
        }
        std::string limit = msg.params[paramIndex++];
        if (!isNumber(limit))
        {
            sendReply(client, "461", ERR_NEEDMOREPARAMS);
            return (false);
        }
        std::stringstream ss(limit);
        size_t value;
        ss >> value;
        if (ss.fail())
        {
            sendReply(client, "461", ERR_NEEDMOREPARAMS);
            return (false);
        }
        channel.setLimit(value);
    }
    else
        channel.removeLimit();
    return (true);
}

void Irc::viewModes(Client &client, Channel &channel)
{
    std::string modes = "+";
    std::string modeParams;
    if (channel.isInviteOnly())
        modes += "i";
    if (channel.isTopicRestricted())
        modes += "t";
    if (channel.hasKey())
        modes += "k";
    if (channel.hasLimit())
    {
        modes += "l";
        modeParams += " " + intToString(channel.getUserLimit());
    }
    sendReplyWithTarget(client, "324", channel.getName(), modes + modeParams);
}

void Irc::handleMode(Client &client, const Message &msg)
{
    if (msg.params.empty())
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return;
    }

    std::string channelName = msg.params[0];
    Channel *channel = server->getChannel(channelName);
    if (!channel)
    {    
        sendReplyWithTarget(client, "403", channelName, ERR_INVALCHANNELNAME);
        return;
    }
    if (!channel->hasClient(&client))
    {
        sendReplyWithTarget(client, "442", channelName, ERR_NOTONCHANNEL);
        return;
    }
    if (msg.params.size() == 1)
    {
        viewModes(client, *channel);
        return;
    }
    
    if (msg.params.size() == 2)
    {

        if (msg.params[1] == "b")
        {
            sendReplyWithTarget(client, "368", channelName, "End of channel ban list");
            return;
        }
    }

    if (!channel->isOperator(&client))
    {
        sendReplyWithTarget(client, "482", channelName, ERR_CHANOPRIVSNEEDED);
        return;
    }
    std::string modes = msg.params[1];
    size_t paramIndex = 2;
    std::string usedParams;
    bool state = false;
    size_t i = 0;
    if (modes[0] != '+' && modes[0] != '-')
    {
        sendReplyWithTarget(client, "472", std::string(1, modes[i]), 
                            "is unknown mode char to me");
        return;
    }
    while (i < modes.length())
    {
        if (modes[i] == '+')
            state = true;
        else if (modes[i] == '-')
            state = false;
        else if (modes[i] == 'i')
            channel->setInviteOnly(state);
        else if (modes[i] == 't')
            channel->setTopicRestricted(state);
        else if (modes[i] == 'k')
        {
            if (!modeK(state, paramIndex, msg, client, *channel))
                return;
            if (state)
                usedParams += " " + channel->getKey();
        }
        else if (modes[i] == 'o')
        {
            if (!modeO(state, paramIndex, msg, client, *channel))
                return;
            usedParams += " " + msg.params[paramIndex - 1];
        }
        else if (modes[i] == 'l')
        {
            if (!modeL(state, paramIndex, msg, client, *channel))
                return;
            if (state)
                usedParams += " " + intToString(channel->getUserLimit());
        }
        else
        {
            sendReplyWithTarget(client, "472", std::string(1, modes[i]), 
                    "is unknown mode char to me");
            return;
        }
        i++;
    }
    std::string modeMsg = ":" + client.getNickname() + "!" + client.getUsername() +
                          "@irc.server.com MODE " + channelName + " " +
                          modes + usedParams + "\r\n";
    channel->broadcast(modeMsg, NULL, this);
}
