/*
 * logger.cpp
 *
 *  Created on: Jul 12, 2015
 *      Author: yan
 */

#include "logger.h"
#include "config.h"
#include "system.h"

int Logger::filefd = -1;
//Logger* Logger::instance = NULL;

Logger::Logger() {
	if((filefd = open(log_file, O_WRONLY | O_CREAT | O_APPEND, S_IWUSR)) < 0){
		perror("open fail");
		return;
	}
}

void Logger::log(char* fmt, ...){
	static Logger logger;

	logger.logtime();
	char *s = 0;
	va_list ap;
	va_start(ap, fmt);
	vasprintf(&s, fmt, ap);
	va_end(ap);

	write(filefd, s, strlen(s));
	free(s);
}

void Logger::logtime(){
	time_t t = time(0);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "[ %Y/%m/%d %H:%M:%S ], ", localtime(&t));
	write(filefd, tmp, strlen(tmp));
}

Logger::~Logger() {
	if(filefd != -1) close(filefd);
}

