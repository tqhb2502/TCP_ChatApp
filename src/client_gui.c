#include "client.h"
#include "client_gui.h"
#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

Package pkg;
GMutex ui_mutex;
int is_done;

sqlite3 *database;
char **resultp;
int nrow, ncolumn;

//* ----------------------- SIGNAL HANDLERS -----------------------
//* login window
void on_login_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);
    int result;

    // get username & password
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(login_acc_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(login_pwd_entry));

    // send to server for checking
    pkg.ctrl_signal = LOGIN_REQ;
    send(client_socket, &pkg, sizeof(pkg), 0);
    result = login(client_socket, (char *) username, (char *) password);

    // checking result
    if (result == LOGIN_SUCC) {

        gtk_widget_destroy(GTK_WIDGET(login_window));
        show_main_window((int *) data);
        
    } else if (result == INCORRECT_ACC) {
        notif_dialog(GTK_WINDOW(login_window), INCORRECT_ACC_NOTIF);
    } else {
        notif_dialog(GTK_WINDOW(login_window), SIGNED_IN_ACC_NOTIF);
    }
}

void on_login_exit_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);

    pkg.ctrl_signal = QUIT_REQ;
    send(client_socket, &pkg, sizeof(pkg), 0);
    close(client_socket);

    g_mutex_clear(&ui_mutex);

    gtk_widget_destroy(GTK_WIDGET(login_window));
}

//* main window
void on_refresh_list_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);
    see_active_user(client_socket);
    show_group(client_socket);
}

void on_logout_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);

    pkg.ctrl_signal = LOG_OUT;
    send(client_socket, &pkg, sizeof(pkg), 0);
    strcpy(my_username, NULL_STRING);
    strcpy(curr_group_name, NULL_STRING);
    curr_group_id = -1;

    gtk_widget_destroy(GTK_WIDGET(main_window));
    show_login_window((int *) data);
}

void on_private_chat_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);
    show_receiver_username_dialog((int *) data);
}

void on_group_chat_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);
    show_join_group_dialog((int *) data);
}

void on_group_invite_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);

    if (curr_group_id != -1) {
        show_invite_to_group_dialog((int *) data);
    } else {
        notif_dialog(GTK_WINDOW(main_window), NOT_IN_GROUP_ROOM_NOTIF);
    }
}

void on_group_info_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);

    if (curr_group_id != -1) {
        show_group_info(client_socket);
    } else {
        notif_dialog(GTK_WINDOW(main_window), NOT_IN_GROUP_ROOM_NOTIF);
    }
}

void on_group_leave_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);

    if (curr_group_id != -1) {
        leave_group(client_socket);
    } else {
        notif_dialog(GTK_WINDOW(main_window), NOT_IN_GROUP_ROOM_NOTIF);
    }
}

void on_send_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);

    const gchar *receiver = gtk_label_get_text(GTK_LABEL(cur_chat_label));
    const gchar *message = gtk_entry_get_text(GTK_ENTRY(send_entry));

    if (curr_group_id != -1) {
        group_chat(client_socket, (char *) message);
    } else {
        int res = check_receiver(client_socket, (char*)receiver);
        if(res == ERR_INVALID_RECEIVER) return;
        private_chat(client_socket, (char *) receiver, (char *) message);
        insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), MYSELF_INDICATOR);
        insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), SPLITER);
        insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), (gchar *) message);
        insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), NEWLINE);

        while (gtk_events_pending()) gtk_main_iteration();
        scroll_window_to_bottom(GTK_SCROLLED_WINDOW(recv_msg_sw));
    }

    GtkEntryBuffer *entry_buffer;
    entry_buffer = gtk_entry_get_buffer(GTK_ENTRY(send_entry));
    gtk_entry_buffer_delete_text(entry_buffer, 0, -1);
}

//* reciever username dialog
void on_receiver_username_confirm_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);
    const gchar *receiver = gtk_entry_get_text(GTK_ENTRY(receiver_username_entry));
    check_receiver(client_socket, (char*)receiver);

    gtk_widget_destroy(receiver_username_dialog);
}

//* join group dialog
void on_join_group_create_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);
    new_group(client_socket);
    show_group(client_socket);
}

void on_join_group_join_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);
    const gchar *group_name = gtk_entry_get_text(GTK_ENTRY(join_group_entry));
    join_group(client_socket, (char *) group_name);

    gtk_widget_destroy(join_group_dialog);
}

