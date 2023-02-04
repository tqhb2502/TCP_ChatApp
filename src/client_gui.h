#ifndef __CLIENT_GUI_H__
#define __CLIENT_GUI_H__

#include <gtk/gtk.h>
#include <gtk/gtkx.h>

#define TEXT_SIZE 4096

#define NEWLINE "\n"
#define SPLITER ": "
#define HELLO_USER_MSG "Hello, "
#define MYSELF_INDICATOR "Bạn"
#define GROUP_NAME_INDICATOR "Tên nhóm"
#define GROUP_MEM_NUMBER_INDICATOR "Số lượng thành viên"
#define GROUP_MEM_USERNAME_INDICATOR "Tên người dùng của thành viên"
#define DEFAULT_CUR_CHAT_LABEL "#none#"

#define INCORRECT_ACC_NOTIF "Đăng nhập thất bại!\nTài khoản hoặc mật khẩu không chính xác."
#define SIGNED_IN_ACC_NOTIF "Đăng nhập thất bại!\nTài khoản đã được đăng nhập bởi người khác."
#define INVALID_RECEIVER_NOTIF "Thất bại!\nNgười nhận không online hoặc không tồn tại."
#define MAKE_GROUP_SUCC_NOTIF "Tạo nhóm mới thành công!\nTên nhóm mới của bạn là: "
#define GROUP_NOT_FOUND_NOTIF "Vào phòng chat thất bại!\nNhóm không tồn tại hoặc bạn không là thành viên của nhóm này."
#define NOT_IN_GROUP_ROOM_NOTIF "Thất bại!\nBạn đang không ở trong phòng chat của nhóm nào."
#define INVITE_MYSELF_NOTIF "Thất bại!\nKhông thể mời chính mình vào nhóm."
#define USER_NOT_FOUND_NOTIF "Thất bại!\nNgười bạn mời không online hoặc không tồn tại."
#define FULL_MEM_NOTIF "Thất bại!\nSố thành viên của nhóm đã đến giới hạn. Không thể mời thêm."
#define IS_MEM_NOTIF "Thất bại!\nNgười bạn mời đang là thành viên của nhóm từ trước."
#define INVITE_FRIEND_SUCC_NOTIF "Thành công!\nNgười dùng \"%s\" từ giờ sẽ là thành viên của nhóm \"%s\"."
#define INVITE_FRIEND_NOTIF "Chú ý!\nNgười dùng \"%s\" đã thêm bạn vào nhóm \"%s\"."
#define LEAVE_GROUP_SUCC_NOTIF "Rời nhóm thành công!"
#define NEW_MESSAGES_NOTIF "------------Tin nhắn mới------------"

//* builder for loading views
GtkBuilder *builder;

//* login window widgets
GtkWidget *login_window;
GtkWidget *login_fixed;
GtkWidget *login_acc_entry;
GtkWidget *login_pwd_entry;
GtkWidget *login_btn;
GtkWidget *signup_btn;
GtkWidget *login_exit_btn;

//* main window widgets
GtkWidget *main_window;
GtkWidget *main_fixed;
GtkWidget *online_sw;
GtkWidget *online_tv;
GtkWidget *group_sw;
GtkWidget *group_tv;
GtkWidget *private_chat_btn;
GtkWidget *group_chat_btn;
GtkWidget *refresh_list_btn;
GtkWidget *cur_user_label;
GtkWidget *logout_btn;
GtkWidget *group_invite_btn;
GtkWidget *group_info_btn;
GtkWidget *group_leave_btn;
GtkWidget *cur_chat_label;
GtkWidget *recv_msg_sw;
GtkWidget *recv_msg_tv;
GtkWidget *send_entry;
GtkWidget *send_btn;

//* receiver username dialog
GtkWidget *receiver_username_dialog;
GtkWidget *receiver_username_box;
GtkWidget *receiver_username_entry;
GtkWidget *receiver_username_btn_box;
GtkWidget *receiver_username_confirm_btn;
GtkWidget *receiver_username_exit_btn;

