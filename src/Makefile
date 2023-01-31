all: server client_gui

server: server.c account_manager.c error.c
	gcc -o server server.c account_manager.c error.c -lpthread

client_gui: client_gui.c client.c error.c util.c rsa.c
	gcc -rdynamic `pkg-config --cflags gtk+-3.0` -o client_gui client_gui.c rsa.c client.c error.c util.c -lpthread `pkg-config --libs gtk+-3.0`