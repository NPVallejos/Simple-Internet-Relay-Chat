#include <netinet/in.h>
#include <arpa/inet.h> // for printing an internet address
#include <netdb.h> // For gethostbyaddr() function
#include <sys/socket.h>
#include <sys/select.h> // for select() and fd_set
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h> // for bool types
#include <sys/wait.h> // for wait()
#include <unistd.h> // for close() and gethostname()
//#include <event.h> //for libevent
#include <fcntl.h> // to set non-blocking

#define MAX 256
#define MAX_CONNECTIONS 5
#define PORT 1234

#ifndef ERESTART
#define ERESTART EINTR
#endif

int 
main(int argc, char ** argv) {
	int fd;
	int rqst[5]; // This represents the socket accepting the request
	int fd_index = 0;
	int num_connections = 0;
	char hostname[128]; // Stores the hostname
	struct hostent *hp; // host information of server
	struct in_addr ip4addr; // server address - to be used by gethostbyaddr
	struct sockaddr_in myaddr; // holds information about this server 
	struct sockaddr_in client_addr[MAX_CONNECTIONS]; // client address's
	//struct event accept_event;
	socklen_t alen; // length of client address struct
	int sockoptval = 1;
	int pid;
	
	// For select
	fd_set readset;
	fd_set writeset;
	int maxfd = -1;
	int read_ret;
	int write_ret;
	
	/* Set client fd's to -1 */
	for(int i = 0; i < MAX_CONNECTIONS; ++i) {
		rqst[i] = -1;
	}
	
	// Step 1: Create a socket
	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("cannot create socket");
		close(fd);
		return 0;
	}
	
	// set listening socket to be non-blocking
	fcntl(fd, F_SETFL, O_NONBLOCK);

	// allow immediate reuse of PORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

	// Step 2: Identify (name) a socket
	memset((char *) &myaddr, 0, sizeof(myaddr)); // alloc space
	
	//inet_pton(AF_INET, ADDRESS, &ip4addr); // Here we are going from a "printable" address to its corresponding network format
	//hp = gethostbyaddr(&ip4addr, sizeof(ip4addr), AF_INET); // lookup hostname of the server given the server address
	
	gethostname(hostname, sizeof(hostname));
	hp = gethostbyname(hostname);
	if (!hp) {
		printf ("gethostbyname: failed to get host name");
		close(fd);
		return 0;
	}
	
	// Fill in fields for struct sockaddr_in myaddr
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(PORT);
	memcpy ((void *)&myaddr.sin_addr, hp->h_addr_list[0], hp->h_length); // now we can put the host's address into the servaddr struct
	
	// now bind socket fd to myaddr
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		close(fd);
		return 0;
	}

	/* Step 3. Listen for connections to the server */
	// allow up to MAX_CONNECTIONS
	if (listen(fd, MAX_CONNECTIONS) < 0) {
		perror("listen failed");
		exit(1);
	}
	
	// Print server information
	printf("server started on %s, listening on port %d. ip=%s \n", hostname, myaddr.sin_port, inet_ntoa(myaddr.sin_addr));
	
	/* set socket to non-blocking 
	if ((flags = fcntl (fd, F_GETFL)) < 0) {
		printf("failed to set server to non-blocking\n");
		return -1;
	}
	else {
		flags |= O_NONBLOCK;
		if (fcntl(fd, F_SETFL, flags) < 0) {
			printf("failed to set server to non-blocking\n");
			return -1;
		}
	} */

	alen = sizeof(client_addr[0]);

	/* Step 4: Accept connection requests */
	bool endServer = false;
	
	/* TODO: Use select to avoid fgets and read from blocking */
	for (;;) {
		/*while ((rqst[fd_index] = accept(fd, (struct sockaddr *)&client_addr, &alen)) < 0) {
			// if syscall 'accept' is interrupted then we break out of the while loop
			// and simply reenter and try again
			if ((errno != ECHILD) && (errno != ERESTART) && (errno != EINTR)) {
				//perror("accept failed");
				//exit(1);
			}
		}*/
		if((rqst[fd_index] = accept(fd, (struct sockaddr *)&client_addr[fd_index], &alen)) > 0) {
			fcntl(rqst[fd_index], F_SETFL, O_NONBLOCK);
		
			// the socket for this accepted connection is rqst
			printf("New User[ip=%s, port=%d] has joined the chat!\n", inet_ntoa(client_addr[fd_index].sin_addr),
			ntohs(client_addr[fd_index].sin_port));
			
			++fd_index;
			++num_connections;
		}
		if(num_connections > 0) {
			/* Reading client data */
			char buffer[MAX]; // This will be used to store client data
			char * q = 'q';
		
		#if 0 
		pid = fork();
		if(pid > 0) {
			close(rqst);
			continue;
		} 
		#endif
		
		//fcntl(rqst, F_SETFF, FNDELAY);
		
		//while(1) {
			//pid = fork();
			//if (pid == 0) {
				// let child handle reading
			for(int i = 0; i < MAX_CONNECTIONS; i++) {
					if(rqst[i] < 0)
						continue;
					
					bzero(buffer, MAX);
					read(rqst[i], &buffer, MAX);
					if (strlen(buffer) > 0) {
						if(buffer[0] == 'q') {
							close(rqst[i]);
							--num_connections;
							printf("User[ip=%s, port=%d] has left the chat\n", inet_ntoa(client_addr[i].sin_addr),
			ntohs(client_addr[i].sin_port));
						}
						else {
							printf("User[ip=%s, port=%d] says:> %s", inet_ntoa(client_addr[i].sin_addr), ntohs(client_addr[i].sin_port), buffer);
						}
					}
					
			}
				
				//return 0;
			//}
				
			/* Now the server-side user can write */
			bzero(buffer, MAX);
			
			//printf ("Type to chat:> ");
			
			//fgets(buffer, MAX, stdin);
			
			if(strlen(buffer) > 0) {
				for(int i = 0; i < MAX_CONNECTIONS; i++) {
					if(rqst[i] < 0)
						continue;
					write (rqst[i], &buffer, sizeof(buffer));
					
					if(buffer[0] == 'q') {
						endServer = true;
					}
				}
			}
			//}		
			if	(endServer) {
				for(int i = 0; i < MAX_CONNECTIONS; i++) {
					if(rqst[i] >= 0) {
						close(rqst[i]);
					}
				}
				break;
			}
			//return 0;
		}
	}
	
	close(fd);
	
	return 0;
}
