#include "config.h"
#include "KSPHP.h"
#include "kserver.h"
#include "logger.h"
#include <ctime>
#include <cstdio>
#include <map>
#include <unistd.h>
using namespace std;

void handler(int sig){
	pid_t pid;
	while((pid = waitpid(-1, NULL, 0)) > 0){
		;
	}
	return;
}

KServer::KServer(){
    start_server();
}

void KServer::init_pool(int sockfd){
	pool->maxi = -1;
	for(int i = 0; i < FD_SETSIZE; i ++){
		pool->clientfd[i] = -1;
	}
	pool->maxfd = sockfd;
	FD_ZERO(&pool->read_set);
	FD_SET(sockfd, &pool->read_set);
}

void KServer::add_client(int sockfd){
	int i;
	for(i = 0; i < FD_SETSIZE; i ++){
		if(pool->clientfd[i] < 0){
			pool->clientfd[i] = sockfd;

			FD_SET(sockfd, &pool->read_set);

			if(sockfd > pool->maxfd)
				pool->maxfd = sockfd;
			if(i > pool->maxi)
				pool->maxi = i;
		}
	}
	if(i == FD_SETSIZE)
		perror("add_client error: Too many clients");
}

void KServer::start_server()
{
	Logger::log("Start server. \n");
	signal(SIGCHLD, handler);

	on = true;
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int optval = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
		perror("setsocketopt");
		return;
	}

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_PORT);


	if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
		perror("bind");
		return;
	}

	listen(sockfd, backlog);


	while(on){
		struct sockaddr_in clientaddr;
		unsigned int clientlen;
		int connectfd = accept(sockfd,
				(struct sockaddr *)(&clientaddr),
				&clientlen);
		struct hostent *hp = gethostbyaddr((const char*)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		char* haddrp = inet_ntoa(clientaddr.sin_addr);
		Logger::log("server connected to %s\n", haddrp);

		//accept successfully
		if(connectfd >= 0){
			pid_t pid;

			//child process
			if((pid = fork()) == 0){
				close(sockfd);		//child closed listening socket
				handleRequest(connectfd);
				close(connectfd);
				exit(0);
			}
			//parent process
			close(connectfd);
		}
	//	waitpid(-1, NULL, WNOHANG);
	}
}

/*
 * Request				= Request-Line
 * 						  *(( general-header
 * 						   | request-header
 * 						   | entity-header ) CRLF)
 * 						   CRLF
 * 						   [message-body]
 *
 *	Request-Line = Method SP Request-URI SP HTTP-Version CRLF
 *
 *	Method				= "OPTIONS"
 *						| "GET"
 *						| "HEAD"
 *						| "POST"
 *						| ...
 *	KServer supports "GET" and "POST" methods
 *
 *	Request-URI 		= "*" | absoluteURI | abs_path | authority (only used by CONNECT method)
 *	KServer supports absoluteURI and abs_path
 *
 *
 * general-header = Cache-Control
                      | Connection
                      | Date
                      | Pragma
                      | Trailer
                      | Transfer-Encoding
                      | Upgrade
                      | Via
                      | Warning

 *	request-header = Accept
                      | Accept-Charset
                      | Accept-Encoding
                      | Accept-Language
                      | Authorization
                      | Expect
                      | From
                      | Host
                      | If-Match
                      | If-Modified-Since
                      | If-None-Match
                      | If-Range
                      | If-Unmodified-Since
                      | Max-Forwards
                      | Proxy-Authorization
                      | Range
                      | Referer
                      | TE
                      | User-Agent

 *  entity-header  = Allow
                      | Content-Encoding
                      | Content-Language
                      | Content-Length
                      | Content-Location
                      | Content-MD5
                      | Content-Range
                      | Content-Type
                      | Expires
                      | Last-Modified
                      | extension-header
       extension-header = message-header
 */
