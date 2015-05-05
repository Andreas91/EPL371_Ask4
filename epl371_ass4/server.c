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
};
struct streq {
	char method[15]; // GET | HEAD | DELETE
	char path[256];   // path to send
	char con[15];    // keep-alive | close
};

// Functions Prototypes
void setConfig();
void printConfig();
char *get_filename_ext(char *filename);
char *get_content_type(char *filename);
void *connection_handler(void *socket_desc);
void getRequest(int sock, struct streq *r);
void sentResponse(int sock, char *con, char *Status_code, char *Content_Type,
		char *HTML);
void responseGet(int sock, struct streq r);
void responseHead(int sock, struct streq r);
void responseDelete(int sock, struct streq r);

// Global Variables
struct config conf;

int main(int argc, char *argv[]) {

	// Variables
	int sock, newsock, serverlen, c, *new_sock;
	struct sockaddr_in server, client;
	struct sockaddr *serverptr;
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

	c = sizeof(struct sockaddr_in);
	while ((newsock = accept(sock, (struct sockaddr *) &client, (socklen_t*) &c))) {

		// Find client's address
		rem = gethostbyaddr((char *) &client.sin_addr.s_addr,
				sizeof(client.sin_addr.s_addr), client.sin_family);
		if (rem == NULL) {
			perror("gethostbyaddr");
			exit(1);
		}
		printf("Accepted connection from %s\n", rem->h_name);

		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = newsock;

		if (pthread_create(&sniffer_thread, NULL, connection_handler,
				(void*) new_sock) < 0) {
			perror("could not create thread");
			exit(1);
			;
		}

		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( sniffer_thread , NULL);
		printf("Handler assigned\n");
	}

	// Accept Connection Failed
	if (newsock < 0) {
		perror("accept failed");
		return 1;
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

/**
 * Returns the extension of the filename or null.
 * @param filename Filename to find it's extension.
 * @return Filename's extension or null.
 */
char *get_filename_ext(char *filename) {
	char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	return dot + 1;
}

/**
 * By using the get_filename_ext, this function
 * returns the content_type of the given filename.
 * @param filename Filename to find it's content-type.
 * @return Filename's content-type.
 */
char *get_content_type(char *filename) {
	char *ext = get_filename_ext(filename);

	if (strcmp(ext, "txt") == 0 || strcmp(ext, "sed") == 0
			|| strcmp(ext, "awk") == 0 || strcmp(ext, "c") == 0
			|| strcmp(ext, "h") == 0) {
		return ("text/plain");
	}

	if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) {
		return ("text/html");
	}

	if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0) {
		return ("image/jpeg");
	}

	if (strcmp(ext, "png") == 0) {
		return ("image/png");
	}

	if (strcmp(ext, "css") == 0) {
		return ("text/css");
	}

	if (strcmp(ext, "js") == 0) {
		return ("text/javascript");
	}

	if (strcmp(ext, "gif") == 0) {
		return ("image/gif");
	}

	if (strcmp(ext, "pdf") == 0) {
		return ("application/pdf");
	}

	return "application/octet-stream";
}

/**
 * Handles connection for each client
 * @param socket_desc
 */
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

/**
 * The functions builds the appropriate response base on the given
 * arguments and writes on the given socket.
 * @param sock Given socket to response to.
 * @param Status_code Status Code for the response (e.g. "200 OK").
 * @param Content_Type Content type for the response (e.g. "text/plain").
 * @param HTML The raw content for the response.
 */
