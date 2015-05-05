# EPL371 - Askisi 4: Multithreaded HTTP Server in C
---------------------------------------------------

Intro:
======
   The entry point of the server (main function) is at the 'server.c' file.
The server is configured by the config.txt file which is handled by the 
config.c. The server listens for connections at the configured port and handles
the requests using the request.c. The response to the client is handled by the
response.c.

For each connection to a new client, a handler is assigned and a thread is created.
According to the client's requests, the handler uses the appropriate response function
from the response.c.

Run From Eclipse:
=================
   To add pthread library to your project flow these steps (in eclipse):

right click on the project in the project explorer
properties -> c/c++ build -> Settings -> linker -> libraries -> add -> pthread -> ok -> rebuild

Run From Terminal:
==================
1) make all
2) ./server