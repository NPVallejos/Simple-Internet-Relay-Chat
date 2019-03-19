all: server.o client.o

server.o:	server.c
	gcc nvallej1_server.o -o server.exe

client.o:	client.c
	gcc nvallej1_client.o -o client.exe

server.c:	nvallej1_server.c
	gcc -Wall -Wextra -Wpedantic -c nvallej1_server.c

client.c:	nvallej1_client.o
	gcc -Wall -Wextra -Wpedantic -c nvallej1_client.c
	
clean:
	rm -f nvallej1_server.o nvallej1_client.o server.exe client.exe
