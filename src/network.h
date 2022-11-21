#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "account_manager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MSG_SIZE 2048

//* Tín hiệu điều khiển
// chung
// server
#define LOGIN_SUCC 101
#define INCORRECT_ACC 102
#define SIGNED_IN_ACC 103
#define RECV_SUCC 104
// client
#define LOGIN_REQ 201
#define QUIT_REQ 202

//* Cấu trúc gói tin
typedef struct Package_ {
    char msg[MSG_SIZE]; /* nội dung thông điệp */
    char sender[USERNAME_SIZE]; /* username người gửi */
    char receiver[USERNAME_SIZE]; /* username người nhận */
    int ctrl_signal; /* mã lệnh */
} Package;

#endif