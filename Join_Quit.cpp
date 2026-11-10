#include "Includes/Irc.hpp"
#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Channel.hpp"

std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t end;
    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        if (end > start)
            result.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    if (start < str.length())
        result.push_back(str.substr(start));
    return (result);
}

void Irc::handleJoin(Client &client, const Message &msg)
{
    if (msg.params.empty())
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return;
    }
    std::string channelName = msg.params[0];
    if (channelName == "0")
    {
        client.partAllChannels(this);
        return;
    }
    std::vector<std::string> channelsToJoin = split(msg.params[0], ',');
    std::vector<std::string> keys;
    if (msg.params.size() >= 2)
        keys = split(msg.params[1], ',');
    size_t keysIndex = 0;
    for (std::vector<std::string>::iterator chanIt = channelsToJoin.begin(); chanIt != channelsToJoin.end(); chanIt++)
    {
        bool newChannel = false;
        if (chanIt->empty() ||
            ((*chanIt)[0] != '#' && (*chanIt)[0] != '&'))
        {
            sendReplyWithTarget(client, "403", *chanIt, ERR_INVALCHANNELNAME);
            continue;
        }
        Channel *channel = server->getChannel(*chanIt);
        if (!channel)
        {
            channel = server->createChannel(*chanIt);
            newChannel = true;
        }
        if (channel->hasClient(&client))
        {
            sendReplyWithTarget(client, "443", *chanIt, ERR_USERONCHANNEL);
            continue;
        }
        if (channel->isInviteOnly() && !channel->isInvited(&client))
        {
            sendReplyWithTarget(client, "473", *chanIt, ERR_INVITEONLYCHAN);
            continue;
        }
        std::string key = "";
        if (channel->hasKey())
        {
            if (keysIndex < keys.size())
                key = keys[keysIndex];
            else
                key = "";
            if (channel->getKey() != key || key.empty())
            {
                sendReplyWithTarget(client, "475", *chanIt, ERR_BADCHANNELKEY);
                continue;
            }
            keysIndex++;
        }

        if (channel->hasLimit() && channel->getMembers().size() >= channel->getUserLimit())
        {
            sendReplyWithTarget(client, "471", *chanIt, ERR_CHANNELISFULL);
            continue;
        }
        channel->addClient(&client);
        client.addChannel(channel);
        if (newChannel)
            channel->addOperator(&client);
        if (channel->isInvited(&client))
            channel->removeInvite(&client);
        std::string joinMsg =
            ":" + client.getNickname() +
            "!" + client.getUsername() +
            "@irc.server.com JOIN " +
            *chanIt + "\r\n";
        channel->broadcast(joinMsg, NULL, this);

        if (channel->getTopic().empty())
            sendReplyWithTarget(client, "331", *chanIt, "No topic is set");
        else
            sendReplyWithTarget(client, "332", *chanIt, channel->getTopic());

        const std::map<int, Client *> &members = channel->getMembers();
        std::string names;
        for (std::map<int, Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
        {
            if (!names.empty())
                names += " ";
            if (channel->isOperator(it->second))
                names += "@";
            names += it->second->getNickname();
        }
        std::string rpl353 = ":irc.server.com 353 " + client.getNickname() + " = " + *chanIt + " :" + names + "\r\n";
        client.queueMessage(rpl353);
        sendReplyWithTarget(client, "366", *chanIt, "End of NAMES list");
    }
}

void Irc::handlePart(Client &client, const Message &msg)
{
    if (msg.params.empty())
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return;
    }
    std::vector<std::string> channelsToPart = split(msg.params[0], ',');
    std::string part_msg;
    if (msg.params.size() > 1 && msg.params[1] != "")
        part_msg = msg.params[1];
    for (std::vector<std::string>::iterator it = channelsToPart.begin(); it != channelsToPart.end(); it++)
    {
        Channel *channel = server->getChannel(*it);
        if (!channel)
        {
            sendReplyWithTarget(client, "403", *it, ERR_INVALCHANNELNAME); 
            continue;
        }
        if (!channel->hasClient(&client))
        {
            sendReplyWithTarget(client, "442", *it, ERR_NOTONCHANNEL); 
            continue;
        }
        std::string out_msg = ":" + client.getNickname() + "!" +
                              client.getUsername() + "@irc.server.com PART " + *it;
        if (!part_msg.empty())
            out_msg += " :" + part_msg;
        out_msg += "\r\n";
        channel->broadcast(out_msg, NULL, this);
        channel->removeClient(&client);
        client.removeChannel(channel);
        if (channel->getMembers().empty())
            server->removeChannel(channel->getName());
    }
}

void Irc::handleKick(Client &client, const Message &msg)
{
    if (msg.params.size() < 2)
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return;
    }
    std::string channelName = msg.params[0];
    Channel *targetChannel = server->getChannel(channelName);
    if (!targetChannel)
    {
        sendReplyWithTarget(client, "403", channelName, ERR_INVALCHANNELNAME);
        return;
    }
    if (!targetChannel->hasClient(&client))
    {
        sendReplyWithTarget(client, "442", channelName, ERR_NOTONCHANNEL);
        return;
    }
    if (!targetChannel->isOperator(&client))
    {
        sendReplyWithTarget(client, "482", channelName, ERR_CHANOPRIVSNEEDED);
        return;
    }
    std::vector<std::string> targets = split(msg.params[1], ',');
    std::string reason;
    if (msg.params.size() > 2)
        reason = msg.params[2];
    for (size_t i = 0; i < targets.size(); i++)
    {
        Client *targetClient = server->getClientByNick(targets[i]);
        if (!targetClient)
        {
            sendReplyWithTarget(client, "401", targets[i], ERR_NOSUCHNICK);
            continue;
        }
        if (!targetChannel->hasClient(targetClient))
        {
            sendReplyWithTarget(client, "441", targets[i] + " " + channelName, ERR_USERNOTINCHANNEL);
            continue;
        }
        std::string kickmsg = ":" + client.getNickname() +
                              "!" + client.getUsername() +
                              "@irc.server.com KICK " + channelName +
                              " " + targets[i];
        if (!reason.empty())
            kickmsg += " :" + reason;
        kickmsg += "\r\n";
        targetChannel->broadcast(kickmsg, NULL, this);
        
        targetChannel->removeClient(targetClient);
        targetClient->removeChannel(targetChannel);
        if (targetChannel->getMembers().empty())
            server->removeChannel(channelName);
    }
}

void Irc::handleQuit(Client &client, const Message &msg)
{
    if(msg.params.empty())
        client.setQuitReason("Client Quit");
    else
        client.setQuitReason(msg.params[0]);
    client.setHasQuitReason(true);
}
