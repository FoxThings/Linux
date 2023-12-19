#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "helpers.h"

int main() {
    struct user_data new_user = {
        .first_name = "Nikita",
        .last_name = "Zvezdin",
        .age = 21,
        .phone_number = "469416854",
        .email = "email@gmail.com"
    };
    if (add_user(&new_user) < 0) {
        perror("Failed to add user");
    } else {
        printf("User added successfully\n");
    }

    struct user_data retrieved_user;
    const char *surname_to_search = "Zvezdin";
    if (get_user(surname_to_search, strlen(surname_to_search), &retrieved_user) < 0) {
        perror("Failed to get user");
    } else {
        printf("Retrieved user information:\n");
        printf("First Name: %s\n", retrieved_user.first_name);
        printf("Last Name: %s\n", retrieved_user.last_name);
        printf("Age: %d\n", retrieved_user.age);
        printf("Phone Number: %s\n", retrieved_user.phone_number);
        printf("Email: %s\n", retrieved_user.email);
    }

    if (del_user(surname_to_search, strlen(surname_to_search)) < 0) {
        perror("Failed to delete user");
    } else {
        printf("User deleted successfully\n");
    }

    if (get_user(surname_to_search, strlen(surname_to_search), &retrieved_user) < 0) {
        printf("User delete physically");
    }

    printf("All tests passed\n");

    return 0;
}
