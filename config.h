
typedef unsigned int uint32_t;

const unsigned int BUFFER_SIZE = 8192; //8KBytes
const unsigned short SERVER_PORT = 8080;
const int backlog = 60; //hold 60 access simutaneously
const char log_file[] = "/home/yan/KServer/log.txt";
const char WEB_ROOT[] = "/home/yan/www";
const char APP_ROOT[] = "blog";
const char DEFAULT_URI[] = "index.php";
const char CGI_PATH[] = "/usr/bin/php5-cgi";
const unsigned int MAX_PARAMS = 1024;

