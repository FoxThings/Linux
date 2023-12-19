#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "helpers.h"

#define DEVICE_PATH "/dev/journal"

long get_user(const char* surname, unsigned int len, struct user_data* output_data) {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd == -1) {
        perror("Failed to open the device for reading");
        return -1;
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "GET %s", surname);

    ssize_t bytes_written = write(fd, buffer, strlen(buffer));
    if (bytes_written == -1) {
        perror("Failed to write to the device");
        close(fd);
        return -1;
    }

    FILE* fp = fdopen(fd, "r");
    int status = fscanf(fp, 
        "Name: %s %s\nAge: %d\nPhone: %s\nEmail: %s\n",
        output_data->first_name, 
        output_data->last_name, 
        &output_data->age,
        output_data->phone_number, 
        output_data->email
    );
    fclose(fp);
    close(fd);
    return 0;
}

long add_user(struct user_data* input_data) {
    int fd = open(DEVICE_PATH, O_WRONLY);
    if (fd == -1) {
        perror("Failed to open the device for writing");
        return -1;
    }

    char buffer[256] = "";
    int len = snprintf(
        buffer,
        sizeof(buffer),
        "ADD %s %s %d %s %s", 
        input_data->first_name, 
        input_data->last_name, 
        input_data->age,
        input_data->phone_number, 
        input_data->email
    );

    ssize_t bytes_written = write(fd, buffer, len);
    if (bytes_written == -1) {
        perror("Failed to write to the device");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

long del_user(const char* surname, unsigned int len) {
    int fd = open(DEVICE_PATH, O_WRONLY);
    if (fd == -1) {
        perror("Failed to open the device for writing");
        return -1;
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "REMOVE %s", surname);

    ssize_t bytes_written = write(fd, buffer, strlen(buffer));
    if (bytes_written == -1) {
        perror("Failed to write to the device");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}
