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
#define LOGIN_SUCC 101 /* Đăng nhập thành công */
#define INCORRECT_ACC 102 /* Tài khoản hoặc mật khẩu không chính xác */
#define SIGNED_IN_ACC 103 /* Tài khoản đã được đăng nhập bởi người khác */
#define RECV_SUCC 104 /* Nhận dữ liệu thành công */
// client
#define LOGIN_REQ 201 /* Yêu cầu đăng nhập */
#define QUIT_REQ 202 /* Thoát ứng dụng */

//* Cấu trúc gói tin
typedef struct Package_ {
    char msg[MSG_SIZE]; /* nội dung thông điệp */
    char sender[USERNAME_SIZE]; /* username người gửi */
    char receiver[USERNAME_SIZE]; /* username người nhận */
    int ctrl_signal; /* mã lệnh */
} Package;

#endif