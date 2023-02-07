#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

char my_username[USERNAME_SIZE];
char curr_group_name[GROUP_NAME_SIZE];
int curr_group_id = -1;
int join_succ = 0;

char *PRIME_SOURCE_FILE = "../assets/primes.txt";

struct public_key_class my_pub[1];
struct private_key_class my_priv[1];

Public_key_users user_pub[1];

int doing = 0;

int connect_to_server()
{

    int client_socket;
    struct sockaddr_in server_addr;

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        report_err(ERR_SOCKET_INIT);
        exit(0);
    }

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        report_err(ERR_CONNECT_TO_SERVER);
        exit(0);
    }

    return client_socket;
}

void login_menu()
{
    printf("------ Welcome to chat app ------\n");
    printf("1. Login\n");
    // printf("2. Sign up\n");
    printf("2. Exit\n");
    // printf("Your choice: ");
}

void user_menu()
{
    printf("\n\n****** Login success ******\n");
    // printf("1. Show current online users\n");
    printf("1. Private chat\n");
    printf("2. Chat All\n");
    printf("3. Logout\n");
    printf("4. Show online users\n");
    printf("5. Group chat\n");
    // printf("Your choice: ");
}

//  nhom chat menu 17/01/2023
void group_chat_menu()
{
    printf("\n\n****** Group chat ******\n");
    printf("1. Show my group\n");
    printf("2. Make new group\n");
    printf("3. Join group\n");
    printf("4. Return main menu\n");
}

void sub_group_chat_menu(char *group_name)
{
    printf("\n\n****** %s ******\n", group_name);
    printf("1. Invite your friends\n");
    printf("2. Chat \n");
    printf("3. Show group infomation \n");
    printf("4. Leave the group chat\n");
    printf("5. View chat history\n");
    printf("6. Return group chat menu\n");
}
// void ask_server(int client_socket)
// {

//     int choice, result;
//     Package pkg;

//     while (1)
//     {
//         sleep(1);
//         login_menu();
//         printf("Your choice: ");
//         scanf("%d", &choice);
//         clear_stdin_buff();

//         switch (choice)
//         {
//         case 1:
//             pkg.ctrl_signal = LOGIN_REQ;
//             send(client_socket, &pkg, sizeof(pkg), 0);
//             result = login(client_socket);
//             if (result == LOGIN_SUCC)
//             {
//                 user_use(client_socket);
//             }
//             else if (result == INCORRECT_ACC)
//             {
//                 report_err(ERR_INCORRECT_ACC);
//             }
//             else
//             {
//                 report_err(ERR_SIGNED_IN_ACC);
//             }
//             break;
//         case 2:
//             pkg.ctrl_signal = QUIT_REQ;
//             send(client_socket, &pkg, sizeof(pkg), 0);
//             close(client_socket);
//             exit(0);
//         }
//     }
// }

int login(int client_socket, char *username, char *password)
{

    // char username[USERNAME_SIZE];
    // char password[PASSWORD_SIZE];
    Package pkg;

    // printf("Username: ");
    // scanf("%s", username);
    // clear_stdin_buff();
    // printf("Password: ");
    // scanf("%s", password);
    // clear_stdin_buff();
    strcpy(pkg.msg, username);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);

    strcpy(pkg.msg, password);
    send(client_socket, &pkg, sizeof(pkg), 0);

    recv(client_socket, &pkg, sizeof(pkg), 0);
    // sleep(1);
    if (pkg.ctrl_signal == LOGIN_SUCC){
        strcpy(my_username, username);
        if(my_priv->exponent == 0)
            rsa_gen_keys(my_pub, my_priv, PRIME_SOURCE_FILE);
        
        printf("Private Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)my_priv->modulus, (long long)my_priv->exponent);
        printf("Public Key:\n Modulus: %lld\n Exponent: %lld\n\n", (long long)my_pub->modulus, (long long)my_pub->exponent);
        // pubkey_count = 0;
        send_my_public_key(client_socket);
    }
    return pkg.ctrl_signal;
}

void send_my_public_key(int client_socket) {
    Package pkg;
    pkg.ctrl_signal = SEND_PUBLIC_KEY;
    strcpy(pkg.sender, my_username);
    memcpy(pkg.msg, &my_pub->modulus, sizeof(my_pub->modulus));
    memcpy(pkg.msg + sizeof my_pub->modulus, &my_pub->exponent, sizeof(my_pub->exponent));
    send(client_socket, &pkg, sizeof(pkg), 0);
}

