/**
 * @file server.
 * @brief Multithreaded HTTP1.1 Server in C
 * 
 * This is the solution of the exercise 4 
 * (EPL371-2015), involving creating a
 * multithreaded HTTP server in C language.
 * 
 * @author Andreas Andreou(ID: 983759)
 */

#include "def.h"
#include "config.h"
#include "types.h"
#include "request.h"
#include "response.h"

/**
 * @brief Program's entry point.
 * 
 * This is the entry point of the server.
 * 
 * @param argc Length of arguments.
 * @param argv Arguments table.
 * @return Nothing important.
 */
int main(int argc, char *argv[]) {

	// Variables
	int sock, newsock, serverlen, *new_sock, clientlen;
	struct sockaddr_in server, client;
	struct sockaddr *serverptr, *clientptr;
	struct hostent *rem;

	// Set and print server's configuration
	setConfig();
	printConfig();

	// Create socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET; // Internet domain
	server.sin_addr.s_addr = htonl(INADDR_ANY); // My Internet address
	server.sin_port = htons(conf.port); // The config port
	serverptr = (struct sockaddr *) &server;
	serverlen = sizeof(server);

	// Bind socket to an address
	if (bind(sock, serverptr, serverlen) < 0) {
		perror("bind");
		exit(1);
	}

	// Listen for connections
	if (listen(sock, 3) < 0) {
		perror("listen");
		exit(1);
	}

	//Accept and incoming connection
	printf("Listening for connections to port %d\n", conf.port);
	while (1) {

		// Accept connection
		clientptr = (struct sockaddr *) &client;
		clientlen = sizeof(client);
		if ((newsock = accept(sock, clientptr, (socklen_t*) &clientlen)) < 0) {
			perror("accept failed");
			return 1;
		}

		// Find client's address
		rem = gethostbyaddr((char *) &client.sin_addr.s_addr,
				sizeof(client.sin_addr.s_addr), client.sin_family);
		if (rem == NULL) {
			perror("gethostbyaddr");
			exit(1);
		}
		printf("Accepted connection from %s\n", rem->h_name);
		
		/* Create child for serving the client */
		switch (fork()) {
		case -1: {
			perror("fork");
			exit(1);
		}
		case 0: {
			pthread_t sniffer_thread;
			new_sock = malloc(1);
			*new_sock = newsock;

			if (pthread_create(&sniffer_thread, NULL, connection_handler,
					(void*) new_sock) < 0) {
				perror("could not create thread");
				exit(1);
			}
			printf("Handler assigned\n");
		}
		}
	}
	return 0;
}