//* invite to group dialog
void on_invite_to_group_confirm_btn_clicked(GtkButton *btn, gpointer data) {

    int client_socket = *((int *) data);
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(invite_to_group_entry));
    invite_friend(client_socket, (char *) username);

    gtk_widget_destroy(invite_to_group_dialog);
}

//* ----------------------- UTILITY FUNCTIONS -----------------------
void view_chat_history()
{
    Package pkg;
    pkg.group_id = curr_group_id;
    strcpy(pkg.sender, my_username);
    see_chat(&pkg);
}

void see_chat(Package *pkg)
{
    database = Create_room_sqlite(pkg);
    int ret;
    char *errmsg = NULL;
    char buf[MAX_SQL_SIZE] = "create table if not exists chat(time TEXT, sender TEXT, message TEXT)";
    ret = sqlite3_exec(database, buf, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("Open table failed\n");
        return;
    }

    // xem lich su
    resultp = NULL;
    char *sq1 = "select * from chat";
    ret = sqlite3_get_table(database, sq1, &resultp, &nrow, &ncolumn, &errmsg);
    if (ret != SQLITE_OK)
    {
        printf("Database operation failed\n");
        printf("sqlite3_get_table: %s\n", errmsg);
    }

    // sqlite3_free_table(resultp);

    // sqlite3_close(database);
}

void scroll_window_to_bottom(GtkScrolledWindow *sw) {

    GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(sw);
    // double value = gtk_adjustment_get_value (vadj);
    double upper = gtk_adjustment_get_upper(vadj);
    double page_size = gtk_adjustment_get_page_size(vadj);
    // printf("%.2lf %.2lf\n", upper, page_size);
    gtk_adjustment_set_value(vadj, upper - page_size);
}

void insert_to_textview(GtkTextView *tv, gchar *text) {

    GtkTextBuffer *buffer;
    GtkTextIter end_iter;

    buffer = gtk_text_view_get_buffer(tv);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_insert(buffer, &end_iter, text, -1);
}

void delete_textview_content(GtkTextView *tv) {
    
    GtkTextBuffer *buffer;
    GtkTextIter start_iter;
    GtkTextIter end_iter;

    buffer = gtk_text_view_get_buffer(tv);
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
}

void delete_lastline_textview(GtkTextView *tv) {

    GtkTextBuffer *buffer;
    GtkTextIter last_line_start_iter, end_iter;
    gint line_count;

    buffer = gtk_text_view_get_buffer(tv);
    line_count = gtk_text_buffer_get_line_count(buffer);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);

    gtk_text_buffer_get_iter_at_line(buffer, &last_line_start_iter, line_count - 2);
    gtk_text_buffer_delete(buffer, &last_line_start_iter, &end_iter);
}

void notif_dialog(GtkWindow *parent, gchar *message) {

    GtkWidget *dialog, *label, *content_area;
    GtkDialogFlags flags;

    // Create the widgets
    flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    dialog = gtk_dialog_new_with_buttons("Thông Báo", parent, flags, "OK", GTK_RESPONSE_NONE, NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
    
    label = gtk_label_new(message);
    gtk_widget_set_margin_top(label, 20);
    gtk_widget_set_margin_bottom(label, 20);
    gtk_widget_set_margin_start(label, 10);
    gtk_widget_set_margin_end(label, 10);

    // Ensure that the dialog box is destroyed when the user responds or destroys it
    g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);

    // Add the label, and show everything we’ve added
    gtk_container_add (GTK_CONTAINER (content_area), label);
    gtk_widget_show_all (dialog);
}

