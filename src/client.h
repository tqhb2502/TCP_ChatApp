#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "server.h"
#include "network.h"
#include "error.h"
#include "util.h"

//* Kết nối
/**
 * Tạo socket và kết nối đến server
 * @return socket đã kết nối đến server
*/
int connect_to_server();

//* Menu
/**
 * Menu đăng nhập
 * 1. Đăng nhập
 * 2. Thoát
*/
void login_menu();

/**
 * Menu người dùng
 * 1. Chat riêng
 * 2. ...
*/
void user_menu();
/**
 * Menu group chat
 * 1. Chat riêng
 * 2. ...
*/
void group_chat_menu();

/**
 * sub menu group chat
 * 1. Chat riêng
 * 2. ...
*/
void sub_group_chat_menu(char *group_name);
//* Chức năng trước đăng nhập
/**
 * Yêu cầu đăng nhập hoặc thoát chương trình
 * @param client_socket socket đã kết nối đến server
*/
void ask_server(int client_socket);

/**
 * Đăng nhập với tên đăng nhập và mật khẩu
 * @param client_socket socket đã kết nối đến server
 * @return 0: đăng nhập thất bại
 * @return 1: đăng nhập thành công
*/
int login(int client_socket);

//* Chức năng sau đăng nhập
/**
 * Cho người dùng nhập lựa chọn, thực hiện chức năng tương ứng
 * Phân luồng đọc-ghi
 * @param client_socket socket đã kết nối đến server
*/
void user_use(int client_socket);

/**
 * Đọc nội dung tin nhắn
 * @param param socket kết nối đến server
*/
void *read_msg(void *param);

/**
 * Hiển thị danh sách người dùng đang hoạt động
 * @param client_socket socket đã kết nối đến server
*/
void see_active_user(int client_socket);

/**
 * Chat riêng:
 * - Nhập tên người nhận
 * - Nhập tin nhắn
 * @param client_socket socket đã kết nối đến server
*/
void private_chat(int client_socket);

/**
 * Chat all:
 * - ...
 * @param client_socket socket đã kết nối đến server
*/
void chat_all(int client_socket);

/**
 * Chat group:
 * - ...
 * @param client_socket socket đã kết nối đến server
*/
void group_chat_init(int client_socket);

/**
 * Show group:
 * - ...
 * @param client_socket socket đã kết nối đến server
*/
void show_group(int client_socket);

/**
 * New group:
 * - ...
 * @param client_socket socket đã kết nối đến server
*/
void new_group(int client_socket);

/**
 * Join group:
 * - ...
 * @param client_socket socket đã kết nối đến server
*/
void join_group(int client_socket);

/**
 * Xử lý khi đã join vào nhóm:
 * - ...
 * @param client_socket socket đã kết nối đến server
*/
void handel_group_mess(int client_socket);

/**
 * mời bạn:
 * - ...
 * @param client_socket socket đã kết nối đến server
*/
void invite_friend(int client_socket);

/**
 * chat trong nhom:
 * - ...
 * @param client_socket socket đã kết nối đến server
*/
void group_chat(int client_socket);


/**
 * Hiển thị thông tin phòng
 * - ...
 * @param client_socket socket đã kết nối đến server
*/
void show_group_info(int client_socket);

/**
 * Thoat nhom
 * - ...
 * @param client_socket socket đã kết nối đến server
*/
void leave_group(int client_socket);


/**
 * Đăng xuất khỏi tài khoản
 * @param client_socket socket đã kết nối đến server
*/
void logout(int client_socket);

#endif