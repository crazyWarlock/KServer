/*
 * logger.h
 *
 *  Created on: Jul 12, 2015
 *      Author: yan
 */

#ifndef LOGGER_H_
#define LOGGER_H_

class Logger {
public:
	/*
	static Logger* getInstance(){
		if(!instance){
			return new Logger();
		}else{
			return instance;
		}
	}*/

	void logtime();
    static void log(char* fmt, ...);
	
	~Logger();

private:
	Logger();
	
	static int filefd;
	//static Logger *instance;
};

#endif /* LOGGER_H_ */
