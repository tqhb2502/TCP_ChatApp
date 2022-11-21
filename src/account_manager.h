#ifndef __ACCOUNT_MANAGER_H__
#define __ACCOUNT_MANAGER_H__

#define USERNAME_SIZE 256
#define PASSWORD_SIZE 256
#define MAX_CONSECUTIVE_FAIL 3

typedef struct Account_ {
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    int status;
    int consecutive_failed_sign_in;
    int is_signed_in;
    struct Account_ *next;
} Account;

void write_to_file(Account *list);
Account* read_account_list();
Account* find_account(Account *list, char *username);
int is_active_account(Account *list, char *username);

#endif