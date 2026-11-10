#ifndef IRC_HPP
#define IRC_HPP

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <cerrno>
#include <fcntl.h>
#include <utility>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cctype>
#include <set>
#include <sstream>
#include <arpa/inet.h>

#include "Replies.hpp"

class Server;
class Client;
class Channel;

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 1024

struct Message
{
    std::string prefix;
    std::string cmd;
    std::vector<std::string> params;
};

class Irc
{
    private:
        std::string serverPassword;
        Server *server;

        typedef void (Irc::*CommandHandler)(Client &client, const Message &msg);
        std::map<std::string, CommandHandler> commands;
    
    public:
        Irc(const std::string &password, Server *server);
        Message parse_msg(const std::string& message);
        void handle_client_message(Client &client, const std::string &message);
        std::string buildReply(const std::string &server, 
                               const std::string &code, 
                               const std::string &target, 
                               const std::string &msg);
        void sendReply(Client &client, const std::string &code, const std::string &msg);
        std::string buildReplyWithTarget(const std::string &server,
                                  const std::string &code,
                                  const std::string &clientNick,
                                  const std::string &target,
                                  const std::string &msg);

        void sendReplyWithTarget(Client &client,
                          const std::string &code,
                          const std::string &target,
                          const std::string &msg);
        void tryRegister(Client &client);
        void sendToClient(Client &client, const std::string &msg);
        Server* getServer();

    private:
        void handleNick(Client &client, const Message &msg);
        void handleUser(Client &client, const Message &msg);
        void handlePass(Client &client, const Message &msg);
        void handleJoin(Client &client, const Message &msg);
        void handlePrivmsg(Client &client, const Message &msg);
        void handleNotice(Client &client, const Message &msg);
        void handlePart(Client &client, const Message &msg);
        void handleKick(Client &client, const Message &msg);
        void handleTopic(Client &client, const Message &msg);
        void handleInvite(Client &client, const Message &msg);
        bool modeK(bool state, size_t& paramIndex, const Message& msg, Client& client, Channel& channel);
        bool modeO(bool state, size_t& paramIndex, const Message& msg, Client& client, Channel& channel);
        bool modeL(bool state, size_t& paramIndex, const Message& msg, Client& client, Channel& channel);
        void viewModes(Client& client, Channel& channel);
        void handleMode(Client &client, const Message &msg);
        void handleQuit(Client &client, const Message &msg);
        void handlePing(Client &client, const Message &msg);
        void handleCap(Client &client, const Message &msg);
        void handleWhois(Client &client, const Message &msg);

        bool isValidNickname(const std::string &nickname);
        bool isNickTaken(const std::string &nickname);
};

std::vector<std::string> split(const std::string &str, char delimiter);
bool isNumber(const std::string &param);
std::string intToString(int n);
void trim(std::string &s);
std::string parse_prefix(std::string &message);
std::string parse_cmd(std::string &message);
std::vector<std::string> parse_params(std::string &message);

#endif