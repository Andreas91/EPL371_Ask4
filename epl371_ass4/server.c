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
void getRequest(int sock);
void sentResponse(int sock, char *Status_code, char *Content_Type, char *HTML);
void response404(int sock);
void response501(int sock);
void responseGet(int sock);
void responseHead(int sock);
void responseDelete(int sock);

// Global Variables
struct config conf;
char req_method[100]; // GET | HEAD | DELETE
char req_path[100];   // path to send
char req_con[100];    // keep-alive | close

int main(int argc, char *argv[]) {

	// Variables
	int sock, newsock, serverlen, clientlen;
	struct sockaddr_in server, client;
	struct sockaddr *serverptr, *clientptr;
	struct hostent *rem;

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
			// While connection: keep-alive
			do {
				// Get Request
				getRequest(newsock);

				// Check Request Method
				int method = -1;
				if (strcmp(req_method, "GET") == 0) method = 1;
				else if(strcmp(req_method, "HEAD") == 0) method = 2;
				else if(strcmp(req_method, "DELETE") == 0) method = 3;
				else method = -1;
				
				// Response by method
				switch (method){
					case 1: responseGet(newsock); break;
					case 2: responseHead(newsock); break;
					case 3: responseDelete(newsock); break;
					case -1: response501(newsock);break;
				}
				
			} while (strcmp(req_con, "keep-alive") == 0);
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

void getRequest(int sock) {
	char buf[512];
	bzero(buf, sizeof(buf)); // Initialize buffer

	// Read from socket
	if (read(sock, buf, sizeof(buf)) < 0) {
		perror("read");
		exit(1);
	}
	printf("Read: \n%s", buf);

	// Tokenize Headers
	char *line = strtok(buf, " \r\n");

	// Get Request method
	strcpy(req_method, line);
	line = strtok(NULL, " \r\n");

	// Get Request path
	strcpy(req_path, line);
	line = strtok(NULL, " \r\n");

	// Get Request Connection
	while (line != NULL) {
		if (strcmp(line, "Connection:") == 0) {
			line = strtok(NULL, " \r\n");
			strcpy(req_con, line);
		}
		line = strtok(NULL, " \r\n");
	}

	printf("Request Method: %s\n", req_method);
	printf("Request Path: %s\n", req_path);
	printf("Request Connection: %s\n", req_con);
}

void sentResponse(int sock, char *Status_code, char *Content_Type, char *HTML) {
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

void response404(int sock){
	sentResponse(sock, "404 Not Found", "text/plain", "Document not found!");
}

void response501(int sock){
	sentResponse(sock, "501 Not Implemented", "text/plain", "Method not implemented!");
}

void responseGet(int sock){
	sentResponse(sock, "200 OK", "text/plain", "OK GET");
}

void responseHead(int sock){
	sentResponse(sock, "200 OK", "text/plain", "OK Head");
}

void responseDelete(int sock){
	sentResponse(sock, "200 OK", "text/plain", "OK Delete");
}
