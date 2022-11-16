#ifndef __ERROR_H__
#define __ERROR_H__

#define ERR_SOCKET_INIT -1 /* không khởi tạo được socket */
#define ERR_CONN_ACCEPT -2 /* không kết nối được */
#define ERR_OPEN_FILE -3 /* không mở được file */

/**
 * Hiển thị thông báo lỗi
 * @param err_code mã lỗi
*/
void report_err(int err_code);

#endif