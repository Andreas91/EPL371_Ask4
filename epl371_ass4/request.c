#include "request.h"

void *connection_handler(void *socket_desc) {

	int sock = *(int*) socket_desc; //Get the socket descriptor
	struct streq req;

	// While connection: keep-alive
	do {
		// Read the Request
		getRequest(sock, &req);

		// Response by method
		if (strcmp(req.method, "GET") == 0)
			responseGet(sock, req);
		else if (strcmp(req.method, "HEAD") == 0)
			responseHead(sock, req);
		else if (strcmp(req.method, "DELETE") == 0)
			responseDelete(sock, req);
		else
			sentResponse(sock, req.con, "501 Not Implemented", "text/plain",
					"Method not implemented!\r\n");

	} while (strcmp(req.con, "keep-alive") == 0);
	printf("Client disconnected\n");

	//Free the socket pointer
	free(socket_desc);
	return 0;
}

void getRequest(int sock, struct streq *r) {
	char buf[512];
	bzero(buf, sizeof(buf)); // Initialize buffer

	// Read from socket
	if (read(sock, buf, sizeof(buf)) < 0) {
		perror("read");
		exit(1);
	}

	// Tokenize Headers
	char *line = strtok(buf, " \r\n");

	// Get Request method
	strcpy(r->method, line);
	line = strtok(NULL, " \r\n");

	// Get Request path
	strcpy(r->path, "anyplace");
	strcat(r->path, line);
	if (strcmp(line, "/") == 0) {
		strcat(r->path, "index.html");
	}
	line = strtok(NULL, " \r\n");

	// Get Request Connection
	while (line != NULL) {
		if (strcmp(line, "Connection:") == 0) {
			line = strtok(NULL, " \r\n");
			strcpy(r->con, line);
		}
		line = strtok(NULL, " \r\n");
	}
	printf("--------------------------------------------------\n");
	printf("Request: %s %s (%s)\n", r->method, r->path, r->con);
}
