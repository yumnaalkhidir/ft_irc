#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Irc.hpp"

enum ClientState
{
    DISCONNECTED,
    CONNECTED,
    REGISTERED
};

class Channel;

class Client
{
    private:
        int fd;
        std::string buffer;
        std::string nickname;
        std::string username;

        ClientState state;
        bool passAccepted;
        bool userSet;

        std::set<Channel*> channels;

        std::string sendBuffer;
        std::string quitReason;
        bool hasQuitReason;

    public:
        Client();
        Client(int fd);
        ~Client();

        int getFd() const;
        const std::string &getBuffer() const;
        std::string &getBuffer();
        const std::string &getNickname() const;
        ClientState getState() const;

        void setNickname(const std::string &nickname);
        void setState(ClientState state);

        void setPassAccepted(bool value);
        bool isPassAccepted() const;

        void setUserSet(bool value);
        bool isUserSet() const;
        
        void setUsername(const std::string &username);
        std::string getUsername() const;

        bool isRegistered() const;
      
        void appendToBuffer(const std::string &data);
        void clearBuffer();

        void addChannel(Channel *channel);
        void removeChannel(Channel *channel);
        const std::set<Channel*> &getChannels() const;
        void leaveAllChannels(Irc *irc);
        void partAllChannels(Irc *irc);

        void queueMessage(const std::string &msg);
        std::string &getSendBuffer();

        void setQuitReason(const std::string &reason);
        const std::string &getQuitReason() const;
        void setHasQuitReason(bool value);
        bool getHasQuitReason() const;
};

#endif