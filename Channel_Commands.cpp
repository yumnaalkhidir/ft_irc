#include "Includes/Irc.hpp"
#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Channel.hpp"

void Irc::handleTopic(Client &client, const Message &msg)
{
    if (msg.params.empty())
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
    if (msg.params.size() == 1)
    {
        if (targetChannel->getTopic().empty())
            sendReplyWithTarget(client, "331", channelName, "No topic is set");
        else
            sendReplyWithTarget(client, "332", channelName, targetChannel->getTopic());
        return;
    }
    if (!targetChannel->isOperator(&client) && targetChannel->isTopicRestricted())
    {
        sendReplyWithTarget(client, "482", channelName, ERR_CHANOPRIVSNEEDED);
        return;
    }
    std::string topic = msg.params[1];
    targetChannel->setTopic(topic);
    std::string newTopicMsg = ":" + client.getNickname() +
                              "!" + client.getUsername() +
                              "@irc.server.com TOPIC " + channelName +
                              " :" + targetChannel->getTopic() + "\r\n";
    targetChannel->broadcast(newTopicMsg, NULL, this);
}

void Irc::handleInvite(Client &client, const Message &msg)
{
    if (msg.params.size() < 2)
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return;
    }
    std::string targetNick = msg.params[0];
    Client *targetClient = server->getClientByNick(targetNick);
    if (!targetClient)
    {
    
        sendReplyWithTarget(client, "401", targetNick, ERR_NOSUCHNICK);
        return;
    }
    std::string channelName = msg.params[1];
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
    if (channel->hasClient(targetClient))
    {   
        sendReplyWithTarget(client, "443",targetNick + " " + channelName, ERR_USERONCHANNEL);
        return;
    }
    if (channel->isInviteOnly() && !channel->isOperator(&client))
    {
    
        sendReplyWithTarget(client, "482", channelName, ERR_CHANOPRIVSNEEDED);
        return;
    }
    channel->inviteClient(targetClient);
    std::string inviteMsg = ":" + client.getNickname() + "!" +
                            client.getUsername() + "@irc.server.com INVITE " +
                            targetNick + " :" + channelName + "\r\n";
    sendToClient(*targetClient, inviteMsg);
    sendReplyWithTarget(client, "341", targetNick, channelName);
}