void receive_public_key(int client_socket, Package* pkg) {
    strcpy(user_pub->username, pkg->receiver);
    user_pub->public_key->exponent = ((struct public_key_class*)pkg->msg)->exponent;
    user_pub->public_key->modulus = ((struct public_key_class*)pkg->msg)->modulus;
    printf("receive pubkey of %s: %lld %lld\n", user_pub->username, (long long)user_pub->public_key->exponent, (long long)user_pub->public_key->modulus);
}

int check_public_key(int client_socket, char* username) {
    // printf("Key of %s, require %s\n", user_pub->username, username);
    if(strcmp(user_pub->username, username) == 0) return 1;
    else {
        Package pkg;
        strcpy(pkg.sender, my_username);
        strcpy(pkg.receiver, username);
        pkg.ctrl_signal = SEND_PUBLIC_KEY_REQ;
        send(client_socket, &pkg, sizeof(pkg), 0);
        return 0;
    }
}

// void user_use(int client_socket)
// {
    // printf("Login successfully!\n");
    // int login = 1;
    // int choice, result;
    // Package pkg;

    // pthread_t read_st;
    // if (pthread_create(&read_st, NULL, read_msg, (void *)&client_socket) < 0)
    // {
    //     report_err(ERR_CREATE_THREAD);
    //     exit(0);
    // }
    // pthread_detach(read_st);

    // see_active_user(client_socket);

    // while (login)
    // {

    //     user_menu();
    //     printf("Your choice: \n");
    //     scanf("%d", &choice);
    //     clear_stdin_buff();

        // switch (choice)
        // {
        // case 1:
        //     private_chat(client_socket);
        //     break;

        // case 2:
        //     chat_all(client_socket);
        //     break;

        // case 3:
        //     login = 0;
        //     pkg.ctrl_signal = LOG_OUT;
        //     // strcpy(pkg.sender, my_username);
        //     send(client_socket, &pkg, sizeof(pkg), 0);
        //     strcpy(my_username, "x");
        //     strcpy(curr_group_name, "x");
        //     curr_group_id = -1;
        //     sleep(1);
        //     break;
        // case 4:
        //     see_active_user(client_socket);
        //     break;
        // 17/01/2023
        // case 5:
        //     group_chat_init(client_socket);
        //     break;
        // default:
        //     printf("Ban nhap sai roi !\n");
        //     break;
        // }
    // }
// }

// void *read_msg(void *param)
// {
//     int *c_socket = (int *)param;
//     int client_socket = *c_socket;
//     // printf("\nmysoc: %d\n", client_socket);
//     // int client_socket = my_socket;
//     Package pkg;
//     while (1)
//     {
//         recv(client_socket, &pkg, sizeof(pkg), 0);
//         // printf("receive %d from server\n", pkg.ctrl_signal);
//         switch (pkg.ctrl_signal)
//         {
//         case SHOW_USER:
//             printf("Current online users: %s \n", pkg.msg);
//             break;

//         case PRIVATE_CHAT:
//             printf("%s: %s\n", pkg.sender, pkg.msg);
//             break;

//         case CHAT_ALL:
//             printf("%s to all: %s\n", pkg.sender, pkg.msg);
//             break;

//         case ERR_INVALID_RECEIVER:
//             report_err(ERR_INVALID_RECEIVER);
//             break;
//         case MSG_SENT_SUCC:
//             printf("Message sent!\n");
//             break;
//         case GROUP_CHAT_INIT:
//             printf("%s\n", pkg.msg);
//             break;
//         case SHOW_GROUP:
//             printf("Your group: \n%s \n", pkg.msg);
//             break;

//         case MSG_MAKE_GROUP_SUCC:
//             printf("Your new group: %s \n", pkg.msg);
//             break;
//         case JOIN_GROUP_SUCC:
//             printf("Current group: %s \n", pkg.msg);
//             strcpy(curr_group_name, pkg.msg);
//             curr_group_id = pkg.group_id;
//             join_succ = 1;
//             break;
//         case INVITE_FRIEND:
//             printf("Attention: %s \n", pkg.msg);
//             break;
//         case ERR_GROUP_NOT_FOUND:
//             report_err(ERR_GROUP_NOT_FOUND);
//             break;
//         case ERR_IVITE_MYSELF:
//             report_err(ERR_IVITE_MYSELF);
//             break;
//         case ERR_USER_NOT_FOUND:
//             report_err(ERR_USER_NOT_FOUND);
//             break;
//         case ERR_FULL_MEM:
//             report_err(ERR_FULL_MEM);
//             break;
//         case INVITE_FRIEND_SUCC:
//             printf("%s\n", pkg.msg);
//             break;
//         case GROUP_CHAT:
//             if (curr_group_id == pkg.group_id)
//             {
//                 printf("%s: %s\n", pkg.sender, pkg.msg);
//             }
//             else
//             {
//                 printf("%s sent to Group_%d: %s\n", pkg.sender, pkg.group_id, pkg.msg);
//             }
//             break;
//         case SHOW_GROUP_NAME:
//             printf("GROUP NAME: %s\n", pkg.msg);
//             break;
//         case SHOW_GROUP_MEM:
//             printf("%s\n", pkg.msg);
//             break;
//         case LEAVE_GROUP_SUCC:
//             printf("%s\n", pkg.msg);
//             break;
//         case LOG_OUT:
//             sleep(1);
//             pthread_exit(NULL);
//             break;
//         default:
//             break;
//         }
//     }
// }

