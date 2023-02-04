#ifndef __SERVER_H__
#define __SERVER_H__

#include "network.h"
#include "db.h"
#include "error.h"
#include "account_manager.h"

#include "rsa.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_USER 1024
#define MAX_GROUP 10
#define NULL_STRING "#NULL_STRING#"
#define SERVER_SYSTEM_USERNAME "#server_system#"
#define GROUP_NAME_SIZE 30

//* Người dùng hoạt động
typedef struct Active_user_ {
    char username[USERNAME_SIZE]; /* Tên đăng nhập của người dùng */
    int socket; /* Socket người dùng dùng để kết nối đến server */
    int group_id[MAX_GROUP]; /*Group hien tai*/ 
} Active_user;

//* Group
typedef struct Member_{
    char username[USERNAME_SIZE]; /* Tên đăng nhập của người dùng */
    int socket; /* Socket người dùng dùng để kết nối đến server */
} Member;

typedef struct Group_ {
    Member group_member[MAX_USER]; /* Thành viên trong nhóm */
    int curr_num; /* Số người hiện tại trong nhóm */
    char group_name[GROUP_NAME_SIZE]; 
} Group;



typedef struct public_key_users_ {
    char username[USERNAME_SIZE];
    struct public_key_class public_key[1];
}Public_key_users;

void send_public_key(int client_socket, char* receiver);
void save_public_key(char* sender, char* msg);

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

/**
 * Search user by conn_socket
 * @param conn_socket socket kết nối đến client
 * @param acc_list danh sách tài khoản
*/
int search_user(int conn_socket);


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
 * Xử lý chức năng chat all
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_chat_all(int conn_socket, Package *pkg);

/**
 * Xử lý chức năng chat nhóm
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_group_chat_init(int conn_socket, Package *pkg);

/**
 * Gửi cho người dùng nhóm hiện tại
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_show_group(int conn_socket, Package *pkg);

/**
 * Tạo group mới
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_new_group(int conn_socket, Package *pkg);

/**
 * Thêm group cho user
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
int sv_add_group_user(Active_user *user, int group_id);

/**
 * Xoa group cho user
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
int sv_leave_group_user(Active_user *user, int group_id);

/**
 * Thêm user vao group
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
int sv_add_user(Active_user user, Group *group);

/**
 * user join group
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_join_group(int conn_socket, Package *pkg);

/**
 * Tìm ID group theo tên
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
int  sv_search_id_group(Group group[],Active_user user, char *group_name);

/**
 * Tìm ID user theo tên
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
int  sv_search_id_user(Active_user user[], char *user_name);

/**
 * Tìm ID user theo tên
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
int  sv_search_id_user_group(Group group, char *user_name);

/**
 * Moi ban
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_invite_friend(int conn_socket, Package *pkg);

/**
 * Chat trong nhom
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_group_chat(int conn_socket, Package *pkg);

/**
 * Hiển thị thông tin phòng
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_show_group_info(int conn_socket, Package *pkg);

/**
 * Thoat phòng
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_leave_group(int conn_socket, Package *pkg);

/**
 * Cap nhat cong cac group khi moi dang nhap
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_update_port_group(Active_user *user, Group *group);




/**
 * In ra thanh vien cua nhom
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void print_members(Group group);
/**
 * Kiem tra nguoi dung co o trong nhom khong
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
int check_user_in_group(Active_user user, int group_id);
/**
 * Xử lý chức năng đăng xuất
 * @param conn_socket socket kết nối đến client
 * @param pkg con trỏ đến gói tin nhận được từ client
*/
void sv_logout(int conn_socket, Package *pkg);

#endif