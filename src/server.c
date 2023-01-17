#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

Active_user user[MAX_USER];
Group group[MAX_GROUP];
Account *acc_list;

int create_listen_socket()
{

    int listen_socket;
    struct sockaddr_in server_addr;

    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        report_err(ERR_SOCKET_INIT);
        exit(0);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        report_err(ERR_SOCKET_INIT);
        exit(0);
    }

    if (listen(listen_socket, MAX_USER) < 0)
    {
        report_err(ERR_SOCKET_INIT);
        exit(0);
    }

    return listen_socket;
}

int accept_conn(int listen_socket)
{

    int conn_socket;
    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(struct sockaddr);

    if ((conn_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_addr_size)) < 0)
    {
        report_err(ERR_CONN_ACCEPT);
        exit(0);
    }

    return conn_socket;
}

void make_server()
{

    int listen_socket;

    acc_list = read_account_list();
    listen_socket = create_listen_socket();
    for (int i = 0; i < MAX_USER; i++)
    {
        user[i].socket = -1;
        // 17/01/2023
        for (int j = 0; j < MAX_GROUP; j++)
            user[i].group_id[j] = -1;
    }
    for (int i = 0; i < MAX_GROUP; i++)
    {
        group[i].curr_num = 0;
    }

    printf("Server created\n");

    while (1)
    {

        int conn_socket = accept_conn(listen_socket);

        pthread_t client_thr;
        if (pthread_create(&client_thr, NULL, pre_login_srv, (void *)&conn_socket) < 0)
        {
            report_err(ERR_CREATE_THREAD);
            exit(0);
        }
        pthread_detach(client_thr);
    }

    close(listen_socket);
}

void *pre_login_srv(void *param)
{

    int conn_socket = *((int *)param);
    Package pkg;

    while (1)
    {

        recv(conn_socket, &pkg, sizeof(pkg), 0);

        switch (pkg.ctrl_signal)
        {
        case LOGIN_REQ:
            handle_login(conn_socket, acc_list);
            break;
        case QUIT_REQ:
            close(conn_socket);
            printf("user quit\n");
            pthread_exit(NULL);
        }
    }
}

void handle_login(int conn_socket, Account *acc_list)
{

    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    Package pkg;
    Account *target_acc;
    int result;

    recv(conn_socket, &pkg, sizeof(pkg), 0);
    strcpy(username, pkg.msg);

    pkg.ctrl_signal = RECV_SUCC;
    send(conn_socket, &pkg, sizeof(pkg), 0);

    recv(conn_socket, &pkg, sizeof(pkg), 0);
    strcpy(password, pkg.msg);

    printf("%s\n", username);
    printf("%s\n", password);

    target_acc = find_account(acc_list, username);
    if (target_acc)
    {
        if (target_acc->is_signed_in)
        {
            result = SIGNED_IN_ACC;
        }
        else
        {
            if (strcmp(target_acc->password, password) == 0)
            {
                result = LOGIN_SUCC;
            }
            else
            {
                result = INCORRECT_ACC;
            }
        }
    }
    else
    {
        result = INCORRECT_ACC;
    }

    if (result == LOGIN_SUCC)
    {
        printf("login success\n");
        target_acc->is_signed_in = 1;

        for (int i = 0; i < MAX_USER; i++)
        {
            if (user[i].socket < 0)
            {
                strcpy(user[i].username, username);
                user[i].socket = conn_socket;
                break;
            }
        }
    }
    else if (result == SIGNED_IN_ACC)
    {
        printf("already signed in acc\n");
    }
    else
    {
        printf("incorrect acc\n");
    }

    pkg.ctrl_signal = result;
    send(conn_socket, &pkg, sizeof(pkg), 0);
    if (result == LOGIN_SUCC)
        sv_user_use(conn_socket);
}

