#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MSG_SIZE 2048
#define USERNAME_SIZE 256
#define PASSWORD_SIZE 256

//* Cấu trúc gói tin
typedef struct Package_ {
    char msg[MSG_SIZE]; /* nội dung thông điệp */
    char sender[USERNAME_SIZE]; /* username người gửi */
    char receiver[USERNAME_SIZE]; /* username người nhận */
    int ctrl_signal; /* mã lệnh */
} Package;

#endif