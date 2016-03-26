#ifndef K_SERVER_H__
#define K_SERVER_H__

#include "system.h"

class KServer
{
public:
    KServer();

    void handleRequest(int sockfd);

    void handleGET(int sockfd, std::string path, std::string filename);
    void handlePOST(int sockfd, std::string path, std::string filename, std::string);

    void fdprintf(int fd, char* fmt, ...);
    void http_err(int fd, int code, char* fmt, ...);

    int handleMD(std::string);
    inline void close_server(){ on = false; }
private:
    void start_server();
	
private:
    bool on;
};
#endif
