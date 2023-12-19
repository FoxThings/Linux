#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/syscalls.h>

#define DEVICE_NAME "journal"
#define BUF_SIZE 1024

MODULE_LICENSE("MIT");
MODULE_AUTHOR("Zvezdin Nikita");
MODULE_DESCRIPTION("Simple journal implementation with add/get/remove commands");
MODULE_VERSION("1.0");

// -----------------------> Structures and globals <-----------------------
#define MESSAGE_BUFFER_SIZE 1024
static char* message_buffer;

#define ADD_COMMAND 0
#define REMOVE_COMMAND 1
#define GET_COMMAND 2

struct journal_entry {
    char first_name[50];
    char last_name[50];
    int age;
    char phone_number[20];
    char email[50];
    struct list_head list;
};

static LIST_HEAD(journal_list);
static int major_number;
// ------------------------------------------------------------------------



// ----------------------------> Functionality <----------------------------
// Search for a user by last name
static struct journal_entry *find_user_by_last_name(const char *last_name) {
    struct journal_entry *entry;
    list_for_each_entry(entry, &journal_list, list) {
        if (strcmp(entry->last_name, last_name) == 0) {
            return entry;
        }
    }
    return NULL;
}

// Addd new user to the journal
static long add_user(const char *first_name, const char *last_name, int age,
                     const char *phone_number, const char *email) {
    struct journal_entry *new_entry = kmalloc(sizeof(struct journal_entry), GFP_KERNEL);

    if (!new_entry) {
        pr_err("Failed to allocate memory\n");
        return -EFAULT;
    }

    strncpy(new_entry->first_name, first_name, sizeof(new_entry->first_name) - 1);
    strncpy(new_entry->last_name, last_name, sizeof(new_entry->last_name) - 1);
    new_entry->age = age;
    strncpy(new_entry->phone_number, phone_number, sizeof(new_entry->phone_number) - 1);
    strncpy(new_entry->email, email, sizeof(new_entry->email) - 1);

    INIT_LIST_HEAD(&new_entry->list);
    list_add(&new_entry->list, &journal_list);

    pr_info("User added: %s %s\n", first_name, last_name);

    return 0;
}

// Remove a user from the journal
static void remove_user(struct journal_entry *entry) {
    pr_info("User removed: %s %s\n", entry->first_name, entry->last_name);
    list_del(&entry->list);
    kfree(entry);
}

static int get_command_from_presentation(const char* command) {
    return (
        (strncmp(command, "ADD", 3) == 0) ? ADD_COMMAND :
        (strncmp(command, "REMOVE", 6) == 0) ? REMOVE_COMMAND :
        (strncmp(command, "GET", 3) == 0) ? GET_COMMAND : -1
    );
}
// -------------------------------------------------------------------------



// ----------------------------> File interface <----------------------------
static int journal_open(struct inode *inode, struct file *file) {
    pr_info("journal device opened\n");
    return 0;
}

static ssize_t journal_read(struct file *file, char __user *user_buffer, size_t length, loff_t *offset) {
    if (*offset >= MESSAGE_BUFFER_SIZE) {
        return 0;
    }


    size_t actual_length = min(length, (size_t)(MESSAGE_BUFFER_SIZE - *offset));

    if (copy_to_user(user_buffer, message_buffer + *offset, actual_length)) {
        return -EFAULT;
    }

    *offset += actual_length;

    return actual_length;
}

static ssize_t journal_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset) {
    char *input_buffer = kmalloc(count, GFP_KERNEL);

    if (!input_buffer) {
        pr_err("Failed to allocate memory\n");
        return -ENOMEM;
    }

    if (copy_from_user(input_buffer, user_buffer, count)) {
        kfree(input_buffer);
        return -EFAULT;
    }

    int command = get_command_from_presentation(input_buffer);
    switch(command) {
        case ADD_COMMAND: {
            char first_name[50], last_name[50], phone_number[20], email[50];
            int age;
            sscanf(input_buffer, "ADD %s %s %d %s %s", first_name, last_name, &age, phone_number, email);
            add_user(first_name, last_name, age, phone_number, email);
            break;
        }
        case REMOVE_COMMAND: {
            char last_name[50];
            sscanf(input_buffer, "REMOVE %s", last_name);
            struct journal_entry* entry = find_user_by_last_name(last_name);
            if (entry) {
                remove_user(entry);
            } else {
                pr_info("User with last name %s not found\n", last_name);
            }
            break;
        }
        case GET_COMMAND: {
            char last_name[50];
            sscanf(input_buffer, "GET %s", last_name);
            struct journal_entry* entry = find_user_by_last_name(last_name);

            if (entry) {
                memset(message_buffer, 0, MESSAGE_BUFFER_SIZE);
                snprintf(message_buffer, MESSAGE_BUFFER_SIZE,
                    "Name: %s %s\nAge: %d\nPhone: %s\nEmail: %s\n\n",
                    entry->first_name, entry->last_name, entry->age,
                    entry->phone_number, entry->email);
            } else {
                memset(message_buffer, 0, MESSAGE_BUFFER_SIZE);
                snprintf(message_buffer, MESSAGE_BUFFER_SIZE, "User not found\n");
            }
            break;
        }
        default: {
            pr_info("Command not found");
        }
    }

    kfree(input_buffer);
    return count;
}

static int journal_release(struct inode *inode, struct file *file) {
    pr_info("journal device closed\n");
    return 0;
}

// File operations description structure
static const struct file_operations journal_fops = {
    .owner = THIS_MODULE,
    .open = journal_open,
    .read = journal_read,
    .write = journal_write,
    .release = journal_release,
};
// --------------------------------------------------------------------------



// --------------------------> Module life events <--------------------------
static int __init journal_init(void) {
    message_buffer = kmalloc(MESSAGE_BUFFER_SIZE, GFP_KERNEL);
    major_number = register_chrdev(0, DEVICE_NAME, &journal_fops);

    if (major_number < 0) {
        pr_err("Failed to register a major number\n");
        return major_number;
    }

    pr_info("journal module loaded with major number %d\n", major_number);
    return 0;
}

static void __exit journal_exit(void) {
    kfree(message_buffer);
    struct journal_entry *entry, *tmp;

    list_for_each_entry_safe(entry, tmp, &journal_list, list) {
        list_del(&entry->list);
        kfree(entry);
    }

    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("journal module unloaded\n");
}

module_init(journal_init);
module_exit(journal_exit);
// --------------------------------------------------------------------------
