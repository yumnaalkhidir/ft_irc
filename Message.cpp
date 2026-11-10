#include "Includes/Irc.hpp"
#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Channel.hpp"

void Irc::handlePrivmsg(Client &client, const Message &msg)
{
    if (msg.params.size() < 2)
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return;
    }
    std::vector<std::string> targets = split(msg.params[0], ',');
    std::string text = msg.params[1];
    for(std::vector<std::string>::iterator targetsIt = targets.begin(); targetsIt!= targets.end(); targetsIt++)
    {
        std::string full_msg = ":" + client.getNickname() + "!" +
                               client.getUsername() + "@irc.server.com PRIVMSG " + *targetsIt + " :" + text + "\r\n";
        if ((*targetsIt)[0] != '#' && (*targetsIt)[0] != '&')
        {
            Client *targetClient = server->getClientByNick(*targetsIt);
            if (!targetClient)
            {
                sendReplyWithTarget(client, "401", *targetsIt, ERR_NOSUCHNICK);
                continue;
            }
            sendToClient(*targetClient, full_msg);
        }
        else
        {
            Channel *targetChannel = server->getChannel(*targetsIt);
            if (!targetChannel)
            {
                sendReplyWithTarget(client, "403", *targetsIt, ERR_INVALCHANNELNAME);
                continue;
            }
            if (!targetChannel->hasClient(&client))
            {
            
                sendReplyWithTarget(client, "442", *targetsIt, ERR_NOTONCHANNEL);
                continue;
            }
            targetChannel->broadcast(full_msg, &client, this);
        }
    }
}

void Irc::handleNotice(Client &client, const Message &msg)
{
    if (msg.params.size() < 2)
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return;
    }
    std::vector<std::string> targets = split(msg.params[0], ',');
    std::string text = msg.params[1];
    for(std::vector<std::string>::iterator targetsIt = targets.begin(); targetsIt!= targets.end(); targetsIt++)
    {
        std::string full_msg = ":" + client.getNickname() + "!" +
                               client.getUsername() + "@irc.server.com PRIVMSG " + *targetsIt + " :" + text + "\r\n";
        if ((*targetsIt)[0] != '#' && (*targetsIt)[0] != '&')
        {
            Client *targetClient = server->getClientByNick(*targetsIt);
            if (!targetClient)
                continue;
            sendToClient(*targetClient, full_msg);
        }
        else
        {
            Channel *targetChannel = server->getChannel(*targetsIt);
            if (!targetChannel)
                continue;
            if (!targetChannel->hasClient(&client))
                continue;
            targetChannel->broadcast(full_msg, &client, this);
        }
    }
}