void show_login_window(int *client_socket_pt) {

    // load login window
    builder = gtk_builder_new_from_file("../views/login_window.glade");

    // load login window's widgets
    login_window = GTK_WIDGET(gtk_builder_get_object(builder, "login_window"));
    login_fixed = GTK_WIDGET(gtk_builder_get_object(builder, "login_fixed"));
    login_acc_entry = GTK_WIDGET(gtk_builder_get_object(builder, "login_acc_entry"));
    login_pwd_entry = GTK_WIDGET(gtk_builder_get_object(builder, "login_pwd_entry"));
    login_btn = GTK_WIDGET(gtk_builder_get_object(builder, "login_btn"));
    signup_btn = GTK_WIDGET(gtk_builder_get_object(builder, "signup_btn"));
    login_exit_btn = GTK_WIDGET(gtk_builder_get_object(builder, "login_exit_btn"));

    // load login window's signal handlers
    g_signal_connect(login_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(login_btn, "clicked", G_CALLBACK(on_login_btn_clicked), client_socket_pt);
    g_signal_connect(login_exit_btn, "clicked", G_CALLBACK(on_login_exit_btn_clicked), client_socket_pt);

    // show login window and wait for signals
    gtk_widget_show(login_window);
    gtk_main();
}

void show_main_window(int *client_socket_pt) {

    // load main window
    builder = gtk_builder_new_from_file("../views/main_window.glade");

    // load main window's widgets
    main_window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    main_fixed = GTK_WIDGET(gtk_builder_get_object(builder, "main_fixed"));
    online_sw = GTK_WIDGET(gtk_builder_get_object(builder, "online_sw"));
    online_tv = GTK_WIDGET(gtk_builder_get_object(builder, "online_tv"));
    group_sw = GTK_WIDGET(gtk_builder_get_object(builder, "group_sw"));
    group_tv = GTK_WIDGET(gtk_builder_get_object(builder, "group_tv"));
    private_chat_btn = GTK_WIDGET(gtk_builder_get_object(builder, "private_chat_btn"));
    group_chat_btn = GTK_WIDGET(gtk_builder_get_object(builder, "group_chat_btn"));
    refresh_list_btn = GTK_WIDGET(gtk_builder_get_object(builder, "refresh_list_btn"));
    cur_user_label = GTK_WIDGET(gtk_builder_get_object(builder, "cur_user_label"));
    logout_btn = GTK_WIDGET(gtk_builder_get_object(builder, "logout_btn"));
    group_invite_btn = GTK_WIDGET(gtk_builder_get_object(builder, "group_invite_btn"));
    group_info_btn = GTK_WIDGET(gtk_builder_get_object(builder, "group_info_btn"));
    group_leave_btn = GTK_WIDGET(gtk_builder_get_object(builder, "group_leave_btn"));
    cur_chat_label = GTK_WIDGET(gtk_builder_get_object(builder, "cur_chat_label"));
    recv_msg_sw = GTK_WIDGET(gtk_builder_get_object(builder, "recv_msg_sw"));
    recv_msg_tv = GTK_WIDGET(gtk_builder_get_object(builder, "recv_msg_tv"));
    send_entry = GTK_WIDGET(gtk_builder_get_object(builder, "send_entry"));
    send_btn = GTK_WIDGET(gtk_builder_get_object(builder, "send_btn"));

    // load main window's signal handlers
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(logout_btn, "clicked", G_CALLBACK(on_logout_btn_clicked), client_socket_pt);
    g_signal_connect(refresh_list_btn, "clicked", G_CALLBACK(on_refresh_list_btn_clicked), client_socket_pt);
    g_signal_connect(private_chat_btn, "clicked", G_CALLBACK(on_private_chat_btn_clicked), client_socket_pt);
    g_signal_connect(group_chat_btn, "clicked", G_CALLBACK(on_group_chat_btn_clicked), client_socket_pt);
    g_signal_connect(send_btn, "clicked", G_CALLBACK(on_send_btn_clicked), client_socket_pt);
    g_signal_connect(group_invite_btn, "clicked", G_CALLBACK(on_group_invite_btn_clicked), client_socket_pt);
    g_signal_connect(group_info_btn, "clicked", G_CALLBACK(on_group_info_btn_clicked), client_socket_pt);
    g_signal_connect(group_leave_btn, "clicked", G_CALLBACK(on_group_leave_btn_clicked), client_socket_pt);

    // create read handler
    g_thread_new("recv_handler", recv_handler, client_socket_pt);

    // init actions
    // hello message
    char hello_cur_user_msg[USERNAME_SIZE + 7] = {0};
    strcpy(hello_cur_user_msg, HELLO_USER_MSG);
    strcat(hello_cur_user_msg, my_username);
    gtk_label_set_text(GTK_LABEL(cur_user_label), hello_cur_user_msg);
    // default cur chat
    gtk_label_set_text(GTK_LABEL(cur_chat_label), DEFAULT_CUR_CHAT_LABEL);
    // show currently online users
    see_active_user(*client_socket_pt);
    // show groups that user is in
    show_group(*client_socket_pt);
    // make text view not editable
    gtk_text_view_set_editable(GTK_TEXT_VIEW(online_tv), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(group_tv), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(recv_msg_tv), FALSE);

    // show main window and wait for signals
    gtk_widget_show(main_window);
    gtk_main();
}

void show_receiver_username_dialog(int *client_socket_pt) {

    builder = gtk_builder_new_from_file("../views/receiver_username_dialog.glade");

    receiver_username_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "receiver_username_dialog"));
    receiver_username_box = GTK_WIDGET(gtk_builder_get_object(builder, "receiver_username_box"));
    receiver_username_entry = GTK_WIDGET(gtk_builder_get_object(builder, "receiver_username_entry"));
    receiver_username_btn_box = GTK_WIDGET(gtk_builder_get_object(builder, "receiver_username_btn_box"));
    receiver_username_confirm_btn = GTK_WIDGET(gtk_builder_get_object(builder, "receiver_username_confirm_btn"));
    receiver_username_exit_btn = GTK_WIDGET(gtk_builder_get_object(builder, "receiver_username_exit_btn"));

    g_signal_connect_swapped(main_window, "destroy", G_CALLBACK(gtk_widget_destroy), receiver_username_dialog);
    g_signal_connect(receiver_username_confirm_btn, "clicked", G_CALLBACK(on_receiver_username_confirm_btn_clicked), client_socket_pt);
    g_signal_connect_swapped(receiver_username_exit_btn, "clicked", G_CALLBACK(gtk_widget_destroy), receiver_username_dialog);

    gtk_widget_show_all(receiver_username_dialog);
}

