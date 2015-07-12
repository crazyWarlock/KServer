/*
 * logger.h
 *
 *  Created on: Jul 12, 2015
 *      Author: yan
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "system.h"

class Logger {
public:
	static Logger* getInstance(){
		if(instance == NULL){
			return new Logger();
		}else{
			return instance;
		}
	}

	virtual ~Logger();

	static void sys_err();

private:
	Logger();

	Logger *instance;
};

#endif /* LOGGER_H_ */
