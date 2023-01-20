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

#define MSG_SENT_SUCC 301
#define END_CHAT 302

#define SHOW_USER 401
#define PRIVATE_CHAT 402
#define CHAT_ALL 403
#define LOG_OUT 404

#define GROUP_CHAT_INIT 405
#define SHOW_GROUP 406
#define NEW_GROUP  407
#define MSG_MAKE_GROUP_SUCC 408
#define MSG_MAKE_GROUP_ERR 409
#define JOIN_GROUP 410
#define JOIN_GROUP_SUCC 411
#define HANDEL_GROUP_MESS 412
#define INVITE_FRIEND 413
#define INVITE_FRIEND_SUCC 414
#define GROUP_CHAT 415
#define GROUP_INFO 416
#define SHOW_GROUP_NAME 417
#define SHOW_GROUP_MEM 418
#define LEAVE_GROUP 419
#define LEAVE_GROUP_SUCC 420

//* Cấu trúc gói tin
typedef struct Package_ {
    char msg[MSG_SIZE]; /* nội dung thông điệp */
    char sender[USERNAME_SIZE]; /* username người gửi */
    char receiver[USERNAME_SIZE]; /* username người nhận */
    int group_id; /*id group muốn gửi*/
    int ctrl_signal; /* mã lệnh */
} Package;

#endif