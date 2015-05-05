#ifndef DEF_H_
#define DEF_H_

// Libraries
#include <sys/types.h>   // For sockets
#include <sys/socket.h>  // For sockets
#include <netinet/in.h>  // For Internet sockets
#include <netdb.h>       // For get host by address
#include <pthread.h>     // For threading
#include <stdio.h>       // For I/O
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// Global Constants
#define FCONFIG "config.txt"
#define MAXBUF 1024
#define DELIM "="

// Structs
struct config {
	int num_treads;
	int port;
	char name[MAXBUF];
};
struct streq {
	char method[15]; // GET | HEAD | DELETE
	char path[256];   // path to send
	char con[15];    // keep-alive | close
};

// Global Variables
struct config conf;

#endif /* DEF_H_ */
