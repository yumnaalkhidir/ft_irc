#ifndef SERVER_HPP
#define SERVER_HPP

#include "Irc.hpp"

class Client;
class Channel;

class Server
{
    private:
        int server_fd;
        int port;
        std::string password;
        bool running;
        Irc *irc;

        std::map<int, Client> clients;
        std::map<std::string, Channel*> channels;
        std::vector<pollfd> poll_fds;

    public:
        Server();
        ~Server();

        void init(int port, const std::string &password);

        std::map<int, Client> &getClients();
        const std::map<int, Client> &getClients() const;

        Channel *getChannel(const std::string &name);
        const Channel *getChannel(const std::string &name) const;

        std::map<std::string, Channel*> &getChannels();
        const std::map<std::string, Channel*> &getChannels() const;

        Client *getClientByNick(const std::string &nick);
        const Client *getClientByNick(const std::string &nick) const;

        Channel *createChannel(const std::string &name);
         void removeChannel(const std::string &name);

        void enableWrite(int fd);
        void handleWrite(int fd);

        void remove_client(int fd);

        void run();
        void stop();


    private:
        void setup_socket(int port);
        void add_to_poll(int fd);

        void accept_client();
        void handle_client(int fd);

        std::string extractFullMessage(Client &client);
        void remove_from_poll(int fd);
};

#endif