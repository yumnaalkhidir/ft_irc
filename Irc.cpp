#include "Includes/Irc.hpp"
#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Channel.hpp"

Irc::Irc(const std::string &password, Server *server)
    : serverPassword(password), server(server)
{
    std::cout << "IRC Logic Ready with password" << std::endl;
    commands["NICK"] = &Irc::handleNick;
    commands["USER"] = &Irc::handleUser;
    commands["PASS"] = &Irc::handlePass;
    commands["JOIN"] = &Irc::handleJoin;
    commands["PRIVMSG"] = &Irc::handlePrivmsg;
    commands["NOTICE"] = &Irc::handleNotice;
    commands["PART"] = &Irc::handlePart;
    commands["KICK"] = &Irc::handleKick;
    commands["TOPIC"] = &Irc::handleTopic;
    commands["INVITE"] = &Irc::handleInvite;
    commands["MODE"] = &Irc::handleMode;
    commands["QUIT"] = &Irc::handleQuit;
    commands["PING"] = &Irc::handlePing;
    commands["CAP"] = &Irc::handleCap;
    commands["WHOIS"] = &Irc::handleWhois;
}

Server *Irc::getServer()
{
    return server;
}

void trim(std::string &s)
{
    while (!s.empty() && s[0] == ' ')
        s.erase(0, 1);
}

std::string parse_prefix(std::string &message)
{
    size_t pos = message.find(' ');
    if (pos == std::string::npos)
    {
        std::string prefix = message.substr(1);
        message.clear();
        return (prefix);
    }
    std::string prefix = message.substr(1, pos - 1);
    message.erase(0, pos + 1);
    return (prefix);
}

std::string parse_cmd(std::string &message)
{
    trim(message);
    size_t pos = message.find(' ');
    std::string cmd;
    if (pos == std::string::npos)
    {
        cmd = message;
        message.clear();
    }
    else
    {
        cmd = message.substr(0, pos);
        message.erase(0, pos + 1);
    }
    for (size_t i = 0; i < cmd.length(); ++i)
        cmd[i] = std::toupper(cmd[i]);
    return cmd;
}

std::vector<std::string> parse_params(std::string &message)
{
    trim(message);
    size_t pos;
    std::vector<std::string> params;
    while (!message.empty())
    {
        pos = message.find(' ');
        if (message[0] == ':')
        {
            params.push_back(message.substr(1));
            break;
        }
        if (pos == std::string::npos)
        {
            if (!message.empty())
                params.push_back(message);
            break;
        }
        params.push_back(message.substr(0, pos));
        message.erase(0, pos + 1);
        while (!message.empty() && message[0] == ' ')
            message.erase(0, 1);
    }
    return (params);
}

Message Irc::parse_msg(const std::string &raw)
{
    Message tokens;
    std::string message = raw;
    trim(message);
    if (!message.empty() && message[0] == ':')
        tokens.prefix = parse_prefix(message);
    else
        tokens.prefix = "";
    if (!message.empty())
        tokens.cmd = parse_cmd(message);
    else
        tokens.cmd = "";
    tokens.params = parse_params(message);
    return (tokens);
}

std::string Irc::buildReply(const std::string &server,
                            const std::string &code,
                            const std::string &target,
                            const std::string &msg)
{
    return ":" + server + " " + code + " " + target + " :" + msg + "\r\n";
}

void Irc::sendReply(Client &client, const std::string &code, const std::string &msg)
{
    std::string reply = buildReply("irc.server.com", code,
                                   client.getNickname().empty() ? "*" : client.getNickname(), msg);
    client.queueMessage(reply);
    server->enableWrite(client.getFd());
}

std::string Irc::buildReplyWithTarget(const std::string &server,
                                       const std::string &code,
                                       const std::string &clientNick,
                                       const std::string &target,
                                       const std::string &msg)
{
    return ":" + server + " " + code + " " + clientNick + " " + target + " :" + msg + "\r\n";
}

void Irc::sendReplyWithTarget(Client &client,
                               const std::string &code,
                               const std::string &target,
                               const std::string &msg)
{
    std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
    std::string reply = buildReplyWithTarget("irc.server.com", code, nick, target, msg);
    client.queueMessage(reply);
    server->enableWrite(client.getFd());
}

void Irc::sendToClient(Client &client, const std::string &msg)
{
    client.queueMessage(msg);
    server->enableWrite(client.getFd());
}

void Irc::handle_client_message(Client &client, const std::string &message)
{
    Message msg = parse_msg(message);
    if (msg.cmd.empty())
        return;
    std::map<std::string, CommandHandler>::iterator it = commands.find(msg.cmd);
    if (it == commands.end())
    {
        sendReply(client, "421", msg.cmd + " " + ERR_UNKNOWNCOMMAND);
        return;
    }
    if (!client.isRegistered() &&
        msg.cmd != "PASS" &&
        msg.cmd != "NICK" &&
        msg.cmd != "USER")
    {
        sendReply(client, "451", ERR_NOTREGISTERED);
        return;
    }
    CommandHandler handler = it->second;
    (this->*handler)(client, msg);
}
