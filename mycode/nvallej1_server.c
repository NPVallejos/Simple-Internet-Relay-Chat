#include <netinet/in.h>
#include <arpa/inet.h> // for printing an internet address
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h> // for bool types
#include <sys/wait.h> // for wait()
#include <unistd.h> // for close() and gethostname()

#define MAX 256

#ifndef ERESTART
#define ERESTART EINTR
#endif

int 
main(int argc, char ** argv) {
	char hostname[128]; // Get the host name
	gethostname(hostname, sizeof(hostname));

	int fd;
	
	// Step 1: Create a socket
	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("cannot create socket");
		close(fd);
		return 0;
	}

	// Step 2: Identify (name) a socket
	struct sockaddr_in myaddr; // address of this service
	int port = 1235; 

	memset((char *) &myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(port);
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		close(fd);
		return 0;
	}

	/* Step 3b. Accept connections on the server */
	int rqst; // This represents the socket accepting the request
	socklen_t alen; // length of address struct
	struct sockaddr_in client_addr; // client address
	int sockoptval = 1;


	// allow immeiate reuse of the port
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

	printf("server started on %s, listening on port %d. ip=%s \n", hostname, 128, inet_ntoa(myaddr.sin_addr)); // To see server info

	/* set the socket for listening  */
	if (listen(fd, 5) < 0) {
		perror("listen failed");
		exit(1);
	}

	alen = sizeof(client_addr);

	/* Here is where we accept connection requests */
	bool endServer = false;
	pid_t pid;
	for (;;) {
		while ((rqst = accept(fd, (struct sockaddr *)&client_addr, &alen)) < 0) {
			// if syscall 'accept' is interrupted then we break out of the while loop
			// and simply reenter and try again
			if ((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR)) {
				perror("accept failed");
				exit(1);
			}
		}
		// fork here and let parent continue to accept more clients
			
		// allow up to 5 children
		
		// Let child deal with the connection
		// the socket for this accepted connection is rqst
		printf("New User[ip=%s, port=%d] has joined the chat!\n", inet_ntoa(client_addr.sin_addr),
			ntohs(client_addr.sin_port));
		
		/* Reading client data */
		char buffer[MAX]; // This will be used to store client data
		char * q = 'q';
		
		while(1) {
			read(rqst, &buffer, MAX);
			
			if(buffer[0] == 'q')
				break;
			else
				printf("User[ip=%s, port=%d] says:> %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
			
			// Now the server-side user can write
			bzero(buffer, MAX);
			
			printf ("Type to chat:> ");
			fgets(buffer, MAX, stdin);
			
			if (buffer[0] == 'q')
				endServer = true;
				
			write (rqst, &buffer, sizeof(buffer));
		}
		
		printf("User[ip=%s, port=%d] has left the chat\n", inet_ntoa(client_addr.sin_addr),
			ntohs(client_addr.sin_port));
			
		shutdown(rqst, SHUT_RDWR);
		
		if	(endServer)
			break;
	}
	
	close(fd);
	
	return 0;
}
