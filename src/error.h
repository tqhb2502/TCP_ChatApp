#ifndef __ERROR_H__
#define __ERROR_H__

//* Lỗi chung
#define ERR_SOCKET_INIT -1 /* không khởi tạo được socket */
#define ERR_CREATE_THREAD -2 /* không mở tạo được thread mới */

//* Lỗi phía server
#define ERR_CONN_ACCEPT -101 /* không kết nối được */
#define ERR_OPEN_FILE -102 /* không mở được file */

//* Lỗi phía client
#define ERR_CONNECT_TO_SERVER -201 /* không kết nối được đến server */
#define ERR_INCORRECT_ACC -202 /* không mật khẩu hoặc tài khoản không chính xác */
#define ERR_SIGNED_IN_ACC -203 /* đăng nhập vào tài khoản đã được đăng nhập từ trước */
#define ERR_INVALID_RECEIVER -204 /* không tìm thấy người nhận */
#define ERR_GROUP_NOT_FOUND  -205 /* không tìm thấy group */
#define ERR_IVITE_MYSELF -206 /*mời bản thân vào nhóm */
#define ERR_USER_NOT_FOUND -207 /*mời bản thân vào nhóm */
#define ERR_FULL_MEM  -208 /*full nguoi*/
#define ERR_IS_MEM  -209 /*da la thanh vien trong nhom*/
/**
 * Hiển thị thông báo lỗi
 * @param err_code mã lỗi
*/
void report_err(int err_code);

#endif