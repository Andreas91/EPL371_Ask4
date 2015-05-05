#include "def.h"

#ifndef RESPONSE_H_
#define RESPONSE_H_

/**
 * The functions builds the appropriate response base on the given
 * arguments and writes on the given socket.
 * @param sock Given socket to response to.
 * @param con The connection that the client wants(keep-alive/close)
 * @param Status_code Status Code for the response (e.g. "200 OK").
 * @param Content_Type Content type for the response (e.g. "text/plain").
 * @param HTML The raw content for the response.
 */
void sentResponse(int sock, char *con, char *Status_code, char *Content_Type,
		char *HTML);


/**
 * The function response to the GET request of 
 * a client to the given socket.
 * @param sock Socket to response to the client.
 * @param r Request struct from the client.
 */
void responseGet(int sock, struct streq r);

/**
 * Checks if the requested path exists. If it does and it's a
 * file, it sends 200 OK, if the path is a directory, it sends
 * @param sock Given socket to response to.
 * @param r 
 */
void responseHead(int sock, struct streq r);


/**
 * Tries to delete a file. It response as follows:
 * Exists & Is File & Deleted: 200.
 * Exists & Is File & Not Deleted: 500.
 * Exists & Is Folder: 403.
 * Not Exists: 404.
 * @param sock Given socket to response to.
 * @param r Request struct from the client.
 */
void responseDelete(int sock, struct streq r);

#endif /* RESPONSE_H_ */
