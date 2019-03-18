#include <netinet/in.h>
#include <arpa/inet.h> // for printing an internet address
#include <netdb.h> // For gethostbyaddr() function
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
#define PORT 1234

#ifndef ERESTART
#define ERESTART EINTR
#endif

int 
main(int argc, char ** argv) {
	int fd;
	int rqst; // This represents the socket accepting the request
	char hostname[128]; // Stores the hostname
	struct hostent *hp; // host information of server
	struct in_addr ip4addr; // server address - to be used by gethostbyaddr
	struct sockaddr_in myaddr; // holds information about this server 
	struct sockaddr_in client_addr; // client address
	socklen_t alen; // length of client address struct
	int sockoptval = 1;
	
	// Step 1: Create a socket
	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("cannot create socket");
		close(fd);
		return 0;
	}

	// Step 2: Identify (name) a socket
	memset((char *) &myaddr, 0, sizeof(myaddr)); // alloc space
	
	//inet_pton(AF_INET, ADDRESS, &ip4addr); // Here we are going from a "printable" address to its corresponding network format
	//hp = gethostbyaddr(&ip4addr, sizeof(ip4addr), AF_INET); // lookup hostname of the server given the server address
	gethostname(hostname, sizeof(hostname));
	hp = gethostbyname(hostname);
	if (!hp) {
		printf ("gethostbyaddr: failed to get host name given address");
		close(fd);
		return 0;
	}
	
	// Fill in fields for struct sockaddr_in myaddr
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(PORT);
	memcpy ((void *)&myaddr.sin_addr, hp->h_addr_list[0], hp->h_length); // now we can put the host's address into the servaddr struct
	
	// allow immediate reuse of PORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));
	
	// now bind socket fd to myaddr
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		close(fd);
		return 0;
	}

	/* Step 3. Listen for connections to the server */
	printf("server started on %s, listening on port %d. ip=%s \n", hostname, myaddr.sin_port, inet_ntoa(myaddr.sin_addr)); // To see server info

	/* set the socket for listening  */
	// allow up to 5 children
	if (listen(fd, 5) < 0) {
		perror("listen failed");
		exit(1);
	}

	alen = sizeof(client_addr);

	/* Step 4: Accept connection requests */
	bool endServer = false;
	
	for (;;) {
		while ((rqst = accept(fd, (struct sockaddr *)&client_addr, &alen)) < 0) {
			// if syscall 'accept' is interrupted then we break out of the while loop
			// and simply reenter and try again
			if ((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR)) {
				perror("accept failed");
				exit(1);
			}
		}
		// the socket for this accepted connection is rqst
		printf("New User[ip=%s, port=%d] has joined the chat!\n", inet_ntoa(client_addr.sin_addr),
			ntohs(client_addr.sin_port));
		
		/* Reading client data */
		char buffer[MAX]; // This will be used to store client data
		char * q = 'q';
		
		while(1) {
			read(rqst, &buffer, MAX);
			
			if(buffer[0] == 'q')
				return -1;
			else
				printf("User[ip=%s, port=%d] says:> %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
				
			// Now the server-side user can write
			bzero(buffer, MAX);
			
			printf ("Type to chat:> ");
			fgets(buffer, MAX, stdin);
			
			write (rqst, &buffer, sizeof(buffer));

			if (buffer[0] == 'q')
				endServer = true;
				
		}
		
		printf("User[ip=%s, port=%d] has left the chat\n", inet_ntoa(client_addr.sin_addr),
			ntohs(client_addr.sin_port));
			
		shutdown(rqst, SHUT_RDWR);
		
		if	(endServer)
			break;
		
		return 0;
	}
	
	close(fd);
	
	return 0;
}
