#ifndef __SERVER_H__
#define __SERVER_H__

#include "network.h"
#include "error.h"

//* Tài khoản
typedef struct Account_ {
    char username[USERNAME_SIZE]; /* Tên đăng nhập */
    char password[PASSWORD_SIZE]; /* Mật khẩu */
    int is_signed_in; /* Tài khoản đã đăng nhập hay chưa */
    struct Account_ *next;
} Account;

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
 * Kiểm tra tên đăng nhập, mật khẩu
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
 * @return 0: đăng nhập thất bại
 * @return 1: đăng nhập thành công
*/
int handle_login(int conn_socket, Package *pkg);

//* Sau khi người dùng đăng nhập
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