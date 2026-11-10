#include "Includes/Server.hpp"
#include "Includes/Client.hpp"
#include "Includes/Irc.hpp"
#include "Includes/Globals.hpp"
#include <cstdlib>

int main(int ac, char **av)
{
    if(ac != 3)
    {
        std::cerr << "Usage: " << av[0] << " <port> <password>" << std::endl;
        return 1;
    }
    int port = std::atoi(av[1]);
    std::string password(av[2]);

    if(port <= 0 || port > 65535)
    {
        std::cerr << "Invalid port number" << std::endl;
        return 1;
    }

    try
    {
        Server server;
        server.init(port, password);
        signal(SIGINT, handle_signal);
        signal(SIGQUIT, handle_signal);
        server.run();
    }
    catch(const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
