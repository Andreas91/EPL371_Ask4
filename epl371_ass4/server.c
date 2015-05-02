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

	// Variables
	int sock, newsock, serverlen, clientlen;
	struct sockaddr_in server, client;
	struct sockaddr *serverptr, *clientptr;
	struct hostent *rem;
	char buf[256];

	// Set and print server's config
	setConfig();
	printConfig();

	// Create socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	server.sin_family = AF_INET; // Internet domain
	server.sin_addr.s_addr = htonl(INADDR_ANY); // My Internet address
	server.sin_port = htons(conf.port); // The config port
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

	while (1) {
		clientptr = (struct sockaddr *) &client;
		clientlen = sizeof client;

		// Accept Connection
		if ((newsock = accept(sock, clientptr, &clientlen)) < 0) {
			perror("accept");
			exit(1);
		}

		// Find client's address
		rem = gethostbyaddr((char *) &client.sin_addr.s_addr,
				sizeof client.sin_addr.s_addr, client.sin_family);
		if (rem == NULL) {
			perror("gethostbyaddr");
			exit(1);
		}
		printf("Accepted connection from %s\n", rem->h_name);

		// Create child for serving the client
		switch (fork()) {
			case -1: {
				perror("fork");
				exit(1);
			}
			case 0: {
				bzero(buf, sizeof buf); // Initialize buffer
				
				// Receive msg
				if (read(newsock, buf, sizeof buf) < 0) {
					perror("read");
					exit(1);
				}
				printf("Read string: %s\n", buf);
				
				
			}
		}

		// Close Socket
		close(newsock);
		printf("Connection from %s is closed\n", rem->h_name);
		exit(0);
	}

	return 0;
}

/**
 * Sets the configurations of the server base on the
 * filename in 'FCONFIG' constant.
 */
void setConfig() {
	FILE *file = fopen(FCONFIG, "r");

	if (file == NULL) {
		printf("*** Unable to configure server! System will now exit. ***\n");
		exit(0);
	}
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
