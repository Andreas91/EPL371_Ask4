// Libraries
#include <sys/types.h> // For sockets
#include <sys/socket.h> // For sockets
#include <netinet/in.h> // For Internet sockets
#include <netdb.h> // For gethostbyaddr
#include <stdio.h> // For I/O
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
};

// Functions Prototypes
void setConfig();
void printConfig();

// Global Variables
struct config conf;

int main(int argc, char *argv[]) {
	int sock, serverlen;
	struct sockaddr_in server;
	struct sockaddr *serverptr;

	setConfig();
	printConfig();

	// Create socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	
	server.sin_family = AF_INET; // Internet domain
	server.sin_addr.s_addr = htonl(INADDR_ANY); // My Internet address
	server.sin_port = htons(conf.port);
	serverptr = (struct sockaddr *) &server;
	serverlen = sizeof server;
	
	// Bind socket to an address
	if (bind(sock, serverptr, serverlen) < 0) {
		perror("bind");
		exit(1);
	}
	
	// Listen for connections
	if (listen(sock, 5) < 0) {
		perror("listen");
		exit(1);
	}

	printf("Listening for connections to port %d\n", conf.port);
	
	while(1){
		
	}

	return 0;
}

/**
 * Sets the configurations of the server base on the
 * filename in 'FCONFIG' constant.
 */
void setConfig() {
	FILE *file = fopen(FCONFIG, "r");

	if (file != NULL) {
		char line[MAXBUF];
		int i = 0;

		while (fgets(line, sizeof(line), file) != NULL) {
			char *cfline;
			cfline = strstr((char *) line, DELIM);
			cfline = cfline + strlen(DELIM);

			if (i == 0) {
				char temp_treads[MAXBUF];
				memcpy(temp_treads, cfline, strlen(cfline));
				conf.num_treads = atoi(temp_treads);
			} else if (i == 1) {
				char temp_port[MAXBUF];
				memcpy(temp_port, cfline, strlen(cfline));
				conf.port = atoi(temp_port);
			}
			i++;
		}
	}
	fclose(file);
}

/**
 * Prints the configuration information of the server.
 * The configuration includes: Number of threads and port number.
 */
void printConfig() {
	printf("SERVER CONFIGURATION\n");
	printf("====================\n");
	printf("Number of Threads: %d\n", conf.num_treads);
	printf("Port Number: %d\n\n", conf.port);
	printf("-------------------------\n\n");
}