void KServer::handleRequest(int sockfd){
	ssize_t n;
	char buf[BUFFER_SIZE];
	string method;
	string request_uri;

	memset(buf, 0, sizeof(buf));

	read(sockfd, buf, BUFFER_SIZE);
#ifdef DEBUG
	printf("%s\n", buf);
#endif

	//decode Request-Line
	int i = 0;
	for(; i < strlen(buf); i ++){
		if(buf[i] != ' '){
			method.push_back(buf[i]);
		}
		else{
			break;
		}
	}
	i ++;
	for(; i < strlen(buf); i ++){
		if(buf[i] != ' '){
			request_uri.push_back(buf[i]);
		}else{
			break;
		}
	}

#ifdef DEBUG
	printf("%s %s\n", method.c_str(), request_uri.c_str());
#endif


	if(method != "GET" && method != "POST"){
#ifdef DEBUG
		printf("Error: %s is not a valid method\n", method.c_str());
#endif
		return;
	}

	string _webroot(WEB_ROOT);
	string _approot(APP_ROOT);
	string path = _webroot + '/' + _approot;
	
	if(request_uri == "/"){
		request_uri = "/" + string(DEFAULT_URI);
	}

	Logger::log("%s %s\n", method.c_str(), request_uri.c_str());
	if(method == "GET"){
		handleGET(sockfd, path, request_uri);
	}else{
		string req(buf);
		size_t found = req.find_last_of('\n');
		string pdata = req.substr(found+1);
#ifdef DEBUG
		printf("%s\n", pdata.c_str());
#endif

		handlePOST(sockfd, path, request_uri, pdata);
	}
}

void KServer::handlePOST(int sockfd, string path, string filename, string pdata){
	string file = path + filename;

	char* argvs[MAX_PARAMS];
	argvs[0] = const_cast<char*>(CGI_PATH);
	argvs[1] = const_cast<char*>(file.c_str());

	stringstream ss;
	ss<<pdata.length();
	string clen, sname;
	ss>>clen;
	clen = "CONTENT_LENGTH=" + clen;
	sname = "SCRIPT_FILENAME="+file;

	char* tmp = (char*)pdata.c_str();
	int fds[2];
	pipe(fds);
	dup2(fds[0], STDIN_FILENO);
	write(fds[1], tmp, strlen(tmp));
	close(fds[1]);

//	istringstream stream(pdata.c_str());
//	cin.rdbuf(stream.rdbuf());
//	string tss;
//	cin>>tss;
//	cout<<tss<<endl;

	char* env[] = {
			"REQUEST_METHOD=POST",
			"REDIRECT_STATUS=CGI",
			const_cast<char*>(clen.c_str()),
			const_cast<char*>(sname.c_str()),
			"CONTENT_TYPE=application/x-www-form-urlencoded",
			0
	};

	fdprintf(sockfd, "HTTP/1.0 200 OK\r\n");
	dup2(sockfd, STDOUT_FILENO);
	execve(argvs[0], argvs, env);
}

/*
 * Response      = Status-Line
                   *(( general-header
                    | response-header
                    | entity-header ) CRLF)
                   CRLF
                   [ message-body ]

 *
 * Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
 *
 *
 *  Status-Code    =
            "100"  ; Section 10.1.1: Continue
          | "101"  ; Section 10.1.2: Switching Protocols
          | "200"  ; Section 10.2.1: OK
          | "201"  ; Section 10.2.2: Created
          | "202"  ; Section 10.2.3: Accepted
          | "203"  ; Section 10.2.4: Non-Authoritative Information
          | "204"  ; Section 10.2.5: No Content
          | "205"  ; Section 10.2.6: Reset Content
          | "206"  ; Section 10.2.7: Partial Content
          | "300"  ; Section 10.3.1: Multiple Choices
          | "301"  ; Section 10.3.2: Moved Permanently
          | "302"  ; Section 10.3.3: Found
          | "303"  ; Section 10.3.4: See Other
          | "304"  ; Section 10.3.5: Not Modified
          | "305"  ; Section 10.3.6: Use Proxy
          | "307"  ; Section 10.3.8: Temporary Redirect
          | "400"  ; Section 10.4.1: Bad Request
          | "401"  ; Section 10.4.2: Unauthorized
          | "402"  ; Section 10.4.3: Payment Required
          | "403"  ; Section 10.4.4: Forbidden
          | "404"  ; Section 10.4.5: Not Found
          | "405"  ; Section 10.4.6: Method Not Allowed
          | "406"  ; Section 10.4.7: Not Acceptable
          | "407"  ; Section 10.4.8: Proxy Authentication Required
          | "408"  ; Section 10.4.9: Request Time-out
          | "409"  ; Section 10.4.10: Conflict
          | "410"  ; Section 10.4.11: Gone
          | "411"  ; Section 10.4.12: Length Required
          | "412"  ; Section 10.4.13: Precondition Failed
          | "413"  ; Section 10.4.14: Request Entity Too Large
          | "414"  ; Section 10.4.15: Request-URI Too Large
          | "415"  ; Section 10.4.16: Unsupported Media Type
          | "416"  ; Section 10.4.17: Requested range not satisfiable
          | "417"  ; Section 10.4.18: Expectation Failed
          | "500"  ; Section 10.5.1: Internal Server Error
          | "501"  ; Section 10.5.2: Not Implemented
          | "502"  ; Section 10.5.3: Bad Gateway
          | "503"  ; Section 10.5.4: Service Unavailable
          | "504"  ; Section 10.5.5: Gateway Time-out
          | "505"  ; Section 10.5.6: HTTP Version not supported
          | extension-code
 *
 */