void show_join_group_dialog(int *client_socket_pt) {

    builder = gtk_builder_new_from_file("../views/join_group_dialog.glade");

    join_group_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "join_group_dialog"));
    join_group_box = GTK_WIDGET(gtk_builder_get_object(builder, "join_group_box"));
    join_group_entry = GTK_WIDGET(gtk_builder_get_object(builder, "join_group_entry"));
    join_group_btn_box = GTK_WIDGET(gtk_builder_get_object(builder, "join_group_btn_box"));
    join_group_join_btn = GTK_WIDGET(gtk_builder_get_object(builder, "join_group_join_btn"));
    join_group_create_btn = GTK_WIDGET(gtk_builder_get_object(builder, "join_group_create_btn"));
    join_group_exit_btn = GTK_WIDGET(gtk_builder_get_object(builder, "join_group_exit_btn"));

    g_signal_connect_swapped(main_window, "destroy", G_CALLBACK(gtk_widget_destroy), join_group_dialog);
    g_signal_connect_swapped(join_group_exit_btn, "clicked", G_CALLBACK(gtk_widget_destroy), join_group_dialog);
    g_signal_connect(join_group_create_btn, "clicked", G_CALLBACK(on_join_group_create_btn_clicked), client_socket_pt);
    g_signal_connect(join_group_join_btn, "clicked", G_CALLBACK(on_join_group_join_btn_clicked), client_socket_pt);

    gtk_widget_show_all(join_group_dialog);
}

void show_invite_to_group_dialog(int *client_socket_pt) {

    builder = gtk_builder_new_from_file("../views/invite_to_group_dialog.glade");

    invite_to_group_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "invite_to_group_dialog"));
    invite_to_group_box = GTK_WIDGET(gtk_builder_get_object(builder, "invite_to_group_box"));
    invite_to_group_entry = GTK_WIDGET(gtk_builder_get_object(builder, "invite_to_group_entry"));
    invite_to_group_btn_box = GTK_WIDGET(gtk_builder_get_object(builder, "invite_to_group_btn_box"));
    invite_to_group_confirm_btn = GTK_WIDGET(gtk_builder_get_object(builder, "invite_to_group_confirm_btn"));
    invite_to_group_exit_btn = GTK_WIDGET(gtk_builder_get_object(builder, "invite_to_group_exit_btn"));

    g_signal_connect_swapped(main_window, "destroy", G_CALLBACK(gtk_widget_destroy), invite_to_group_dialog);
    g_signal_connect_swapped(invite_to_group_exit_btn, "clicked", G_CALLBACK(gtk_widget_destroy), invite_to_group_dialog);
    g_signal_connect(invite_to_group_confirm_btn, "clicked", G_CALLBACK(on_invite_to_group_confirm_btn_clicked), client_socket_pt);

    gtk_widget_show_all(invite_to_group_dialog);
}

