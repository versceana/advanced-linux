#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE "/dev/int_stack"
#define IOCTL_SET_SIZE 0

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }

    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT || errno == ENODEV) {
            fprintf(stderr, "error: USB key not inserted\n");
            return 1;
        }
        perror("open");
        return 1;
    }

    if (strcmp(argv[1], "set-size") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s set-size <size>\n", argv[0]);
            close(fd);
            return 1;
        }
        int size = atoi(argv[2]);
        if (ioctl(fd, IOCTL_SET_SIZE, &size) < 0) {
            perror("ioctl");
            close(fd);
            return 1;
        }
    } else if (strcmp(argv[1], "push") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s push <value>\n", argv[0]);
            close(fd);
            return 1;
        }
        int value = atoi(argv[2]);
        ssize_t ret = write(fd, &value, sizeof(int));
        if (ret < 0) {
            if (errno == ERANGE) {
                fprintf(stderr, "ERROR: stack is full\n");
                close(fd);
                return 34;
            }
            perror("write");
            close(fd);
            return 1;
        }
    } else if (strcmp(argv[1], "pop") == 0) {
        int value;
        ssize_t ret = read(fd, &value, sizeof(int));
        if (ret == 0) {
            printf("NULL\n");
        } else if (ret < 0) {
            perror("read");
            close(fd);
            return 1;
        } else {
            printf("%d\n", value);
        }
    } else if (strcmp(argv[1], "unwind") == 0) {
        int value;
        while (1) {
            ssize_t ret = read(fd, &value, sizeof(int));
            if (ret == 0) {
                printf("NULL\n");
                break;
            } else if (ret < 0) {
                perror("read");
                close(fd);
                return 1;
            } else {
                printf("%d\n", value);
            }
        }
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
