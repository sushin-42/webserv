CC = c++ 
CFLAGS = -Wall -Werror -Wextra -std=c++98
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
NAME = webserv 

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	rm -rf $(OBJS)

fclean :
	rm -rf $(NAME) $(OBJS)

re : fclean all
