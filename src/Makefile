all: server client

server: server.c account_manager.c error.c
	gcc -o server server.c account_manager.c error.c

client: client.c error.c util.c
	gcc -o client client.c error.c util.c