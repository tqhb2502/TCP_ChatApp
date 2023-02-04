all: server client_gui

server: server.c account_manager.c error.c dbtest.c
	gcc -o server server.c account_manager.c error.c dbtest.c -lpthread -ldl -lm -lsqlite3

client_gui: client_gui.c client.c error.c util.c rsa.c dbtest.c
	gcc -rdynamic `pkg-config --cflags gtk+-3.0` -o client_gui client_gui.c rsa.c client.c error.c util.c dbtest.c -lpthread -ldl -lm -lsqlite3 `pkg-config --libs gtk+-3.0`