void KServer::handleGET(int sockfd, string path, string filename)
{
	size_t count = 0;
	string file = path + filename;
	string uri = file;
	string get_param = file.substr(file.find('?')+1);
	file = file.substr(0, file.find('?'));
	const char *ext = strrchr(file.c_str(), '.');
	const char *mimetype;

	int filefd = open(file.c_str(), O_RDONLY);
	if(filefd < 0){
		http_err(sockfd, 404, "<h2>404 Not Found</h1><br>File %s does not exist.", filename.c_str());
		return;
	}
	
	if(!strcmp(ext, ".html")){
		mimetype = "text/html";
	}else if(!strcmp(ext, ".css")){
		mimetype = "text/css";
	}else if(!strcmp(ext, ".jpg") || 
			!strcmp(ext, ".jpeg") ||
			!strcmp(ext, ".jpe")){
		mimetype = "image/jpeg";
	}else if(!strcmp(ext, ".js")){
		mimetype = "application/x-javascript";
	}else if(!strcmp(ext, ".php")){
		char* argvs[MAX_PARAMS];
		argvs[0] = const_cast<char*>(CGI_PATH);
		argvs[1] = const_cast<char*>(file.c_str());
	

		string sname = "SCRIPT_FILENAME="+file;
		string qstring = "QUERY_STRING="+get_param;

		//cout<<file<<" "<<sname<<" "<<qstring<<endl;
		char* env[] = {
			"REQUEST_METHOD=GET",
			"REDIRECT_STATUS=CGI",
			const_cast<char*>(sname.c_str()),
			const_cast<char*>(qstring.c_str()),
			0
		};

		fdprintf(sockfd, "HTTP/1.0 200 OK\r\n");
		dup2(sockfd, STDOUT_FILENO);
		execve(argvs[0], argvs, env);
		return;
	}

	fdprintf(sockfd, "HTTP/1.0 200 OK\r\n");
	fdprintf(sockfd, "Content-Type: %s\r\n", mimetype);
	fdprintf(sockfd, "\r\n");

	struct stat st;
	if (!fstat(filefd, &st))
		count = st.st_size;
	if (sendfile(sockfd, filefd, 0, count) < 0){
		perror("System Error\n");
	}
	close(filefd);
	close(sockfd);
}

int KServer::handleMD(string file){

}

void KServer::http_err(int fd, int code, char *fmt, ...)
{
	fdprintf(fd, "HTTP/1.0 %d Error\r\n", code);
	fdprintf(fd, "Content-Type: text/html\r\n");
	fdprintf(fd, "\r\n");
	fdprintf(fd, "<h1>An error occurred</h1>\r\n");

	char *msg = 0;
    va_list ap;
    va_start(ap, fmt);
    vasprintf(&msg, fmt, ap);
    va_end(ap);

    fdprintf(fd, "%s\n", msg);
    close(fd);
}

void KServer::fdprintf(int fd, char* fmt, ...)
{
	char *s = 0;

	va_list ap;
	va_start(ap, fmt);
	vasprintf(&s, fmt, ap);
	va_end(ap);

	write(fd, s, strlen(s));
	free(s);
}