void sentResponse(int sock, char *con, char *Status_code, char *Content_Type,
		char *HTML) {
	// Set Response Headers
	char *head = "HTTP/1.1 ";
	char *server_head = "\r\nServer: Server371";
	char *length_head = "\r\nContent-Length: ";
	char *connection_head = "\r\nConnection: ";
	char *content_head = "\r\nContent-Type: ";
	char *newline = "\r\n";
	char Content_Length[100];
	int content_length = strlen(HTML);
	char connection[100];
	if (strcmp(con, "keep-alive") == 0)
		sprintf(connection, "%s", "keep-alive");
	else
		sprintf(connection, "%s", "close");

	// Malloc for response message
	sprintf(Content_Length, "%i", content_length);
	char *message = malloc(
			(strlen(head) + strlen(Status_code) + strlen(server_head)
					+ strlen(length_head) + strlen(Content_Length)
					+ strlen(connection_head) + strlen(connection)
					+ strlen(content_head) + strlen(Content_Type)
					+ strlen(newline) + content_length + sizeof(char)) * 2);

	// Built response message
	if (message != NULL) {
		strcpy(message, head);
		strcat(message, Status_code);
		strcat(message, server_head);
		strcat(message, length_head);
		strcat(message, Content_Length);
		strcat(message, connection_head);
		strcat(message, connection);
		strcat(message, content_head);
		strcat(message, Content_Type);
		strcat(message, newline);
		strcat(message, newline);
		strcat(message, HTML);

		// Send message
		if (write(sock, message, strlen(message)) < 0) {
			perror("write");
			exit(1);
		}
		free(message);
	}
	printf("Response: %s %s (%s)\n", head, Status_code, connection);
}

void responseGet(int sock, struct streq r) {
	struct stat s;
	int exists = stat(r.path, &s);
	if (exists == -1) {
		sentResponse(sock, r.con, "404 Not Found", "text/html", "<h1>HTTP Error 404</h1>");
	} else {
		// Directory listing denied, show 403
		if (S_ISDIR(s.st_mode)) {
			sentResponse(sock, r.con, "403 Forbidden", "text/plain", "");
		} else {
			// Get file's content-type
			char *ctype = get_content_type(r.path);

			// Get file's content
			char *buffer = 0;
			long length;
			FILE * f = fopen(r.path, "rb");

			if (f) {
				fseek(f, 0, SEEK_END);
				length = ftell(f);
				fseek(f, 0, SEEK_SET);
				buffer = malloc(length);
				if (buffer) {
					fread(buffer, 1, length, f);
				}
				fclose(f);
			}
			sentResponse(sock, r.con, "200 OK", ctype, buffer);
		}
	}
}

/**
 * Checks if the requested path exists. If it does and it's a
 * file, it sends 200 OK, if the path is a directory, it sends
 * 403 Forbidden and if it doesn't exists it sends 404 Not Found.
 * @param sock Given socket to response to.
 */
void responseHead(int sock, struct streq r) {
	struct stat s;
	int exists = stat(r.path, &s);
	if (exists == -1) {
		sentResponse(sock, r.con, "404 Not Found", "text/plain", "");
	} else {
		// Directory listing denied, show 403
		if (S_ISDIR(s.st_mode)) {
			sentResponse(sock, r.con, "403 Forbidden", "text/plain", "");
		} else {
			sentResponse(sock, r.con, "200 OK", "text/plain", "");
		}
	}
}

/**
 * Tries to delete a file. It response as follows:
 * Exists & Is File & Deleted: 200.
 * Exists & Is File & Not Deleted: 500.
 * Exists & Is Folder: 403.
 * Not Exists: 404.
 * @param sock Given socket to response to.
 */
void responseDelete(int sock, struct streq r) {
	struct stat s;
	int exists = stat(r.path, &s);
	if (exists == -1) {
		sentResponse(sock, r.con, "404 Not Found", "text/plain", "");
	} else {
		// Directory delete denied, show 403
		if (S_ISDIR(s.st_mode)) {
			sentResponse(sock, r.con, "403 Forbidden", "text/plain", "");
		} else {
			// try to delete requested file
			int status = remove(r.path);
			if (status == 0) {
				sentResponse(sock, r.con, "200 OK", "text/plain",
						"File Deleted\n");
			} else {
				sentResponse(sock, r.con, "500 Internal Error", "text/plain",
						"Unable to delete the file\n");
			}
		}
	}
}
