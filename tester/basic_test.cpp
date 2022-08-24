#include <iostream>
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>

# define _NC				"\033[0m"
# define _UL				"\033[4m"
# define _RED 				"\033[1;31m"
# define _BLUE 				"\033[1;34m"
# define _GRAY 				"\033[0;90m"
# define _CYAN 				"\033[1;36m"
# define _GREEN				"\033[0;32m"
# define _YELLOW			"\033[0;33m"
# define _PURPLE			"\033[0;35m"
# define UL(s)				_UL s _NC
# define RED(s) 			_RED s _NC
# define BLUE(s) 			_BLUE s _NC
# define GRAY(s) 			_GRAY s _NC
# define CYAN(s) 			_CYAN s _NC
# define GREEN(s)			_GREEN s _NC
# define YELLOW(s)			_YELLOW s _NC
# define PURPLE(s)			_PURPLE s _NC


#define SERVERIP "127.0.0.1"
#define SERVERPORT 8000
#define BUFSIZE 10000

int main(int argc, char *argv[])
{
    int             sock;
    short           port;
    bool            keep;
    sockaddr_in     info;
    std::string     sendBuf = "";
    std::string     readBuf = "";     
    int             byte;
    int             fd;
    std::ifstream   fin;
    char            buf[BUFSIZE];
    
    if (argc != 3)
    {
        std::cout << "useage : port add file path" << std::endl;
        return 1;
    }
    port = atoi(argv[1]);
    std::cout << "port : " << port << std::endl;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&info, sizeof(sockaddr_in));
    info.sin_family = PF_INET;
    info.sin_addr.s_addr = inet_addr(SERVERIP);
    info.sin_port = htons(port);
    if (connect(sock, (struct sockaddr *)&info, sizeof(sockaddr_in)) != 0)
    {
        std::cout << "Error connect" << std::endl;
        return 1;
    }

    fin.open(argv[2]);
    if (!fin.is_open())
        return 1;
    
    
    while (getline(fin, readBuf))
    {
        sendBuf.append(readBuf);
        sendBuf.append("\r\n");
    }
    sendBuf.append("\r\n");
    fin.close();

    std::cout << "=============================" << std::endl;
    std::cout << _BLUE << sendBuf << _NC;

    byte = write(sock, sendBuf.c_str(), sendBuf.length());
    sleep(1);
    bzero(buf, BUFSIZE);
    byte = read(sock, buf, BUFSIZE);
    std::cout << _GREEN << buf  << _NC << std::endl;
    std::cout << "=============================" << std::endl;
    return (0);
}

