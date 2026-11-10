NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98


RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
RES = \033[0m

SRCS = main.cpp \
	   Server.cpp \
	   Client.cpp \
	   Irc.cpp \
	   Channel.cpp \
	   Join_Quit.cpp \
	   Modes.cpp \
	   Authenticate.cpp \
	   Channel_Commands.cpp \
	   Irssi_checks.cpp \
	   Message.cpp


OBJS = $(SRCS:.cpp=.o)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(NAME) has been created!$(RES)"

clean:
	@rm -f $(OBJS)
	@echo "$(YELLOW)Object files have been removed!$(RES)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)$(NAME) has been removed!$(RES)"

re: fclean all

.PHONY: all clean fclean re