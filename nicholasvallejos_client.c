#include <netinet/in.h>	// for printing an internet address - useful for debugging
#include <arpa/inet.h> 	// for inet_pton
#include <netdb.h> 		// For gethostbyaddr() function
#include <sys/socket.h> // needed for socket programming
#include <sys/types.h> 	// needed for socket programming
#include <stdio.h>
#include <string.h>		// for string related functions like bzero
#include <unistd.h>		// for close(), read(), and write()
#include <fcntl.h> 		// for fcntl() - to set fd's as non-blocking

#define MAX 256
#define PORT 1234

int 
main(int argc, char ** argv) {
	if(argc < 2) {
		printf("Please enter the server ipv4 address.\n");
		return -1;
	}
	
	char * address; // hold first command line arg - the servers ip address
	int fd; // this will hold our socket file descriptor
	int port = PORT; // port between client and server will be the same in program
	struct sockaddr_in servaddr; // holds server information
	struct hostent *hp; // host information of server
	struct in_addr ip4addr; // server address - to be used by gethostbyaddr
	char buffer[MAX]; // Used for terminal input for IRC Chat
	
	address = argv[1];
	
	// Step 1: Create the socket
	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("cannot create socket");
		close(fd);
		return 0;
	}

	/* Step 2. Connect to the server from a client */
	inet_pton(AF_INET, address, &ip4addr); // Here we are going from a "printable" address to its corresponding network format

	/* we now have to setup the sockaddr_in struct */
	memset ((char *)&servaddr, 0, sizeof(servaddr));
	
	servaddr.sin_family = AF_INET; 	// This means ipv4 address
	servaddr.sin_port = htons(port);	// converting port to a short/network byte order
	
	// lookup hostname of the server given the server address
	hp = gethostbyaddr(&ip4addr, sizeof(ip4addr), AF_INET);
	if (!hp) {
		printf ("gethostbyaddr: failed to get host name given address");
		close(fd);
		return 0;
	}
	
	printf("Server Host Name: %s\n", hp->h_name);
	
	// now we can put the host's address into the servaddr struct
	memcpy ((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);

	// Step 3. Finally we can establish a connection to the server
	if (connect (fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror ("connect failed");
		close(fd);
		return 0;
	}

	printf ("Connected socket = %d", fd);
	
	/* Step 4: Start IRC Chat */
	printf ("\nType 'q' to exit chat room\n");

	fcntl(fd, F_SETFL, O_NONBLOCK); // set the socket file descriptor to non blocking
	
	while(1) {
		// Write messages to the server
		bzero (buffer, MAX); // clear the buffer
		fcntl(0, F_SETFL, O_NONBLOCK); // stdin_fileno is just 0 so we set it to non-blocking
		read(0, &buffer, MAX); // read from stdin (if any data is inputted)
		
		if(strlen(buffer) > 0) {
			write (fd, &buffer, sizeof(buffer)); // transmit message to server
			if	(buffer[0] == 'q')
				break;
		}
		
		// Read messages from server
		bzero (buffer, MAX); // clear the buffer
		read (fd, &buffer, sizeof(buffer)); // Now we read message from the server
		
		if(strlen(buffer) > 0) {
			if	(buffer[0] == 'q')
				break;
			printf ("> %s", buffer); // print server message to console
		}
	}
	
	close(0);  // Close stdin
	close(fd); // Close the socket
	
	return 0;
}
