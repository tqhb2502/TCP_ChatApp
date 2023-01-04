#ifndef __SERVER_H__
#define __SERVER_H__

#include "network.h"
#include "error.h"
#include "account_manager.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_USER 10

#define EMPTY_STRING "EMPTY_STRING"

//* Người dùng hoạt động
typedef struct Active_user_ {
    char username[USERNAME_SIZE]; /* Tên đăng nhập của người dùng */
    int socket; /* Socket người dùng dùng để kết nối đến server */
} Active_user;

//* Khởi tạo server
/**
 * Tạo socket của server lắng nghe kết nối từ client
 * @return file descriptor của socket
*/
int create_listen_socket();

/**
 * Chấp nhận kết nối từ client
 * @param listen_socket socket lắng nghe kết nối
 * @return file descriptor của socket đã kết nối
*/
int accept_conn(int listen_socket);

/**
 * Đọc danh sách tài khoản
 * Tạo listen socket và chấp nhận kết nối
 * Phân luồng cho từng user
*/
void make_server();

//* Trước khi người dùng đăng nhập
/**
 * Tiếp nhận các yêu cầu: đăng nhập, thoát,...
 * @param param socket kết nối đến client
*/
void *pre_login_srv(void *param);

/**
 * Nhận tên đăng nhập, mật khẩu và kiểm tra
 * @param conn_socket socket kết nối đến client
 * @param acc_list danh sách tài khoản
*/
void handle_login(int conn_socket, Account *acc_list);

//* Sau khi người dùng đăng nhập
void sv_user_use(int conn_socket);

/**
 * Gửi cho người dùng danh sách người dùng đang online
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_active_user(int conn_socket, Package *pkg);

/**
 * Xử lý chức năng chat riêng
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_private_chat(int conn_socket, Package *pkg);

/**
 * Xử lý chức năng chat nhóm
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_group_chat(int conn_socket, Package *pkg);

/**
 * Xử lý chức năng đăng xuất
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_logout(int conn_socket, Package *pkg);

#endif