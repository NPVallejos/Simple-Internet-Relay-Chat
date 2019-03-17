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
#define ADDRESS "128.226.114.206"
#define PORT 1235

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
	gethostname(hostname, sizeof(hostname));
	
	// Step 1: Create a socket
	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("cannot create socket");
		close(fd);
		return 0;
	}

	// Step 2: Identify (name) a socket
	memset((char *) &myaddr, 0, sizeof(myaddr)); // alloc space
	
	inet_pton(AF_INET, ADDRESS, &ip4addr); // Here we are going from a "printable" address to its corresponding network format
	// lookup hostname of the server given the server address
	//hp = gethostbyaddr(&ip4addr, sizeof(ip4addr), AF_INET);
	hp = gethostbyname(hostname);
	if (!hp) {
		printf ("gethostbyaddr: failed to get host name given address");
		close(fd);
		return 0;
	}
	
	// Fill in fields for struct sockaddr_in myaddr
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(PORT);
	// now we can put the host's address into the servaddr struct
	memcpy ((void *)&myaddr.sin_addr, hp->h_addr_list[0], hp->h_length);
	//myaddr.sin_addr.s_addr = htonl(INADDR_ANY); // this does not get the correct ip address

	// now bind socket fd to myaddr
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		close(fd);
		return 0;
	}

	/* Step 3. Listen for connections to the server */
	// allow immediate reuse of PORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

	printf("server started on %s, listening on port %d. ip=%s \n", hostname, myaddr.sin_port, inet_ntoa(myaddr.sin_addr)); // To see server info

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
