#include "response.h"

void sentResponse(int sock, char *con, char *Status_code, char *Content_Type,
		char *HTML) {
	// Set Response Headers
	char *head = "HTTP/1.1 ";
	char *server_head = "\r\nServer: ";
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
					+ strlen(conf.name) + strlen(length_head)
					+ strlen(Content_Length) + strlen(connection_head)
					+ strlen(connection) + strlen(content_head)
					+ strlen(Content_Type) + strlen(newline) + content_length
					+ sizeof(char)) * 2);

	// Built response message
	if (message != NULL) {
		strcpy(message, head);
		strcat(message, Status_code);
		strcat(message, server_head);
		strcat(message, conf.name);
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
		sentResponse(sock, r.con, "404 Not Found", "text/html",
				"<h1>HTTP Error 404</h1>");
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