//* join group dialog
GtkWidget *join_group_dialog;
GtkWidget *join_group_box;
GtkWidget *join_group_entry;
GtkWidget *join_group_btn_box;
GtkWidget *join_group_join_btn;
GtkWidget *join_group_create_btn;
GtkWidget *join_group_exit_btn;

//* invite to group dialog
GtkWidget *invite_to_group_dialog;
GtkWidget *invite_to_group_box;
GtkWidget *invite_to_group_entry;
GtkWidget *invite_to_group_btn_box;
GtkWidget *invite_to_group_confirm_btn;
GtkWidget *invite_to_group_exit_btn;

//* group info dialog
GtkWidget *group_info_dialog;
GtkWidget *group_info_box;
GtkWidget *group_info_sw;
GtkWidget *group_info_tv;
GtkWidget *group_info_btn_box;
GtkWidget *group_info_confirm_btn;

//* signal handlers
void on_login_btn_clicked(GtkButton *btn, gpointer data);
void on_login_exit_btn_clicked(GtkButton *btn, gpointer data);

void on_logout_btn_clicked(GtkButton *btn, gpointer data);
void on_refresh_list_btn_clicked(GtkButton *btn, gpointer data);
void on_private_chat_btn_clicked(GtkButton *btn, gpointer data);
void on_group_chat_btn_clicked(GtkButton *btn, gpointer data);
void on_group_invite_btn_clicked(GtkButton *btn, gpointer data);
void on_group_info_btn_clicked(GtkButton *btn, gpointer data);
void on_group_leave_btn_clicked(GtkButton *btn, gpointer data);
void on_send_btn_clicked(GtkButton *btn, gpointer data);

void on_receiver_username_confirm_btn_clicked(GtkButton *btn, gpointer data);

void on_join_group_create_btn_clicked(GtkButton *btn, gpointer data);
void on_join_group_join_btn_clicked(GtkButton *btn, gpointer data);

void on_invite_to_group_confirm_btn_clicked(GtkButton *btn, gpointer data);

//* utility functions
void view_chat_history();
void see_chat(Package *pkg);

void scroll_window_to_bottom(GtkScrolledWindow *sw);

void insert_to_textview(GtkTextView *tv, gchar *text);
void delete_textview_content(GtkTextView *tv);
void delete_lastline_textview(GtkTextView *tv);

void notif_dialog(GtkWindow *parent, gchar *message);

void show_login_window(int *client_socket_pt);
void show_main_window(int *client_socket_pt);
void show_receiver_username_dialog(int *client_socket_pt);
void show_join_group_dialog(int *client_socket_pt);
void show_invite_to_group_dialog(int *client_socket_pt);
void show_group_info_dialog();

gpointer recv_handler(gpointer data);
gboolean recv_show_user(gpointer data);
gboolean recv_show_group(gpointer data);
gboolean recv_err_invalid_receiver(gpointer data);
gboolean recv_msg_sent_succ(gpointer data);
gboolean recv_private_chat(gpointer data);
gboolean recv_make_group_succ(gpointer data);
gboolean recv_join_group_succ(gpointer data);
gboolean recv_err_group_not_found(gpointer data);
gboolean recv_err_invite_myself(gpointer data);
gboolean recv_err_user_not_found(gpointer data);
gboolean recv_err_full_mem(gpointer data);
gboolean recv_err_is_mem(gpointer data);
gboolean recv_invite_friend_succ(gpointer data);
gboolean recv_invite_friend(gpointer data);
gboolean recv_group_chat(gpointer data);
gboolean recv_show_group_info_start(gpointer data);
gboolean recv_show_group_info_end(gpointer data);
gboolean recv_show_group_name(gpointer data);
gboolean recv_show_group_mem_number(gpointer data);
gboolean recv_show_group_mem_username(gpointer data);
gboolean recv_leave_group_succ(gpointer data);

#endif
