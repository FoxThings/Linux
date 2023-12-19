// helpers.h

#ifndef JOURNAL_HELPERS_H
#define JOURNAL_HELPERS_H

struct user_data {
    char first_name[50];
    char last_name[50];
    int age;
    char phone_number[20];
    char email[50];
};

long get_user(const char* surname, unsigned int len, struct user_data* output_data);
long add_user(struct user_data* input_data);
long del_user(const char* surname, unsigned int len);

#endif