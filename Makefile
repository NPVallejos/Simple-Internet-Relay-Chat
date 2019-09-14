all: server.o client.o

server.o:	server.c
	gcc nicholasvallejos_server.o -o server

client.o:	client.c
	gcc nicholasvallejos_client.o -o client

server.c:	nicholasvallejos_server.c
	gcc -Wall -Wextra -Wpedantic -c nicholasvallejos_server.c

client.c:	nicholasvallejos_client.o
	gcc -Wall -Wextra -Wpedantic -c nicholasvallejos_client.c
	
clean:
	rm -f *.o server client