void show_group_info_dialog() {

    builder = gtk_builder_new_from_file("../views/group_info_dialog.glade");

    group_info_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "group_info_dialog"));
    group_info_box = GTK_WIDGET(gtk_builder_get_object(builder, "group_info_box"));
    group_info_sw = GTK_WIDGET(gtk_builder_get_object(builder, "group_info_sw"));
    group_info_tv = GTK_WIDGET(gtk_builder_get_object(builder, "group_info_tv"));
    group_info_btn_box = GTK_WIDGET(gtk_builder_get_object(builder, "group_info_btn_box"));
    group_info_confirm_btn = GTK_WIDGET(gtk_builder_get_object(builder, "group_info_confirm_btn"));

    g_signal_connect_swapped(main_window, "destroy", G_CALLBACK(gtk_widget_destroy), group_info_dialog);
    g_signal_connect_swapped(group_info_confirm_btn, "clicked", G_CALLBACK(gtk_widget_destroy), group_info_dialog);

    gtk_widget_show_all(group_info_dialog);
}

gpointer recv_handler(gpointer data) {
    int *c_socket = (int *)data;
    int client_socket = *c_socket;
    // printf("\nmysoc: %d\n", client_socket);
    // int client_socket = my_socket;
    Package pkg;
    while (1)
    {
        recv(client_socket, &pkg, sizeof(pkg), 0);
        // printf("receive %d from server\n", pkg.ctrl_signal);
        // set the task to be not done yet
        is_done = 0;
        // printf("%d\n", pkg.ctrl_signal);
        switch (pkg.ctrl_signal) {

            case SHOW_USER:
                gdk_threads_add_idle(recv_show_user, pkg.msg);
                break;

            case PRIVATE_CHAT:
                gdk_threads_add_idle(recv_private_chat, &pkg);
                break;
            
            case SEND_PUBLIC_KEY:
                // printf("Receive public key!\n");
                receive_public_key(client_socket, &pkg);
                is_done = 1;
                break;
                       
            // case CHAT_ALL:
            //     printf("%s to all: %s\n", pkg.sender, pkg.msg);
            //     break;

            case ERR_INVALID_RECEIVER:
                make_done(ERR_INVALID_RECEIVER);
                gdk_threads_add_idle(recv_err_invalid_receiver, &pkg);
                break;
            case MSG_SENT_SUCC:
                make_done(MSG_SENT_SUCC);
                gdk_threads_add_idle(recv_msg_sent_succ, &pkg);
                break;
            // case GROUP_CHAT_INIT:
            //     printf("%s\n", pkg.msg);
            //     break;
            case SHOW_GROUP:
                gdk_threads_add_idle(recv_show_group, pkg.msg);
                break;

            case MSG_MAKE_GROUP_SUCC:
                gdk_threads_add_idle(recv_make_group_succ, pkg.msg);
                break;
            case JOIN_GROUP_SUCC:
                strcpy(curr_group_name, pkg.msg);
                curr_group_id = pkg.group_id;
                join_succ = 1;
                gdk_threads_add_idle(recv_join_group_succ, pkg.msg);
                break;
            case INVITE_FRIEND:
                gdk_threads_add_idle(recv_invite_friend, &pkg);
                break;
            case ERR_GROUP_NOT_FOUND:
                gdk_threads_add_idle(recv_err_group_not_found, NULL);
                break;
            case ERR_IVITE_MYSELF:
                gdk_threads_add_idle(recv_err_invite_myself, NULL);
                break;
            case ERR_USER_NOT_FOUND:
                gdk_threads_add_idle(recv_err_user_not_found, NULL);
                break;
            case ERR_FULL_MEM:
                gdk_threads_add_idle(recv_err_full_mem, NULL);
                break;
            case ERR_IS_MEM:
                gdk_threads_add_idle(recv_err_is_mem, NULL);
                break;
            case INVITE_FRIEND_SUCC:
                gdk_threads_add_idle(recv_invite_friend_succ, &pkg);
                break;
            case GROUP_CHAT:
                gdk_threads_add_idle(recv_group_chat, &pkg);
                // if (curr_group_id == pkg.group_id)
                // {
                //     printf("%s: %s\n", pkg.sender, pkg.msg);
                // }
                // else
                // {
                //     printf("%s sent to Group_%d: %s\n", pkg.sender, pkg.group_id, pkg.msg);
                // }
                break;
            case SHOW_GROUP_INFO_START:
                gdk_threads_add_idle(recv_show_group_info_start, NULL);
                break;
            case SHOW_GROUP_NAME:
                gdk_threads_add_idle(recv_show_group_name, pkg.msg);
                break;
            case SHOW_GROUP_MEM_NUMBER:
                gdk_threads_add_idle(recv_show_group_mem_number, pkg.msg);
                break;
            case SHOW_GROUP_MEM_USERNAME:
                gdk_threads_add_idle(recv_show_group_mem_username, pkg.msg);
                break;
            case SHOW_GROUP_INFO_END:
                gdk_threads_add_idle(recv_show_group_info_end, NULL);
                break;
            case LEAVE_GROUP_SUCC:
                gdk_threads_add_idle(recv_leave_group_succ, NULL);
                break;
            case LOG_OUT:
                g_thread_exit(NULL);
                break;
            default:
                is_done = 1;
                break;
        }
        // wait for the task to be done
        while (!is_done);
    }

    return NULL;
}

