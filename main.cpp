#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <poll.h>
#include <cstring>

void exit_on_failure(std::string message)
{
    std::cerr << message << std::endl;
    std::cerr << "errno: " << errno << std::endl;
    exit(1);
}

int main(int ac, char**av)
{
    if (ac == 3)
    {
        std::vector<pollfd> pollfds;
        int port = std::atoi(av[1]);
        std::string password = av[2];
        //Socket creation

        int sockfd;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            exit_on_failure("Failed to create socket");
        else
        {
            printf("Socket Successfully created\n");
            printf("Sockfd: %d\n" , sockfd);
        }
        pollfd serverpoll;
        serverpoll.fd = sockfd;
        serverpoll.events = POLLIN;
        pollfds.push_back(serverpoll);

        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            exit_on_failure("setsocket failed");
        //Binding
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        if (bind(sockfd, (struct sockaddr*)&address, sizeof(address)) != 0)
            exit_on_failure("Failed to bind");
        else
            printf("Bind is successfull\n");

        //Listening
        if (listen(sockfd, SOMAXCONN) != 0)
            exit_on_failure("Failed to listen");
        else
            printf("Listen successfull\n");
        
        //Accepting requests
        //////////////////////////////////////
        
        sockaddr_in client_address;
        socklen_t clientsock_len = sizeof(client_address);
        while(1)
        {
            if (poll(pollfds.data(), pollfds.size(), -1) == -1)
                exit_on_failure("Poll{} failed");
            for(size_t i= 0; i < pollfds.size(); i++)
            {
                if (!(pollfds[i].revents & POLLIN))
                    continue ;
                
                if (pollfds[i].fd == sockfd)
                {
                    int clientfd = accept(sockfd, (struct sockaddr *)&client_address, &clientsock_len);
                    if (clientfd < 0)
                        exit_on_failure("Failed to accept");
                    
                    pollfd new_client;
                    new_client.fd = clientfd;
                    new_client.events = POLLIN;
                    pollfds.push_back(new_client);
                }
                else
                {
                    char buff[1024];
                    int read_bytes = recv(pollfds[i].fd, buff, sizeof(buff) - 1, 0);
                    if(read_bytes <= 0)
                    {
                        std::cout << "Client disconnected" << std::endl;
                        pollfds.erase(pollfds.begin() - i);
                        i--;
                    }
                    else
                    {
                        buff[read_bytes] = '\0';
                        if (!std::strcmp(buff, "exit\0"))
                            exit(0);
                        std::cout << "Message received form client no." << pollfds[i].fd << std::endl;
                        send(pollfds[i].fd, "Hello There\n", 12, 0);
                    }
                }
            }
        }
        exit(0);
    }
    else
    {
        std::cout << "Usage: ircserv <port> <password>" << std::endl;
    }
}
