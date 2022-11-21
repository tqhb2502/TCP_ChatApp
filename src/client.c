#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int connect_to_server() {

    int client_socket;
    struct sockaddr_in server_addr;

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        report_err(ERR_SOCKET_INIT);
        exit(0);
    }

    if (connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        report_err(ERR_CONNECT_TO_SERVER);
        exit(0);
    }

    return client_socket;
}

void login_menu() {

}

void user_menu() {

}

void ask_server(int client_socket) {

    int choice, result;
    Package pkg;

    while (1) {

        login_menu();
        printf("Your choice: ");
        scanf("%d", &choice);
        clear_stdin_buff();

        switch (choice) {
        case 1:
            pkg.ctrl_signal = LOGIN_REQ;
            send(client_socket, &pkg, sizeof(pkg), 0);
            result = login(client_socket);
            if (result == LOGIN_SUCC) {
                user_use(client_socket);
            } else if (result == INCORRECT_ACC) {
                report_err(ERR_INCORRECT_ACC);
            } else {
                report_err(ERR_SIGNED_IN_ACC);
            }
            break;
        case 2:
            pkg.ctrl_signal = QUIT_REQ;
            send(client_socket, &pkg, sizeof(pkg), 0);
            close(client_socket);
            exit(0);
        }
    }
}

int login(int client_socket) {

    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    Package pkg;

    printf("Username: ");
    scanf("%s", username);
    clear_stdin_buff();
    printf("Password: ");
    scanf("%s", password);
    clear_stdin_buff();

    strcpy(pkg.msg, username);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);

    strcpy(pkg.msg, password);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);
    return pkg.ctrl_signal;
}

void user_use(int client_socket) {
    printf("Login successfully!\n");
}

int main() {
    int client_socket = connect_to_server();
    ask_server(client_socket);
    close(client_socket);
    return 0;
}