void see_active_user(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = SHOW_USER;
    send(client_socket, &pkg, sizeof(pkg), 0);

    // sleep(1);

    // recv(client_socket, &pkg, sizeof(pkg), 0);
}

void make_done(int msg) {
    doing = msg;
}

int check_receiver(int client_socket, char* receiver) {
    int res;
    Package pkg;
    
    strcpy(pkg.receiver, receiver);
    strcpy(pkg.sender, my_username);
    strcpy(pkg.msg, TESTING_MSG);
    pkg.ctrl_signal = PRIVATE_CHAT;
    doing = 0;
    send(client_socket, &pkg, sizeof(pkg), 0);
    printf("ccc\n");
    while(!doing);
    printf("dd\n");
    return doing;
    // sleep(1);
}

int private_chat(int client_socket, char *receiver, char *msg)
{
    Package pkg;
    strcpy(pkg.receiver, receiver);
    strcpy(pkg.sender, my_username);
    // recv(client_socket, &pkg, sizeof(pkg), 0);
    // res = pkg.ctrl_signal;
    // if(res == ERR_INVALID_RECEIVER) {
    //     printf("No user\n");
    //     return res;
    // }
    // strcpy(user_pub->username,NULL_STRING);
    printf("check key\n");
    int check = check_public_key(client_socket, receiver);
    if(check == 0) {
        user_pub->public_key->exponent = 0;
        printf("nnn\n");
        while( !user_pub->public_key->exponent);
        printf("mmm\n");
    }

    printf("Start chatting with %s\n", receiver);
    
    pkg.ctrl_signal = PRIVATE_CHAT;

    if(strcmp(msg, TESTING_MSG) == 0)
        strcpy(pkg.msg, msg);
    else {
        long long *encrypted = rsa_encrypt(msg, strlen(msg), user_pub->public_key);
        memset(pkg.encrypted_msg, '\0', sizeof(pkg.encrypted_msg));
        int i = 0;
        printf("Encrypted!\n");
        // printf("%s %ld\n", msg, strlen(msg));
        for(i = 0; i < strlen(msg); i++) {
            pkg.encrypted_msg[i] = (long long)encrypted[i];
            printf("%lld ", (long long)pkg.encrypted_msg[i]);
        }
        printf("\n");
        i = 0;
        while (pkg.encrypted_msg[i] != 0) {
            printf("%lld\n", pkg.encrypted_msg[i]);
            i++;
        }
        printf("\n");
    }
    send(client_socket, &pkg, sizeof(pkg), 0);
    printf("sent chat\n");

        // sleep(1);
    // }
}

void chat_all(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = CHAT_ALL;
    strcpy(pkg.sender, my_username);
    char msg[MSG_SIZE];
    while (1)
    {
        printf("Message(leave blank to exit group chat): \n");
        fgets(msg, MSG_SIZE, stdin);
        msg[strlen(msg) - 1] = '\0';
        if (strlen(msg) == 0)
        {
            break;
        }

        strcpy(pkg.msg, msg);
        send(client_socket, &pkg, sizeof(pkg), 0);

        // sleep(1);
    }
}
// 17/01/2023
//  xu ly lua chon trong group chat menu
// void group_chat_init(int client_socket)
// {
//     Package pkg;
//     pkg.ctrl_signal = GROUP_CHAT_INIT;
//     send(client_socket, &pkg, sizeof(pkg), 0);
//     // xu ly
//     int choice = 0;

//     while (1)
//     {
//         sleep(1);

//         group_chat_menu();
//         printf("Your choice: \n");
//         scanf("%d", &choice);
//         clear_stdin_buff();

