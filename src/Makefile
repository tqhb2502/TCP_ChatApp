all: server client

server: server.c account_manager.c error.c dbtest.c
	gcc -o server server.c account_manager.c error.c dbtest.c -lpthread -ldl -lm -lsqlite3

client: client.c error.c util.c dbtest.c
	gcc -o client client.c error.c util.c dbtest.c -lpthread -ldl -lm -lsqlite3
