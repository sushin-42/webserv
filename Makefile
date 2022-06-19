CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98 -g -fsanitize=address
SRCS = main.cpp Config.cpp HttpConfig.cpp ServerConfig.cpp LocationConfig.cpp
INC_DIR = incs
SRC_DIR = srcs
OBJS =  ${SRCS:.cpp=.o}
SRCS =  $(SRC_DIR)/CGI.cpp \
		$(SRC_DIR)/Config.cpp \
		$(SRC_DIR)/ConfigUtils.cpp \
		$(SRC_DIR)/ConfigChecker.cpp \
		$(SRC_DIR)/ConfigLoader.cpp \
		$(SRC_DIR)/ConnSocket.cpp \
		$(SRC_DIR)/HttpConfig.cpp \
		$(SRC_DIR)/IHeader.cpp \
		$(SRC_DIR)/ISocket.cpp \
		$(SRC_DIR)/IStream.cpp \
		$(SRC_DIR)/IText.cpp \
		$(SRC_DIR)/LocationConfig.cpp \
		$(SRC_DIR)/Pipe.cpp \
		$(SRC_DIR)/Poll.cpp \
		$(SRC_DIR)/ReqBody.cpp \
		$(SRC_DIR)/ReqHeader.cpp \
		$(SRC_DIR)/ResBody.cpp \
		$(SRC_DIR)/ResHeader.cpp \
		$(SRC_DIR)/ServerConfig.cpp \
		$(SRC_DIR)/ServerSocket.cpp \
		$(SRC_DIR)/Timer.cpp \
		$(SRC_DIR)/core.cpp \
		$(SRC_DIR)/main.cpp \
		$(SRC_DIR)/utils.cpp


NAME = webserv

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(SRC_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_DIR)

clean :
	rm -rf $(OBJS)

fclean :
	rm -rf $(NAME) $(OBJS)

re : fclean all




