#include "Includes/Irc.hpp"
#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Channel.hpp"
#include "Includes/Globals.hpp"

volatile sig_atomic_t g_stop = 0;

void handle_signal(int)
{
    g_stop = 1;
}

Server::Server() : server_fd(-1), port(0), running(false), irc(NULL) {}

Server::~Server()
{
    stop();
}

void Server::init(int port, const std::string &password)
{
    this->port = port;
    this->password = password;
    irc = new Irc(password, this);
    setup_socket(port);
}

std::map<int, Client> &Server::getClients()
{
    return clients;
}

const std::map<int, Client> &Server::getClients() const
{
    return clients;
}

Client *Server::getClientByNick(const std::string &nick)
{
    for(std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if(it->second.getNickname() == nick)
            return &(it->second);
    }
    return NULL;
}

const Client *Server::getClientByNick(const std::string &nick) const
{
    for(std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if(it->second.getNickname() == nick)
            return &(it->second);
    }
    return NULL;
}

Channel *Server::getChannel(const std::string &name)
{
    std::map<std::string, Channel*>::iterator it = channels.find(name);
    if(it == channels.end())
        return NULL;
    return it->second;
}

const Channel *Server::getChannel(const std::string &name) const
{
    std::map<std::string, Channel*>::const_iterator it = channels.find(name);
    if(it == channels.end())
        return NULL;
    return it->second;
}

std::map<std::string, Channel*> &Server::getChannels()
{
    return channels;
}

const std::map<std::string, Channel*> &Server::getChannels() const
{
    return channels;
}

Channel *Server::createChannel(const std::string &name)
{
    Channel *ch = new Channel(name);
    channels[name] = ch;
    return ch;
}

void Server::setup_socket(int port)
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0)
        throw std::runtime_error("Failed to create socket");

    try
    {
        int opt = 1;
        if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            throw std::runtime_error("Failed to set socket options");
  
        if(fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0)  
            throw std::runtime_error("Failed to set socket to non-blocking");
        
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;  
        addr.sin_port = htons(port);

        if(bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
            throw std::runtime_error("Failed to bind socket");

        if(listen(server_fd, MAX_CLIENTS) < 0)
            throw std::runtime_error("Failed to listen on socket");

        add_to_poll(server_fd);
    }
    catch(...)
    {
        close(server_fd);
        server_fd = -1;
        throw;
    }
}

void Server::add_to_poll(int fd)
{
    pollfd p;
    p.fd = fd;
    p.events = POLLIN;
    p.revents = 0;

    poll_fds.push_back(p);
}

void Server::enableWrite(int fd)
{
    for(size_t i = 0; i < poll_fds.size(); ++i)
    {
        if(poll_fds[i].fd == fd)
        {
            poll_fds[i].events |= POLLOUT;
            return;
        }
    }
}

void Server::run()
{
    std::cout << "IRC Server started , listening on port " << port << std::endl;
    running = true;
    while(running && !g_stop)
    {
        if(poll_fds.empty())
        {
            usleep(1000);
            continue;
        }   
        int ready = poll(&poll_fds[0], poll_fds.size(), -1);
        if(ready < 0)
        {
            if(errno == EINTR)
                continue;
            break;
        }  

        for(size_t i = 0; i < poll_fds.size();)
        {
            int fd = poll_fds[i].fd;
            if(poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                std::cout << "Client disconnected (error): " << fd << std::endl;
                remove_client(fd);
                continue;
            }
            if(poll_fds[i].revents & POLLIN)
            {
                if(fd == server_fd)
                    accept_client();
                else
                    handle_client(fd);
            }
            if(poll_fds[i].revents & POLLOUT)
                handleWrite(fd);
            poll_fds[i].revents = 0;
            ++i;
        }
    }
}

void Server::accept_client()
{
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
    if(client_fd < 0)
        return;

    if(clients.size() >= MAX_CLIENTS)
    {
        std::cout << "Server is full" << std::endl;
        close(client_fd);
        return;
    }

    if(fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        close(client_fd);
        return;
    }

    clients.insert(std::make_pair(client_fd, Client(client_fd)));

    add_to_poll(client_fd);

    std::cout << "New client #" << client_fd 
              << " from " << inet_ntoa(client_addr.sin_addr) 
              << ":" << ntohs(client_addr.sin_port) << std::endl;
}

void Server::handleWrite(int fd)
{
    std::map<int, Client>::iterator it = clients.find(fd);
    if(it == clients.end())
        return;

    std::string &buf = it->second.getSendBuffer();
    if(buf.empty())
    {
        for(size_t i = 0; i < poll_fds.size(); ++i)
        {
            if(poll_fds[i].fd == fd)
            {
                poll_fds[i].events &= ~POLLOUT;
                break;
            }
        }
        return;
    }

    int sent = send(fd, buf.c_str(), buf.size(), 0);
    if(sent < 0)
    {
        remove_client(fd);
        return;
    }

    buf.erase(0, sent);

    if(buf.empty())
    {
        for(size_t i = 0; i < poll_fds.size(); ++i)
        {
            if(poll_fds[i].fd == fd)
            {
                poll_fds[i].events &= ~POLLOUT;
                break;
            }
        }
    }
}

void Server::handle_client(int fd)
{
    char buffer[BUFFER_SIZE];

    int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if(bytes <= 0)
    {
        remove_client(fd);
        return;
    }

    buffer[bytes] = '\0'; 
    std::map<int, Client>::iterator it = clients.find(fd);
    if(it == clients.end())
        return;
    it->second.appendToBuffer(buffer);
    while(true)
    {
        std::string message = extractFullMessage(it->second);
        if(message.empty())
            break;
        irc->handle_client_message(it->second, message);
        if(it == clients.end() || it->second.getHasQuitReason())
        {
            remove_client(fd);
            return;
        }
    }
}

void Server::remove_client(int fd)
{
    std::map<int, Client>::iterator it = clients.find(fd);
    if(it != clients.end())
    {
        it->second.leaveAllChannels(irc);

        clients.erase(it);
    }
    
    remove_from_poll(fd);
    
    close(fd);

    std::cout << "Client " << fd << " disconnected" << std::endl;
}

void Server::remove_from_poll(int fd)
{
    for(size_t i = 0; i < poll_fds.size(); ++i)
    {
        if(poll_fds[i].fd == fd)
        {
            poll_fds.erase(poll_fds.begin() + i);
            return;
        }
    }
}

void Server::removeChannel(const std::string &name)
{
    std::map<std::string, Channel*>::iterator it = channels.find(name);
    if(it != channels.end())
    {
        delete it->second;
        channels.erase(it);
    }
}

std::string Server::extractFullMessage(Client &client)
{
    std::string &buffer = client.getBuffer();
    size_t pos = buffer.find("\n");

    if(pos == std::string::npos)
        return "";

    std::string message = buffer.substr(0, pos);
    if(!message.empty() && message[message.size() - 1] == '\r')
        message.erase(message.size() - 1);

    buffer.erase(0, pos + 1);
    if(!buffer.empty() && buffer[0] ==  '\r')
        buffer.erase(0, 1);
    return message;
}

void Server::stop()
{
    if(!running && poll_fds.empty())
        return;

    running = false;

    std::cout << "\nStopping server..." << std::endl;

    for(std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it)
        delete it->second;
    channels.clear();
    
    for(std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
        close(it->first);
    clients.clear();

    poll_fds.clear();

    if(server_fd != -1)
    {
        close(server_fd);
        server_fd = -1;
    }

    if(irc)
    {
        delete irc;
        irc = NULL;
    }

    std::cout << "Server stopped" << std::endl;
}