gboolean recv_show_user(gpointer data) {

    char *str = (char *) data;
    char text[USERNAME_SIZE];
    int i = -1;
    int j;

    g_mutex_lock(&ui_mutex);

    // delete old content
    delete_textview_content(GTK_TEXT_VIEW(online_tv));

    // add new user list
    while (1) {

        j = i + 1;
        if (str[j] == ' ' || str[j] == '\0') break;

        i++;
        while (str[i] != ' ' && str[i] != '\0') i++;

        memset(text, '\0', sizeof(text));
        strncpy(text, str + j, i - j);
        insert_to_textview(GTK_TEXT_VIEW(online_tv), text);
        insert_to_textview(GTK_TEXT_VIEW(online_tv), NEWLINE);
        
        if (str[i] == '\0') break;
    }

    // scroll to bottom
    while (gtk_events_pending()) gtk_main_iteration();
    scroll_window_to_bottom(GTK_SCROLLED_WINDOW(online_sw));

    // task is done
    is_done = 1;

    g_mutex_unlock(&ui_mutex);

    return FALSE;
}

gboolean recv_show_group(gpointer data) {

    char *str = (char *) data;
    char text[USERNAME_SIZE];
    int i = -1;
    int j;

    g_mutex_lock(&ui_mutex);

    // delete old content
    delete_textview_content(GTK_TEXT_VIEW(group_tv));

    // add new user list
    while (1) {

        j = i + 1;
        if (str[j] == ' ' || str[j] == '\0') break;

        i++;
        while (str[i] != ' ' && str[i] != '\0') i++;

        memset(text, '\0', sizeof(text));
        strncpy(text, str + j, i - j);
        insert_to_textview(GTK_TEXT_VIEW(group_tv), text);
        insert_to_textview(GTK_TEXT_VIEW(group_tv), NEWLINE);
        
        if (str[i] == '\0') break;
    }

    // scroll to bottom
    while (gtk_events_pending()) gtk_main_iteration();
    scroll_window_to_bottom(GTK_SCROLLED_WINDOW(group_sw));

    // task is done
    is_done = 1;

    g_mutex_unlock(&ui_mutex);

    return FALSE;
}

gboolean recv_err_invalid_receiver(gpointer data) {

    Package *pkg_pt = (Package *) data;

    g_mutex_lock(&ui_mutex);

    notif_dialog(GTK_WINDOW(main_window), INVALID_RECEIVER_NOTIF);
    if (strcmp(pkg_pt->msg, TESTING_MSG) != 0) {
        delete_lastline_textview(GTK_TEXT_VIEW(recv_msg_tv));
    }

    g_mutex_unlock(&ui_mutex);

    // task is done
    is_done = 1;
    return FALSE;
}

gboolean recv_msg_sent_succ(gpointer data) {

    Package *pkg_pt = (Package *) data;
    
    g_mutex_lock(&ui_mutex);

    if (strcmp(pkg_pt->msg, TESTING_MSG) == 0) {
        // if user is in a group room, out of this group room
        printf("11\n");
        const gchar* receiver = gtk_label_get_text(GTK_LABEL(cur_chat_label));
        if(strcmp(pkg_pt->receiver, receiver) != 0){
            printf("22\n");
            curr_group_id = -1;
            join_succ = 0;
            gtk_label_set_text(GTK_LABEL(cur_chat_label), (const gchar *) pkg_pt->receiver);
            delete_textview_content(GTK_TEXT_VIEW(recv_msg_tv));
        }
        printf("33\n");
    }
    gtk_widget_grab_focus(send_entry);

    g_mutex_unlock(&ui_mutex);

    // task is done
    is_done = 1;
    return FALSE;
}

