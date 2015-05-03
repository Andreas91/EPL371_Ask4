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
void SendHTML(int sock, char *Status_code, char *Content_Type, char *HTML);
void mySend(int sock, char *Status_code, char *Content_Type, char *HTML);

// Global Variables
struct config conf;
char *head_method; // GET | HEAD | DELETE
char *head_path;   // path to send
char *head_con;    // keep-alive | close

int main(int argc, char *argv[]) {

	// Variables
	int sock, newsock, serverlen, clientlen;
	struct sockaddr_in server, client;
	struct sockaddr *serverptr, *clientptr;
	struct hostent *rem;
	char buf[512];

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
	serverlen = sizeof(server);

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
		clientlen = sizeof(client);

		// Accept Connection
		if ((newsock = accept(sock, clientptr, &clientlen)) < 0) {
			perror("accept");
			exit(1);
		}

		// Find client's address
		rem = gethostbyaddr((char *) &client.sin_addr.s_addr,
				sizeof(client.sin_addr.s_addr), client.sin_family);
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
			int i = 0;
			bzero(buf, sizeof(buf)); // Initialize buffer

			// Receive msg
			if (read(newsock, buf, strlen(buf)) < 0) {
				perror("read");
				exit(1);
			}
			printf("Read string: \n%s", buf);

			bzero(buf, sizeof(buf)); // Initialize buffer

			mySend(newsock, "200 OK", "text/html",
					"<html><head></head><body><h1>Hello!</h1></body></html>");
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

void SendHTML(int sock, char *Status_code, char *Content_Type, char *HTML) {
	char *head = "HTTP/1.1 ";
	char *content_head = "\r\nContent-Type: ";
	char *server_head = "\r\nServer: Server371";
	char *connection_head = "\r\nConnection: keep-alive";
	char *length_head = "\r\nContent-Length: ";
	char *newline = "\r\n";
	char Content_Length[100];
	int content_length = strlen(HTML);

	sprintf(Content_Length, "%i", content_length);

	char *message = malloc(
			(strlen(head) + strlen(content_head) + strlen(server_head)
					+ strlen(length_head) + strlen(newline)
					+ strlen(Status_code) + strlen(Content_Type)
					+ strlen(Content_Length) + content_length + sizeof(char))
					* 2);

	if (message != NULL) {
		strcpy(message, head);
		strcat(message, Status_code);
		strcat(message, content_head);
		strcat(message, Content_Type);
		strcat(message, server_head);
		strcat(message, connection_head);
		strcat(message, length_head);
		strcat(message, Content_Length);
		strcat(message, newline);
		strcat(message, HTML);

		printf("\nSending:\n%s", message);
		printf("\n----------------\n");

		// Send Msg
		if (write(sock, message, sizeof(message)) < 0) {
			perror("write");
			exit(1);
		}
		free(message);
	}
}

void mySend(int sock, char *Status_code, char *Content_Type, char *HTML) {
	char *head = "HTTP/1.1 ";
	char *server_head = "\r\nServer: Server371";
	char *length_head = "\r\nContent-Length: ";
	char *connection_head = "\r\nConnection: keep-alive";
	char *content_head = "\r\nContent-Type: ";
	char *newline = "\r\n";
	char Content_Length[100];
	int content_length = strlen(HTML);

	sprintf(Content_Length, "%i", content_length);

	char *message = malloc(
			(strlen(head) + strlen(Status_code) + strlen(server_head)
					+ strlen(length_head) + strlen(Content_Length)
					+ strlen(connection_head) + strlen(content_head)
					+ strlen(Content_Type) + strlen(newline) + content_length
					+ sizeof(char)) * 2);

	if (message != NULL) {
		strcpy(message, head);
		strcat(message, Status_code);
		strcat(message, server_head);
		strcat(message, length_head);
		strcat(message, Content_Length);
		strcat(message, connection_head);
		strcat(message, content_head);
		strcat(message, Content_Type);
		strcat(message, newline);
		strcat(message, newline);
		strcat(message, HTML);

		printf("\nSending:\n%s", message);
		printf("\n----------------\n");

		// Send Msg
		if (write(sock, message, strlen(message)) < 0) {
			perror("write");
			exit(1);
		}
		free(message);
	}
}
