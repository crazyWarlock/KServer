#ifndef K_SERVER_H__
#define K_SERVER_H__

#include "system.h"

struct Pool{
	int maxfd;
	fd_set read_set;
	fd_set ready_set;
	int nready;
	int maxi;
	int clientfd[FD_SETSIZE];
};

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

    void init_pool(int);
private:
    void start_server();
    void add_client(int);
private:
    bool on;
    Pool* pool;
};
#endif
