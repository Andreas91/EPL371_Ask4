#include "def.h"

#ifndef REQUEST_H_
#define REQUEST_H_

/**
 * Handles connection for each client
 * @param socket_desc
 */
void *connection_handler(void *socket_desc);

/**
 * Reads the client's request from the given
 * socket and fills the given request struct.
 * @param sock Socket to read request from.
 * @param r Request struct to break up the request.
 */
void getRequest(int sock, struct streq *r);

#endif /* REQUEST_H_ */
