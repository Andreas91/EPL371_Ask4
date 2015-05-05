#include "def.h"

#ifndef CONFIG_H_
#define CONFIG_H_

/**
 * Sets the configurations of the server base on the
 * filename in 'FCONFIG' constant.
 */
void setConfig();

/**
 * Prints the configuration information of the server.
 * The configuration includes: Number of threads, port 
 * number and the server's name.
 */
void printConfig();

#endif /* CONFIG_H_ */
