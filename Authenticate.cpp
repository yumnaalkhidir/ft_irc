#include "Includes/Irc.hpp"
#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Channel.hpp"

void Irc::tryRegister(Client &client)
{
    if (client.isRegistered())
        return;
    if (client.isPassAccepted() &&
        client.isUserSet() &&
        !client.getUsername().empty() &&
        !client.getNickname().empty())
    {
        client.setState(REGISTERED);
        sendReply(client, "001", RPL_WELCOME + client.getNickname());
    }
}

void Irc::handlePass(Client &client, const Message &msg)
{
    if (client.isRegistered() || client.isPassAccepted())
    {
        sendReply(client, "462", ERR_ALREADYREGISTRED);
        return;
    }
    if (msg.params.empty())
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return;
    }
    std::string givenPassword = msg.params[0];
    if (givenPassword != serverPassword)
    {
        sendReply(client, "464", ERR_PASSWDMISMATCH);
        client.setState(DISCONNECTED);
        return;
    }
    client.setPassAccepted(true);
    tryRegister(client);
}

bool Irc::isValidNickname(const std::string &nickname)
{
    if (nickname.empty() || nickname.length() > 9)
        return false;
    const std::string validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";
    if (nickname.find_first_not_of(validChars) != std::string::npos)
        return false;
    return true;
}

bool Irc::isNickTaken(const std::string &nickname)
{
    const std::map<int, Client> &clients = server->getClients();
    for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.getNickname() == nickname)
            return true;
    }
    return false;
}

void Irc::handleNick(Client &client, const Message &msg)
{

    if (msg.params.empty())
    {
        sendReply(client, "431", ERR_NONICKNAMEGIVEN);
        return;
    }
    std::string newNick = msg.params[0];

    if (!isValidNickname(newNick))
    {
        sendReplyWithTarget(client, "432", newNick, ERR_ERRONEUSNICKNAME);
        return;
    }
    if (client.getNickname() == newNick)
        return;

    if (isNickTaken(newNick))
    {
        
        sendReplyWithTarget(client, "433", newNick, ERR_NICKNAMEINUSE);
        return;
    }

    if (!client.getNickname().empty())
    {
        std::string oldNick = client.getNickname();
        client.setNickname(newNick);
        std::string nickMsg = ":" + oldNick + "!" + client.getUsername() +
                              "@irc.server.com NICK :" + newNick + "\r\n";
        const std::set<Channel *> &channels = client.getChannels();
        for (std::set<Channel *>::const_iterator it = channels.begin();
             it != channels.end(); ++it)
        {
            (*it)->broadcast(nickMsg, &client, this);
        }
        sendToClient(client, nickMsg);
        return;
    }

    client.setNickname(newNick);
    tryRegister(client);
}

void Irc::handleUser(Client &client, const Message &msg)
{
    if (client.isRegistered() || client.isUserSet())
    {
        sendReply(client, "462", ERR_ALREADYREGISTRED);
        return;
    }
    if (!client.isPassAccepted())
    {
        sendReply(client, "464", ERR_PASSWDMISMATCH);
        client.setState(DISCONNECTED);
        return;
    }
    if (msg.params.size() < 4)
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return;
    }
    std::string username = msg.params[0];
    std::string realname = msg.params[3];

    if (username.empty())
    {
        sendReply(client, "461", ERR_NEEDMOREPARAMS);
        return;
    }

    client.setUsername(username);
    client.setUserSet(true);

    tryRegister(client);
}
