#include <sys/socket.h>	// needed for socket functions
#include <sys/types.h>	// neeeded for socket functions
#include <netinet/in.h> // for printing an internet address
#include <arpa/inet.h> 	// for printing an internet address
#include <netdb.h>		// for gethostbyname
#include <stdlib.h>
#include <stdio.h>	
#include <errno.h>	// can be used in conjunction with perror
#include <string.h> 	// for str related functions (i.e. bzero)
#include <stdbool.h> // for bool types
#include <unistd.h> // for close(), gethostname(), read(), write()
#include <fcntl.h> // to set non-blocking

#define MAX 256 
#define MAX_CONNECTIONS 15
#define PORT 1234

int 
main(int argc, char ** argv) {
	int fd;
	int rqst[MAX_CONNECTIONS]; // This represents the socket accepting the request
	int fd_index = 0;
	int num_connections = 0;
	char hostname[128]; // Stores the hostname
	struct hostent *hp; // host information of server
	struct sockaddr_in myaddr; // holds information about this server 
	struct sockaddr_in client_addr[MAX_CONNECTIONS]; // client address's
	socklen_t alen; // length of client address struct
	int sockoptval = 1;
	int pid;
	
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
	fcntl (fd, F_SETFL, O_NONBLOCK);

	// allow immediate reuse of PORT
	setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

	/* Step 2: Identify (name) a socket */
	memset ((char *) &myaddr, 0, sizeof(myaddr));
	
	gethostname (hostname, sizeof(hostname));
	hp = gethostbyname (hostname);
	if (!hp) {
		perror ("gethostbyname: failed to get host name");
		close(fd);
		return 0;
	}
	
	// Fill in fields for struct sockaddr_in myaddr
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons (PORT);
	memcpy ((void *)&myaddr.sin_addr, hp->h_addr_list[0], hp->h_length); // now we can put the host's address into the servaddr struct
	
	// now bind socket fd to myaddr
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror ("bind failed");
		close(fd);
		return 0;
	}

	/* Step 3. Listen for connections to the server */
	// allow up to MAX_CONNECTIONS
	if (listen(fd, MAX_CONNECTIONS) < 0) {
		perror ("listen failed");
		close(fd);
		return 0;
	}
	
	// Print server information
	printf ("server started on %s, listening on port %d (ip=%s)\n[%d] client(s) allowed in the chat\nType 'q' to end server\n-----------------------------------\n", hostname, myaddr.sin_port, inet_ntoa(myaddr.sin_addr), MAX_CONNECTIONS);

	alen = sizeof (client_addr[0]);

	/* Step 4: Accept connection requests */
	bool endServer = false;
	int stack[MAX_CONNECTIONS]; // holds all of the indexes that are "available" in rqst array
	int read_index = 0;	// This represents the position of the 'top' of the stack 
								// This means my stack grows from low index to high index such that low index is top of stack and high index is bottom of stack
	
	// Initially, stack holds indexes 0 to (MAX_CONNECTIONS - 1)
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		stack[i] = i;
	}

	for (;;) {
		// new accepted clients overwrite value at rqst[stack[read_index]] and at client_addr[stack[read_index]]
		if (num_connections < MAX_CONNECTIONS && (rqst[stack[read_index]] = accept (fd, (struct sockaddr *)&client_addr[stack[read_index]], &alen)) > 0) {
			// set each new connection's fd to be nonblocking
			fcntl(rqst[stack[read_index]], F_SETFL, O_NONBLOCK);
		
			// the socket for this accepted connection is stored in rqst[i]
			printf ("New User[ip=%s, port=%d] has joined the chat!\n", inet_ntoa (client_addr[stack[read_index]].sin_addr),
						ntohs (client_addr[stack[read_index]].sin_port));
			
			++num_connections;
			++read_index; // this is essentially 'popping' off the top of the stack (we dont have to change the actual value of stack[read_index], just increment read_index)
		}
		
		if (num_connections > 0) {
			/* Reading client data */
			char buffer[MAX]; // This will be used to store client data
		
			for (int i = 0; i < MAX_CONNECTIONS; i++) {
					if (rqst[i] < 0) 
						continue; // skip if no client stored at rqst[i]
					
					bzero (buffer, MAX); // clear buffer
					read (rqst[i], &buffer, MAX); // read data (if any) into buffer
					
					if (strlen(buffer) > 0) {
						if(buffer[0] == 'q') {
							// client at rqst[i] has left
							close(rqst[i]);
							--num_connections;
							
							printf ("User[ip=%s, port=%d] has left the chat\n", inet_ntoa (client_addr[i].sin_addr),
								ntohs (client_addr[i].sin_port));
							
							rqst[i] = -1; 	// indicate that no client is stored at this position in rqst array
							stack[--read_index] = i; // This is essentially "pushing" the newly "freed" index ('i') in rqst to the top of the stack 
							continue;
						}
						else {
							// print client data to server console and to all clients
							printf ("User[ip=%s, port=%d] says:> %s", inet_ntoa (client_addr[i].sin_addr), ntohs (client_addr[i].sin_port), buffer);
						
							for (int j = 0; j < MAX_CONNECTIONS; j++) {
								if (rqst[j] < 0 || j==i)
									continue;
									
								write (rqst[j], buffer, sizeof(buffer));
							}
						}
					}
					
			}
				
			/* Now the server-side user can write */
			bzero (buffer, MAX);
			fcntl (0, F_SETFL, O_NONBLOCK); // stdin_fileno is just 0 so we set it to non-blocking
			read (0, &buffer, MAX);	// reading from stdin
			
			if (strlen (buffer) > 0) {
				for (int i = 0; i < MAX_CONNECTIONS; i++) {
					if (rqst[i] < 0)
						continue; // skip if no client at rqst[i]
						
					write (rqst[i], &buffer, sizeof (buffer)); // writing information to the client
					
					if (buffer[0] == 'q') {
						endServer = true;
					}
				}
			}
					
			if	(endServer) {
				for (int i = 0; i < MAX_CONNECTIONS; i++) {
					if(rqst[i] >= 0) {
						close(rqst[i]); // we have to close all file descriptors that have been opened 
					}
				}
				
				break;
			}
		}
		else {
			// Allow server to end chatroom even when no client is connected
			char buffer[MAX];
			
			bzero (buffer, MAX);
			
			fcntl (0, F_SETFL, O_NONBLOCK); // stdin_fileno is just 0 so we set it to non-blocking
			
			read (0, &buffer, MAX);
			
			if (strlen (buffer) > 0 && buffer[0] == 'q') {
				break;
			}
		}
	}
	
	close(0); // close stdin to reset it so that it isn't nonblocking outside of program
	close(fd); // close the listening socket
	
	return 0;
}