//         switch (choice)
//         {
//         case 1:
//             show_group(client_socket);
//             break;
//         case 2:
//             new_group(client_socket);
//             break;
//         case 3:
//             join_group(client_socket);
//             break;
//         default:
//             return;
//         }
//     }
// }


char* group_msg_encrypt(char* msg, char* key) {

}

char* group_msg_decrypt(char* msg, char* key) {

}

// hien thi nhom hien tai
void show_group(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = SHOW_GROUP;
    send(client_socket, &pkg, sizeof(pkg), 0);
    // sleep(1);
}

// tao group moi
void new_group(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = NEW_GROUP;
    send(client_socket, &pkg, sizeof(pkg), 0);
}

// vao group cua minh

void join_group(int client_socket, char *group_name)
{
    // show_group(client_socket);
    // sleep(1);
    Package pkg;
    pkg.ctrl_signal = JOIN_GROUP;
    /* chon group*/
    // char group_name[GROUP_NAME_SIZE];
    // printf("Group Name (Group_n): \n");
    // fgets(group_name, GROUP_NAME_SIZE, stdin);
    // group_name[strlen(group_name) - 1] = '\0';
    strcpy(pkg.sender, my_username);
    strcpy(pkg.msg, group_name);
    send(client_socket, &pkg, sizeof(pkg), 0);
    // sleep(1);
    // if (join_succ == 1)
    //     handel_group_mess(client_socket);
    // else
    //     return;
}

// void handel_group_mess(int client_socket)
// {
//     Package pkg;
//     pkg.ctrl_signal = HANDEL_GROUP_MESS;
//     send(client_socket, &pkg, sizeof(pkg), 0);
//     // xu ly
//     int choice = 0;
//     int login_group = 1;
//     while (login_group)
//     {
//         sleep(1);

//         sub_group_chat_menu(curr_group_name);
//         printf("Your choice: \n");
//         scanf("%d", &choice);
//         clear_stdin_buff();

//         switch (choice)
//         {
//         case 1:
//             invite_friend(client_socket);
//             break;
//         case 2:
//             group_chat(client_socket);
//             break;
//         case 3:
//             show_group_info(client_socket);
//             break;
//         case 4:
//             leave_group(client_socket);
//             login_group = 0;
//             break;
//         default:
//             login_group = 0;
//             break;
//         }
//     }
//     join_succ = 0;
//     curr_group_id = -1;
//     return;
// }

// moi ban
void invite_friend(int client_socket, char *friend_username)
{
    // see_active_user(client_socket);
    Package pkg;
    // char friends_name[USERNAME_SIZE];

    // printf("Friends name: \n");
    // fgets(friends_name, USERNAME_SIZE, stdin);
    // friends_name[strlen(friends_name) - 1] = '\0';

    strcpy(pkg.sender, my_username);
    strcpy(pkg.receiver, friend_username);
    strcpy(pkg.msg, my_username);
    strcat(pkg.msg, " Added you to ");
    strcat(pkg.msg, curr_group_name);
    pkg.ctrl_signal = INVITE_FRIEND;
    pkg.group_id = curr_group_id;
    send(client_socket, &pkg, sizeof(pkg), 0);
}

// chat trong nhom
void group_chat(int client_socket, char *msg)
{
    Package pkg;
    pkg.ctrl_signal = GROUP_CHAT;
    pkg.group_id = curr_group_id;
    strcpy(pkg.sender, my_username);
    // char msg[MSG_SIZE];
    // while (1)
    // {
        // printf("Message(leave blank to exit group chat): \n");
        // fgets(msg, MSG_SIZE, stdin);
        // msg[strlen(msg) - 1] = '\0';
        // if (strlen(msg) == 0)
        // {
        //     break;
        // }

        strcpy(pkg.msg, msg);
        save_chat(&pkg);
        send(client_socket, &pkg, sizeof(pkg), 0);

    //     sleep(1);
    // }
}

// hien thi thong tin phong
void show_group_info(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = GROUP_INFO;
    pkg.group_id = curr_group_id;
    send(client_socket, &pkg, sizeof(pkg), 0);
}
// Thoat nhom
void leave_group(int client_socket)
{
    Package pkg;
    pkg.ctrl_signal = LEAVE_GROUP;
    pkg.group_id = curr_group_id;
    strcpy(pkg.sender, my_username);
    // curr_group_id = -1;
    send(client_socket, &pkg, sizeof(pkg), 0);
}

// main
// int main()
// {
//     int client_socket = connect_to_server();
//     ask_server(client_socket);
//     return 0;
// }