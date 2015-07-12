
typedef unsigned int uint32_t;

const unsigned char _SERVER_ADDRESS[4]={202,120,40,229};
const uint32_t SERVER_ADDRESS = (
		_SERVER_ADDRESS[3]+
		_SERVER_ADDRESS[2]<<8+
		_SERVER_ADDRESS[1]<<16+
		_SERVER_ADDRESS[0]<<24);

const unsigned short SERVER_PORT = 8080;
const int backlog = 60;
const char* log_file = "/home/yan/server/log.txt";
const char* WEB_ROOT = "/home/yan/www";
const char* APP_ROOT = "blog";
