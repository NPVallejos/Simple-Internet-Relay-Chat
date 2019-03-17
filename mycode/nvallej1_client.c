#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> // For gethostbyaddr() function
#include <arpa/inet.h> // for inet_pton
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX 256

int 
main(int argc, char ** argv) {
	// Step 1: Create a socket
	int fd;
	
	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror ("cannot create socket");
		close(fd);
		return 0;
	}

	// Step 2: Identify (name) a socket
	struct sockaddr_in myaddr;

	memset ((char *) &myaddr, 0, sizeof (myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	myaddr.sin_port = htons(0);

	/*if (bind (fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		close(fd);
		return 0;
	}*/

	/* Step 3a. Connect to a server from a client */
	
	int port = 1235;

	/* Connection from host to server setup */
	struct hostent *hp; // host information
	struct sockaddr_in servaddr; // server address
	struct in_addr ip4addr; // server address
	
	inet_pton(AF_INET, "128.226.114.206", &ip4addr);
	
	char host[128];
	
	if (gethostname(host, sizeof(host)) == -1 ) {
		perror ("gethostname");
		return -1;
	} 

	// we now have to setup the sockaddr_in struct like we did with bind
	memset ((char *)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	// lookup address of the server given the name
	// hp = gethostbyname(host); 
	hp = gethostbyaddr(&ip4addr, sizeof(ip4addr), AF_INET);
	//printf("Host name: %s\n", hp->h_name);
	if (!hp) {
		fprintf (stderr, "could not obtain adddress of %s\n", host);
		close(fd);
		return 0;
	}
	
	// Figure out how to do this!
	// this is the server IP: 128.226.114.206
	// You need to put this address in servaddr.sin_addr
	
	// now we can put the host's address into the servaddr struct
	memcpy ((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);

	// Finally we can establish a connection to the server
	if (connect (fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror ("connect failed");
		close(fd);
		return 0;
	}

	printf ("connected socket = %d", fd);
	
	/* Write data to server */
	char buffer[MAX];
	
	printf ("\nType 'q' to exit chat room\n");
	
	while(1) {
		bzero (buffer, MAX); // clear the buffer
		
		printf ("Type to chat:> ");
		fgets (buffer, MAX, stdin); // get user input
		
		write (fd, &buffer, sizeof(buffer)); // transmit to server
		
		if	(buffer[0] == 'q')
			break;
			
		// Read messages from server
		bzero (buffer, MAX); // clear the buffer
		read (fd, &buffer, sizeof(buffer));
		
		if	(buffer[0] == 'q')
			break;
			
		printf ("> %s", buffer);
	}
	
	close(fd); // Close the socket
	return 0;
}