gboolean recv_private_chat(gpointer data) {

    Package *pkg_pt = (Package *) data;

    if (strcmp(pkg_pt->msg, TESTING_MSG) == 0) {
        is_done = 1;
        return FALSE;
    }

    g_mutex_lock(&ui_mutex);

    // if user is in a group room, out of this group room
    curr_group_id = -1;
    join_succ = 0;

    int i = 0;
    while(pkg_pt->encrypted_msg[i] != 0) {
        // printf("%lld\n", pkg_pt->encrypted_msg[i]);
        i++;
    }
    
    printf("Private Key:\n Modulus: %lld\n Exponent: %lld\n", (long long)my_priv->modulus, (long long)my_priv->exponent);
    // printf("%ld\n", sizeof(pkg_pt->encrypted_msg));
    char* decrypted = rsa_decrypt((long long*)pkg_pt->encrypted_msg, i * 8, my_priv);

    const gchar *cur_chat_label_content = gtk_label_get_text(GTK_LABEL(cur_chat_label));
    if (strcmp(cur_chat_label_content, pkg_pt->sender) != 0) {
        gtk_label_set_text(GTK_LABEL(cur_chat_label), (const gchar *) pkg_pt->sender);
        delete_textview_content(GTK_TEXT_VIEW(recv_msg_tv));
    }

    if (strcmp(my_username, pkg_pt->sender) != 0) {

        insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), pkg_pt->sender);
        insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), SPLITER);
        insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), decrypted);
        insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), NEWLINE);

        while (gtk_events_pending()) gtk_main_iteration();
        scroll_window_to_bottom(GTK_SCROLLED_WINDOW(recv_msg_sw));
    }

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_make_group_succ(gpointer data) {

    char *group_name = (char *) data;

    g_mutex_lock(&ui_mutex);

    char text[TEXT_SIZE] = {0};
    strcpy(text, MAKE_GROUP_SUCC_NOTIF);
    strcat(text, group_name);
    notif_dialog(GTK_WINDOW(main_window), (gchar *) &text);
    gtk_widget_grab_focus(join_group_entry);

    g_mutex_unlock(&ui_mutex);

    // task is done
    is_done = 1;
    return FALSE;
}