void sv_user_use(int conn_socket)
{

    Package pkg;
    int login = 1;
    while (login)
    {
        if (recv(conn_socket, &pkg, sizeof(pkg), 0) > 0) // printf("Receive from %d\n", conn_socket);
            printf("%d chooses %d \n", conn_socket, pkg.ctrl_signal);
        switch (pkg.ctrl_signal)
        {
        case PRIVATE_CHAT:
            sv_private_chat(conn_socket, &pkg);
            break;

        case CHAT_ALL:
            sv_chat_all(conn_socket, &pkg);
            break;

        case SHOW_USER:
            sv_active_user(conn_socket, &pkg);
            break;

        case LOG_OUT:
            login = 0;
            printf("%d logout\n", conn_socket);
            break;
        case GROUP_CHAT:
            sv_group_chat(conn_socket, &pkg);
            break;
        case SHOW_GROUP:
            sv_show_group(conn_socket, &pkg);
            break;
        case NEW_GROUP:
            sv_new_group(conn_socket, &pkg);
            break;
        default:
            break;
        }
        printf("Done %d of %d\n", pkg.ctrl_signal, conn_socket);
    }

    for (int i = 0; i < MAX_USER; i++)
    {
        if (user[i].socket == conn_socket)
        {
            Account *target_acc = find_account(acc_list, user[i].username);
            target_acc->is_signed_in = 0;
            user[i].socket = -1;
            break;
        }
    }
}

void sv_active_user(int conn_socket, Package *pkg)
{

    char user_list[MSG_SIZE] = {0};
    for (int i = 0; i < MAX_USER; i++)
    {
        if (user[i].socket > 0)
        {
            strcat(user_list, user[i].username);
            int len = strlen(user_list);
            user_list[len] = ' ';
        }
    }
    strcpy(pkg->msg, user_list);
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void sv_private_chat(int conn_socket, Package *pkg)
{

    printf("%d: %s to %s: %s\n", pkg->ctrl_signal, pkg->sender, pkg->receiver, pkg->msg);

    int i = 0;

    for (i = 0; i < MAX_USER; i++)
    {
        if (strcmp(pkg->receiver, user[i].username) == 0 && user[i].socket > 0)
        {
            // recv_socket = user[i].socket;
            send(user[i].socket, pkg, sizeof(*pkg), 0);
            break;
        }
    }

    if (i == MAX_USER)
        pkg->ctrl_signal = ERR_INVALID_RECEIVER;
    else
        pkg->ctrl_signal = MSG_SENT_SUCC;

    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void sv_chat_all(int conn_socket, Package *pkg)
{
    printf("%d: %s to all: %s\n", pkg->ctrl_signal, pkg->sender, pkg->msg);

    int i = 0;

    for (i = 0; i < MAX_USER; i++)
    {
        if (user[i].socket > 0)
            send(user[i].socket, pkg, sizeof(*pkg), 0);
    }

    pkg->ctrl_signal = MSG_SENT_SUCC;

    send(conn_socket, pkg, sizeof(*pkg), 0);
}

// 17/1/2023
int search_user(int conn_socket)
{
    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        if (user[i].socket == conn_socket)
            return i;
    }
    return -1;
}

void sv_group_chat(int conn_socket, Package *pkg)
{
    strcpy(pkg->msg, "CHUC NANG TAO NHOM\n");
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

void sv_show_group(int conn_socket, Package *pkg)
{
    int user_id = search_user(conn_socket);
    char group_list[MSG_SIZE] = {0};
     int group_id;
    for (int i = 0; i < MAX_GROUP; i++)
    {
        if (user[user_id].group_id[i] >= 0)
        {
            int group_id = user[user_id].group_id[i];
            strcat(group_list, group[group_id].group_name);
            int len = strlen(group_list);
            group_list[len] = ' ';
        }
    }
    strcpy(pkg->msg, group_list);
    printf("\n\n%s\n",pkg->msg);
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

int sv_add_group_user(Active_user *user, int group_id)
{
    for(int i = 0; i < MAX_GROUP; i++){
        if(user->group_id[i] < 0 ){
            user->group_id[i] = group_id;
            return 1;
        }
    }
    return 0;
}

void sv_new_group(int conn_socket, Package *pkg)
{
    int user_id = search_user(conn_socket);
    int group_id = -1;
    for (int i = 0; i < MAX_GROUP; i++)
    {
        if (group[i].curr_num == 0)
        {
            group_id = i;
            sv_add_group_user(&user[user_id], group_id);
            group[i].curr_num++;
            sprintf(group[i].group_name,"Group %d", group_id);
            break;
        }
    }
    strcpy(pkg->msg, group[group_id].group_name);
    pkg->ctrl_signal = MSG_MAKE_GROUP_SUCC;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

// main
int main()
{
    make_server();
    return 0;
}