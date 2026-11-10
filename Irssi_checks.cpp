#include "Includes/Irc.hpp"
#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Channel.hpp"

void Irc::handlePing(Client &client, const Message &msg)
{
    if (msg.params.size() < 1)
    {
        sendReply(client, "409", "No origin specified");
        return;
    }
    std::string pongMsg = ":irc.server.com PONG irc.server.com :" + msg.params[0] + "\r\n";
    sendToClient(client, pongMsg);
}

void Irc::handleCap(Client &client, const Message &msg)
{
    if (msg.params.size() > 0 && msg.params[0] == "LS")
    {
        std::string response = ":irc.server.com CAP * LS :\r\n";
        sendToClient(client, response);
    }
    else if (msg.params.size() > 0 && msg.params[0] == "END")
        return ;
}

void Irc::handleWhois(Client &client, const Message &msg)
{
    if (msg.params.empty())
    {
        sendReply(client, "431", ERR_NONICKNAMEGIVEN);
        return;
    }
    std::string targetNick = msg.params[0];
    Client *target = server->getClientByNick(targetNick);
    if (!target)
    {
        sendReplyWithTarget(client, "401", targetNick, ERR_NOSUCHNICK);
        return;
    }
    sendReplyWithTarget(client, "311",
        target->getNickname() + " " + target->getUsername() + " irc.server.com *",
        target->getNickname());

    sendReplyWithTarget(client, "312",
        target->getNickname() + " irc.server.com",
        "IRC Server");

    const std::set<Channel *> &channels = target->getChannels();
    if (!channels.empty())
    {
        std::string chanList;
        for (std::set<Channel *>::const_iterator it = channels.begin();
             it != channels.end(); ++it)
        {
            if ((*it)->isOperator(target))
                chanList += "@";
            chanList += (*it)->getName() + " ";
        }
        sendReplyWithTarget(client, "319", target->getNickname(), chanList);
    }

    sendReplyWithTarget(client, "318", target->getNickname(), "End of /WHOIS list");
}