gboolean recv_join_group_succ(gpointer data) {

    char *group_name = (char *) data;

    g_mutex_lock(&ui_mutex);

    // label
    gtk_label_set_text(GTK_LABEL(cur_chat_label), (const gchar *) group_name);

    // delete old recv_msg_tv content
    delete_textview_content(GTK_TEXT_VIEW(recv_msg_tv));

    // print chat history
    view_chat_history();

    for (int i = 3; i < (nrow + 1) * ncolumn; i++)
    {
        // if (i % 3 == 0)
        // {
        //     printf("\n");
        //     printf("%-25s", resultp[i]);
        // }
        // else
        //     printf("%15s", resultp[i]);
        if (i % 3 == 1) {
            insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), resultp[i]);
            insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), SPLITER);
        }
        if (i % 3 == 2) {
            insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), resultp[i]);
            insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), NEWLINE);
        }
    }

    insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), NEW_MESSAGES_NOTIF);
    insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), NEWLINE);

    while (gtk_events_pending()) gtk_main_iteration();
    scroll_window_to_bottom(GTK_SCROLLED_WINDOW(recv_msg_sw));

    sqlite3_free_table(resultp);
    sqlite3_close(database);

    // focus on send_netry
    gtk_widget_grab_focus(send_entry);

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_err_group_not_found(gpointer data) {

    g_mutex_lock(&ui_mutex);

    notif_dialog(GTK_WINDOW(main_window), GROUP_NOT_FOUND_NOTIF);

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_err_invite_myself(gpointer data) {

    g_mutex_lock(&ui_mutex);
    notif_dialog(GTK_WINDOW(main_window), INVITE_MYSELF_NOTIF);
    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_err_user_not_found(gpointer data) {

    g_mutex_lock(&ui_mutex);
    notif_dialog(GTK_WINDOW(main_window), USER_NOT_FOUND_NOTIF);
    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_err_full_mem(gpointer data) {

    g_mutex_lock(&ui_mutex);
    notif_dialog(GTK_WINDOW(main_window), FULL_MEM_NOTIF);
    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_err_is_mem(gpointer data) {

    g_mutex_lock(&ui_mutex);
    notif_dialog(GTK_WINDOW(main_window), IS_MEM_NOTIF);
    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_invite_friend_succ(gpointer data) {

    Package *pkg_pt = (Package *) data;

    g_mutex_lock(&ui_mutex);

    char text[TEXT_SIZE] = {0};
    sprintf(text, INVITE_FRIEND_SUCC_NOTIF, pkg_pt->receiver, pkg_pt->msg);
    notif_dialog(GTK_WINDOW(main_window), text);

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_invite_friend(gpointer data) {

    Package *pkg_pt = (Package *) data;

    g_mutex_lock(&ui_mutex);

    char text[TEXT_SIZE] = {0};
    sprintf(text, INVITE_FRIEND_NOTIF, pkg_pt->sender, pkg_pt->msg);
    notif_dialog(GTK_WINDOW(main_window), text);
    gtk_button_clicked(GTK_BUTTON(refresh_list_btn));

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_group_chat(gpointer data) {

    Package *pkg_pt = (Package *) data;

    g_mutex_lock(&ui_mutex);

    if (curr_group_id != pkg_pt->group_id) {

        join_succ = 1;
        curr_group_id = pkg_pt->group_id;

        char group_name[GROUP_NAME_SIZE] = {0};
        sprintf(group_name, "Group_%d", curr_group_id);
        gtk_label_set_text(GTK_LABEL(cur_chat_label), group_name);

        delete_textview_content(GTK_TEXT_VIEW(recv_msg_tv));

        // print chat history
        view_chat_history();

        int cell_num;
        if (strcmp(pkg_pt->sender, SERVER_SYSTEM_USERNAME) == 0) {
            cell_num = (nrow + 1) * ncolumn;
        } else {
            cell_num = (nrow + 1) * ncolumn - 3;
        }

        for (int i = 3; i < cell_num; i++)
        {
            // if (i % 3 == 0)
            // {
            //     printf("\n");
            //     printf("%-25s", resultp[i]);
            // }
            // else
            //     printf("%15s", resultp[i]);
            if (i % 3 == 1) {
                insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), resultp[i]);
                insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), SPLITER);
            }
            if (i % 3 == 2) {
                insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), resultp[i]);
                insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), NEWLINE);
            }
        }

        insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), NEW_MESSAGES_NOTIF);
        insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), NEWLINE);

        sqlite3_free_table(resultp);
        sqlite3_close(database);
    }

    insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), pkg_pt->sender);
    insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), SPLITER);
    insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), pkg_pt->msg);
    insert_to_textview(GTK_TEXT_VIEW(recv_msg_tv), NEWLINE);
    
    while (gtk_events_pending()) gtk_main_iteration();
    scroll_window_to_bottom(GTK_SCROLLED_WINDOW(recv_msg_sw));

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_show_group_info_start(gpointer data) {

    g_mutex_lock(&ui_mutex);

    show_group_info_dialog();
    gtk_widget_set_sensitive(group_info_confirm_btn, FALSE);

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_show_group_info_end(gpointer data) {

    g_mutex_lock(&ui_mutex);

    gtk_widget_set_sensitive(group_info_confirm_btn, TRUE);

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_show_group_name(gpointer data) {

    char *str = (char *) data;

    g_mutex_lock(&ui_mutex);

    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), GROUP_NAME_INDICATOR);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), SPLITER);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), str);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), NEWLINE);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), NEWLINE);

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_show_group_mem_number(gpointer data) {

    char *str = (char *) data;

    g_mutex_lock(&ui_mutex);

    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), GROUP_MEM_NUMBER_INDICATOR);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), SPLITER);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), str);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), NEWLINE);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), NEWLINE);

    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), GROUP_MEM_USERNAME_INDICATOR);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), SPLITER);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), NEWLINE);

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_show_group_mem_username(gpointer data) {

    char *str = (char *) data;

    g_mutex_lock(&ui_mutex);

    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), str);
    insert_to_textview(GTK_TEXT_VIEW(group_info_tv), NEWLINE);

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}

gboolean recv_leave_group_succ(gpointer data) {

    g_mutex_lock(&ui_mutex);

    join_succ = 0;
    curr_group_id = -1;

    gtk_label_set_text(GTK_LABEL(cur_chat_label), DEFAULT_CUR_CHAT_LABEL);
    delete_textview_content(GTK_TEXT_VIEW(recv_msg_tv));
    notif_dialog(GTK_WINDOW(main_window), LEAVE_GROUP_SUCC_NOTIF);
    gtk_button_clicked(GTK_BUTTON(refresh_list_btn));

    g_mutex_unlock(&ui_mutex);

    is_done = 1;
    return FALSE;
}


//* ----------------------- MAIN FUNCTION -----------------------
int main(int argc, char *argv[]) {

    // init client socket
    int client_socket = connect_to_server();

    // init GTK
    gtk_init(&argc, &argv);

    // init GMutex
    g_mutex_init(&ui_mutex);

    // show login window
    show_login_window(&client_socket);

    return 0;
}
