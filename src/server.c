#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

Active_user user[MAX_USER];
Group group[MAX_GROUP];
Account *acc_list;

Public_key_users pub[512];
int pubkey_count = 0;

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
        for (int j = 0; j < MAX_USER; j++)
        {
            group[i].group_member[j].socket = -1;
        }
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
                sv_update_port_group(&user[i], group);
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
            printf("\n--------------\n%d chooses %d \n", conn_socket, pkg.ctrl_signal);
        switch (pkg.ctrl_signal)
        {
        case PRIVATE_CHAT:
            sv_private_chat(conn_socket, &pkg);
            break;
        case SEND_PUBLIC_KEY:
            save_public_key(pkg.sender, pkg.msg);
            break;

        case SEND_PUBLIC_KEY_REQ:
            send_public_key(conn_socket, pkg.receiver);
            break;

        case CHAT_ALL:
            sv_chat_all(conn_socket, &pkg);
            break;

        case SHOW_USER:
            sv_active_user(conn_socket, &pkg);
            break;

        case LOG_OUT:
            login = 0;
            sv_logout(conn_socket, &pkg);
            break;
        case GROUP_CHAT_INIT:
            sv_group_chat_init(conn_socket, &pkg);
            break;
        case SHOW_GROUP:
            sv_show_group(conn_socket, &pkg);
            break;
        case NEW_GROUP:
            sv_new_group(conn_socket, &pkg);
            break;
        case JOIN_GROUP:
            sv_join_group(conn_socket, &pkg);
            break;
        case HANDEL_GROUP_MESS:
            // hien ra thong tin phong
            break;
        case INVITE_FRIEND:
            sv_invite_friend(conn_socket, &pkg);
            break;
        case GROUP_CHAT:
            sv_group_chat(conn_socket, &pkg);
            break;
        case GROUP_INFO:
            sv_show_group_info(conn_socket, &pkg);
            break;
        case LEAVE_GROUP:
            sv_leave_group(conn_socket, &pkg);
            break;
        default:
            break;
        }
        printf("Done %d of %d\n", pkg.ctrl_signal, conn_socket);
    }

    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        if (user[i].socket == conn_socket)
        {
            Account *target_acc = find_account(acc_list, user[i].username);
            target_acc->is_signed_in = 0;
            user[i].socket = -1;
            for (int j = 0; j < MAX_GROUP; j++)
            {
                if (user[i].group_id[j] >= 0)
                {
                    int group_id = user[i].group_id[j];
                    int user_id_group = sv_search_id_user_group(group[group_id], user[i].username);
                    if (user_id_group >= 0)
                    {
                        // printf("1\n");
                        // printf("%d %d\n", group_id, user_id_group);
                        group[group_id].group_member[user_id_group].socket = 0; // can cap nhat khi dang nhap lai
                    }
                    user[i].group_id[j] = -1;
                }
            }
            break;
        }
    }
    for(int j = 0; j < pubkey_count; j++) {
        if(strcmp(user[i].username, pub[j].username) == 0) {
            pub[j].public_key->exponent = 0;
            pub[j].public_key->modulus = 0;
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


int check_public_key(Public_key_users* user_pub, char* username) {
    int check = 0;
    // printf("has %d public key(s)\n", pubkey_count);
    for(int i = 0; i < pubkey_count; i++) {
        
        if(strcmp(pub[i].username, username) == 0) {
            if(pub[i].public_key->exponent == 0) return 0;
            user_pub->public_key->exponent = pub[i].public_key->exponent;
            user_pub->public_key->modulus = pub[i].public_key->modulus;
            strcpy(user_pub->username, username);
            check = 1;
            break;
        }
    }
    return check;
}

void send_public_key(int client_socket, char* receiver) {
    Package pkg;
    strcpy(pkg.receiver, receiver);
    Public_key_users user_key[1];
    int check = check_public_key(user_key, receiver);
    if(check == 0) {
        printf("No public key of %s\n", receiver);
        pkg.ctrl_signal = ERR_INVALID_RECEIVER;
        send(client_socket, &pkg, sizeof(pkg), 0);
        return;
    }
    // strcpy(pkg.sender, my_username);

    pkg.ctrl_signal = SEND_PUBLIC_KEY;
    memcpy(pkg.msg, &user_key->public_key->modulus, sizeof(user_key->public_key->modulus));
    memcpy(pkg.msg + sizeof user_key->public_key->modulus, &user_key->public_key->exponent, sizeof(user_key->public_key->exponent));
    printf("Public Key of %s:\n Modulus: %lld\n Exponent: %lld\n", receiver, (long long)user_key->public_key->modulus, (long long)user_key->public_key->exponent);

    send(client_socket, &pkg, sizeof(pkg), 0);
    printf("Public key sent!\n\n");
}

void save_public_key(char* sender, char* msg) {
    int check = 0;
    int i;
    for(i = 0; i < pubkey_count; i++) {
        if(strcmp(pub[i].username, sender) == 0) {
            pub[i].public_key->exponent = ((struct public_key_class*)msg)->exponent;
            pub[i].public_key->modulus = ((struct public_key_class*)msg)->modulus;
            
            check = 1;
            break;
        }
    }
    if(check != 1) {
        i = pubkey_count;
        // pub[i] = (Public_key_users)malloc(sizeof(Public_key_users));
        pub[i].public_key->exponent = ((struct public_key_class*)msg)->exponent;
        pub[i].public_key->modulus = ((struct public_key_class*)msg)->modulus;
        strcpy(pub[i].username, sender);
        pubkey_count++;
    }
    printf("Public Key of %s:\n Modulus: %lld\n Exponent: %lld\n\n", sender, (long long)pub[i].public_key->modulus, (long long)pub[i].public_key->exponent);

}

void sv_private_chat(int conn_socket, Package *pkg)
{
    int i = 0;
    int recv_socket;
    for (i = 0; i < MAX_USER; i++)
    {
        if (user[i].socket > 0)
            if(strcmp(pkg->receiver, user[i].username) == 0)
            {
                recv_socket = user[i].socket;
                break;
            }
    }

    if (i == MAX_USER){
        pkg->ctrl_signal = ERR_INVALID_RECEIVER;
        send(conn_socket, pkg, sizeof(*pkg), 0);
        printf("sent err\n");
        return;
    }
    
    pkg->ctrl_signal = MSG_SENT_SUCC;

    send(conn_socket, pkg, sizeof(*pkg), 0);
    printf("sent nor\n");

    if(strcmp(pkg->msg, TESTING_MSG) == 0) {
        send_public_key(recv_socket, pkg->sender);
        send_public_key(conn_socket, pkg->receiver);
        return;
    }
    // if(pkg->ctrl_signal == PRIVATE_CHAT)
    pkg->ctrl_signal = PRIVATE_CHAT;
    printf("%d: %s to %s: %s\n", pkg->ctrl_signal, pkg->sender, pkg->receiver, pkg->msg);


    send(recv_socket, pkg, sizeof(*pkg), 0);
    printf("Sent %d to %s\n", pkg->ctrl_signal, pkg->receiver);
    
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
int sv_search_id_user(Active_user user[], char *user_name)
{
    int user_id = -1;
    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        if (strcmp(user[i].username, user_name) == 0 && user[i].socket >= 0)
        {
            user_id = i;
            return user_id;
        }
    }
    return -1;
}
int sv_search_id_user_group(Group group, char *user_name)
{
    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        if (strcmp(group.group_member[i].username, user_name) == 0)
        {
           // printf("%d %s\n", i, user_name);
            return i;
        }
    }
    return -1;
}

void sv_group_chat_init(int conn_socket, Package *pkg)
{
    strcpy(pkg->msg, "CHUC NANG CHAT NHOM\n");
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
    send(conn_socket, pkg, sizeof(*pkg), 0);
}
// new group

int check_user_in_group(Active_user user, int group_id)
{
    int i = 0;
    for (i = 0; i < MAX_GROUP; i++)
    {
        if (user.group_id[i] == group_id)
            return 1;
    }
    return 0;
}
int sv_add_group_user(Active_user *user, int group_id)
{
    for (int i = 0; i < MAX_GROUP; i++)
    {
        if (user->group_id[i] < 0)
        {
            user->group_id[i] = group_id;
            return 1;
        }
    }
    return 0;
}

int sv_add_user(Active_user user, Group *group)
{
    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        if (group->group_member[i].socket < 0)
        {
            group->group_member[i].socket = user.socket;
            strcpy(group->group_member[i].username, user.username);
            group->curr_num++;
            return i;
        }
    }
    return 0;
}

