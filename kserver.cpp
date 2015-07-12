#include "config.h"
#include "kserver.h"
#include <ctime>
#include <cstdio>
#include <unistd.h>
using namespace std;

KServer::KServer(){
	this->start_server();
}

void KServer::start_server()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_PORT);

	bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	listen(sockfd, backlog);

	while(1){
		int connectfd = accept(sockfd, (struct sockaddr *)NULL, NULL);

		if(connectfd >= 0){
			pid_t pid;

			//child process
			if((pid = fork()) == 0){
				close(sockfd);		//child closed listening socket
				this->handleRequest(connectfd);
				close(connectfd);
				exit(0);
			}

			//parent process
			close(connectfd);

		}
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
	char buf[8192];
	string method;
	string request_uri;

	memset(buf, 0, sizeof(buf));

	read(sockfd, buf, 8192);
//	printf("%s\n", buf);

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

	printf("%s %s\n", method.c_str(), request_uri.c_str());

	if(method != "GET" && method != "POST"){
		printf("Error: %s is not a valid method\n", method.c_str());
	}

	string _webroot(WEB_ROOT, strlen(WEB_ROOT));
	string _approot(APP_ROOT, strlen(APP_ROOT));
	string path = _webroot + '/' + _approot;

	if(request_uri == "/"){
		request_uri = "/index.html";
	}


	if(method == "GET"){
		this->handleGET(sockfd, path, request_uri);
	}
//	char timebuffer[129];
//	time_t currentTime = time(NULL);
//	snprintf(timebuffer, 128, "%s\n", ctime(&currentTime));
//	write(sockfd, timebuffer, strlen(timebuffer));
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
	off_t offset = 0;
	int filefd = open((path + filename).c_str(), O_RDONLY);

	if(filefd < 0){
		http_err(sockfd, 404, "<h2>404 Not Found</h2><br>File %s does not exist.", filename.c_str());
		return;
	}

	const char *ext = strrchr(filename.c_str(), '.');
	const char *mimetype = "text/html";
	if(ext && !strcmp(ext, ".css")){
		mimetype = "text/css";
	}
	if(ext && !strcmp(ext, ".jpg")){
		mimetype = "text/jpeg";
	}

	fdprintf(sockfd, "HTTP/1.0 200 OK\r\n");
	fdprintf(sockfd, "Content-Type: %s\r\n", mimetype);
	fdprintf(sockfd, "\r\n");

	struct stat st;
	if (!fstat(filefd, &st))
		offset = st.st_size;
	if (sendfile(sockfd, filefd, 0, offset) < 0){
		printf("System Error\n");
	}
	close(filefd);
}

void KServer::http_err(int fd, int code, char *fmt, ...)
{
	fdprintf(fd, "HTTP/1.0 %d Error\r\n", code);
	fdprintf(fd, "Content-Type: text\html\r\n");
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
