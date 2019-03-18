#include <netinet/in.h>
#include <arpa/inet.h> // for inet_pton
#include <netdb.h> // For gethostbyaddr() function
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX 256
#define PORT 1234

int 
main(int argc, char ** argv) {
	if(argc < 0) {
		printf("Please enter the server ip address.\n");
	}
	
	char * address;
	int fd; // this will hold our socket file descriptor
	int port = PORT; // port between clietn and server will be the same
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
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	
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

	while(1) {
		// Write messages to the server
		bzero (buffer, MAX); // clear the buffer
		
		printf ("Type to chat:> ");
		fgets (buffer, MAX, stdin); // get user input
		
		write (fd, &buffer, sizeof(buffer)); // transmit to server
		
		if	(buffer[0] == 'q')
			return -1;
		
		bzero (buffer, MAX); // clear the buffer
		read (fd, &buffer, sizeof(buffer));
		
		printf ("> %s", buffer);
		
		if	(buffer[0] == 'q')
			return -1;
	}
	
	close(fd); // Close the socket
	
	return 0;
}