void print_members(Group group)
{
    printf("MEMBERS OF GROUP %s: \n", group.group_name);
    for (int i = 0; i < MAX_USER; i++)
    {
        if (group.group_member[i].socket > 0)
        {
            printf("%s\n", group.group_member[i].username);
        }
    }
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
            sv_add_user(user[user_id], &group[i]);
            sprintf(group[i].group_name, "Group_%d", group_id);
            break;
        }
    }
    drop_table(group_id);
    strcpy(pkg->msg, group[group_id].group_name);
    pkg->ctrl_signal = MSG_MAKE_GROUP_SUCC;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

// join group
int sv_search_id_group(Group group[], Active_user user, char *group_name)
{
    int i;
    int group_id = -1;
    for (i = 0; i < MAX_GROUP; i++)
    {
        if (user.group_id[i] >= 0)
        {
            group_id = user.group_id[i];
            if (strcmp(group[group_id].group_name, group_name) == 0)
            {
                // printf("%s\n",group[i].group_name);
                return group_id;
            }
        }
    }
    return -1;
}

void sv_join_group(int conn_socket, Package *pkg)
{
    char group_name[GROUP_NAME_SIZE];
    int group_id = -1;
    int user_id = -1;

    user_id = search_user(conn_socket);
    strcpy(group_name, pkg->msg);
    group_id = sv_search_id_group(group, user[user_id], group_name);
    if (group_id >= 0)
    {
        printf("%s JOIN GROUP %s\n", pkg->sender, group[group_id].group_name);
        strcpy(pkg->msg, group_name);
        pkg->ctrl_signal = JOIN_GROUP_SUCC;
        pkg->group_id = group_id;
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
    else
    {
        pkg->ctrl_signal = ERR_GROUP_NOT_FOUND;
        send(conn_socket, pkg, sizeof(*pkg), 0);
    }
}

void sv_invite_friend(int conn_socket, Package *pkg)
{
    char friend_name[USERNAME_SIZE];
    int user_id = search_user(conn_socket);
    int friend_id;
    int group_id;

    group_id = pkg->group_id;
    strcpy(friend_name, pkg->receiver);
    friend_id = sv_search_id_user(user, friend_name);
    if (friend_id >= 0)
    {
        if (friend_id == user_id)
        {
            pkg->ctrl_signal = ERR_IVITE_MYSELF;
            send(conn_socket, pkg, sizeof(*pkg), 0);
            return;
        }
        else if (group[group_id].curr_num > MAX_USER - 1)
        {
            pkg->ctrl_signal = ERR_FULL_MEM;
            send(conn_socket, pkg, sizeof(*pkg), 0);
            return;
        }
        else if (check_user_in_group(user[friend_id], group_id))
        {
            pkg->ctrl_signal = ERR_IS_MEM;
            send(conn_socket, pkg, sizeof(*pkg), 0);
            return;
        }
        else // thanh cong
        {
            strcpy(pkg->msg, group[group_id].group_name);

            send(user[friend_id].socket, pkg, sizeof(*pkg), 0);
            printf("%s add %s to %s\n", user[user_id].username,
                   user[friend_id].username, group[group_id].group_name);
            sv_add_group_user(&user[friend_id], group_id);
            sv_add_user(user[friend_id], &group[group_id]);

            pkg->ctrl_signal = INVITE_FRIEND_SUCC;
            send(conn_socket, pkg, sizeof(*pkg), 0);

            // gui thong bao den cho moi nguoi
            memset(pkg->sender, '\0', sizeof(pkg->sender));
            strcpy(pkg->sender, SERVER_SYSTEM_USERNAME);
            memset(pkg->msg, '\0', sizeof(pkg->msg));
            sprintf(pkg->msg, "\"%s\" đã thêm \"%s\" vào nhóm.", user[user_id].username, user[friend_id].username);
            pkg->ctrl_signal = GROUP_CHAT;
            sv_group_chat(conn_socket, pkg);
        }
    }

    else
    {
        pkg->ctrl_signal = ERR_USER_NOT_FOUND;
        send(conn_socket, pkg, sizeof(*pkg), 0);
        return;
    }
}

// chat trong nhom
void sv_group_chat(int conn_socket, Package *pkg)
{
    int group_id = pkg->group_id;
    
    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        if (group[group_id].group_member[i].socket > 0)// && group[group_id].group_member[i].socket != conn_socket)
        {
            send(group[group_id].group_member[i].socket, pkg, sizeof(*pkg), 0);
        }
    }
    
    pkg->ctrl_signal = MSG_SENT_SUCC;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

// group info
void sv_show_group_info(int conn_socket, Package *pkg)
{
    // giao dien (bat dau gui thong tin)
    pkg->ctrl_signal = SHOW_GROUP_INFO_START;
    send(conn_socket, pkg, sizeof(*pkg), 0);

    int group_id = pkg->group_id;
    printf("Group name: %s\n", group[group_id].group_name);
    // gui group name
    strcpy(pkg->msg, group[group_id].group_name);
    pkg->ctrl_signal = SHOW_GROUP_NAME;
    send(conn_socket, pkg, sizeof(*pkg), 0);

    // gui ten thanh vien
    print_members(group[group_id]);

    sprintf(pkg->msg, "%d", group[group_id].curr_num);
    pkg->ctrl_signal = SHOW_GROUP_MEM_NUMBER;
    send(conn_socket, pkg, sizeof(*pkg), 0);

    // strcpy(pkg->msg, "MEMBERS OF GROUP:");
    // pkg->ctrl_signal = SHOW_GROUP_MEM_USERNAME;
    // send(conn_socket, pkg, sizeof(*pkg), 0);
    for (int i = 0; i < MAX_USER; i++)
    {
        if (group[group_id].group_member[i].socket >= 0)
        {
            strcpy(pkg->msg, group[group_id].group_member[i].username);
            pkg->ctrl_signal = SHOW_GROUP_MEM_USERNAME;
            send(conn_socket, pkg, sizeof(*pkg), 0);
        }
    }

    // giao dien (gui thong tin xong)
    pkg->ctrl_signal = SHOW_GROUP_INFO_END;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

// thoat nhom
void sv_leave_group(int conn_socket, Package *pkg)
{
    int group_id = pkg->group_id;
    int user_id = search_user(conn_socket);
    int i = 0;
    for (i = 0; i < MAX_USER; i++)
    {
        Member mem = group[group_id].group_member[i];
        if (strcmp(mem.username, user[user_id].username) == 0)
        {
            group[group_id].group_member[i].socket = -1;

            strcpy(group[group_id].group_member[i].username, NULL_STRING);

            group[group_id].curr_num--;
            // if(group[group_id].curr_num == 0)
            // {
            //     drop_table(group_id);
            // }
            if (sv_leave_group_user(&user[user_id], group_id))
            {
                // gui lai cho user
                strcpy(pkg->msg, "LEAVE GROUP SUCCESS: ");
                strcat(pkg->msg, group[group_id].group_name);
                pkg->ctrl_signal = LEAVE_GROUP_SUCC;
                send(conn_socket, pkg, sizeof(*pkg), 0);

                // gui thong bao den cho moi nguoi
                memset(pkg->sender, '\0', sizeof(pkg->sender));
                strcpy(pkg->sender, SERVER_SYSTEM_USERNAME);
                memset(pkg->msg, '\0', sizeof(pkg->msg));
                sprintf(pkg->msg, "Người dùng \"%s\" đã rời nhóm và không còn là thành viên của nhóm.", user[user_id].username);
                pkg->ctrl_signal = GROUP_CHAT;
                sv_group_chat(conn_socket, pkg);
            }
            break;
        }
    }
}

int sv_leave_group_user(Active_user *user, int group_id)
{
    for (int i = 0; i < MAX_GROUP; i++)
    {
        if (user->group_id[i] == group_id)
        {
            user->group_id[i] = -1;
            return 1;
        }
    }
    return 0;
}

void sv_update_port_group(Active_user *user, Group *group)
{
    int i = 0;
    int user_id_port;
    for (i = 0; i < MAX_GROUP; i++)
    {
        user_id_port = sv_search_id_user_group(group[i], user->username);
        if (user_id_port >= 0)
        {
            sv_add_group_user(user, i);
            group[i].group_member[user_id_port].socket = user->socket;
        }
    }
}


void sv_logout(int conn_socket, Package *pkg)
{
    printf("%d logout\n", conn_socket);
    pkg->ctrl_signal = LOG_OUT;
    send(conn_socket, pkg, sizeof(*pkg), 0);
}

// main
int main()
{
    make_server();
    return 0;
}