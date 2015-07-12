#ifndef K_SERVER_H__
#define K_SERVER_H__

#include "system.h"

class KServer
{
public:
    KServer();

    void handleRequest(int sockfd);

    void handleGET(int sockfd, std::string path, std::string filename);
    void handlePOST(std::string path);

    void fdprintf(int fd, char* fmt, ...);
    void http_err(int fd, int code, char* fmt, ...);
private:
    void start_server();
};
#endif
