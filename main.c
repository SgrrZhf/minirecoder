#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "serial.h"

#define PACKAGE "minirecoder"

static void serial_read_c(const char *port, int baudrate, bool timestamp_enable, FILE *capture_file);
static int create_file(const char *path);
static void helpthen(const char *name);
static size_t get_timestamp(char *dst, size_t dst_size);
static sigjmp_buf jmpbuf;

void signal_handler(int sig)
{
    siglongjmp(jmpbuf, 1);
    exit(0);
}

int main(int argc, char *argv[])
{
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        printf("signal(%d) error, reason %s\n", SIGINT, strerror(errno));
        exit(0);
    }
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        printf("signal(%d) error, reason %s\n", SIGKILL, strerror(errno));
        exit(0);
    }

    int baudrate = 115200;
    FILE *capture_file = NULL;
    bool timestamp_enable = false;
    static struct option long_option[] = {
        { "baudrate", required_argument, NULL, 'b' },
        { "capturefile", required_argument, NULL, 's' },
        { "timestamp", no_argument, NULL, 't' },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 },
    };
    int c;
    while ((c = getopt_long(argc, argv, ":htb:C:", long_option, NULL)) != -1) {
        switch (c) {
        case 'h':
            helpthen(argv[0]);
            exit(1);
            break;
        case 'b':
            if ((baudrate = atoi(optarg)) == 0) {
                fprintf(stderr, "%s: option '-%c requires number'\n", argv[0], optopt);
                exit(1);
            }
            break;
        case 'C':
            if ((capture_file = fopen(optarg, "w")) == NULL) {
                if (errno != 2) {
                    fprintf(stderr, "open %s failed, errno: %d, reason: %s\n", optarg, errno, strerror(errno));
                    exit(1);
                }

                if (create_file(optarg) == 0) {
                    capture_file = fopen(optarg, "w");
                    assert(capture_file);
                }
            }
            break;
        case 't':
            timestamp_enable = true;
            break;
        case ':':
            fprintf(stderr, "%s: option '-%c' requires an argument\n", argv[0], optopt);
            break;
        case '?':
        default:
            fprintf(stderr, "%s: option '-%c' is invalid: ignord\n", argv[0], optopt);
            break;
        }
    }

    const char *port = argv[argc - 1];
    if (argc == 1 || port == NULL || port[0] == '-') {
        fprintf(stderr, "Unknow port\n");
        exit(1);
    }

    if (sigsetjmp(jmpbuf, 1) == 0) {
        serial_read_c(port, baudrate, timestamp_enable, capture_file);
    } else {
        if (capture_file) {
            fclose(capture_file);
        }
    }
    printf("Exit...\n");
    return 0;
}

static void print(FILE *fd, const char *format, ...)
{
    va_list args1;
    va_start(args1, format);
    vfprintf(stdout, format, args1);
    va_end(args1);
    if (fd) {
        va_start(args1, format);
        vfprintf(fd, format, args1);
        va_end(args1);
    }
}

static void serial_read_c(const char *port, int baudrate, bool timestamp_enable, FILE *capture_file)
{
    speed_t rate = serial_parse_baudrate(baudrate);
    if (rate == 0) {
        fprintf(stderr, "baudrate %d is not supportted\n", baudrate);
        return;
    }

    int fs = open(port, O_RDONLY);
    if (fs == -1) {
        fprintf(stderr, "Open %s failed, reason: %s\n", port, strerror(errno));
        return;
    }

    serial_set_param(fs, rate, kDataBits8, kStopbits1, kParityNone);

    fd_set read_set, ready_set;
    char buff[1024];
    while (true) {
        FD_ZERO(&read_set);
        FD_SET(fs, &read_set);

        char last_c = '\n';
        while (true) {
            int ret_size;
            ready_set = read_set;
            int ret = select(fs + 1, &ready_set, NULL, NULL, NULL);
            if (FD_ISSET(fs, &ready_set)) {
                ret_size = read(fs, buff, sizeof(buff));
                if (ret_size == 0) {
                    break;
                }

                if (timestamp_enable && last_c == '\n') {
                    char buf[32];
                    get_timestamp(buf, sizeof(buf));
                    print(capture_file, "[%s] ", buf);
                }
                print(capture_file, "%.*s", ret_size, buff);

                last_c = buff[ret_size - 1];
            }
        }
        close(fs);

        printf("try to reopen %s\n", port);
        while ((fs = open(port, O_RDONLY)) == -1) {
            sleep(1);
        }
        serial_set_param(fs, rate, kDataBits8, kStopbits1, kParityNone);
    }
}

static int create_dir_recursive(const char *path)
{
    if (mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0) {
        return 0;
    }
    if (errno != 2) {
        fprintf(stderr, "create_dir error(%d),reason: %s", errno, strerror(errno));
        return -1;
    }
    return -1;
}

static int create_file(const char *path)
{
    int ret = 0;
    if (access(path, W_OK) == 0) {
        return 0;
    }

    if (errno != 2) {
        fprintf(stderr, "create_file error, errno: %d, reason: %s\n", errno, strerror(errno));
        return -1;
    }

    int dir_len = strlen(path);
    char *dir = malloc(dir_len + 1);
    memcpy(dir, path, strlen(path));
    dir[dir_len] = '\0';
    char *p = strrchr(dir, '/');
    if (p == NULL) {
        ret = -1;
        goto _clean;
    }
    *p = '\0';
    ret = create_dir_recursive(dir);
_clean:
    free(dir);
    return ret;
}

static size_t get_timestamp(char *dst, size_t dst_size)
{
    time_t timep = time(NULL);
    struct tm *p = localtime(&timep);
    return strftime(dst, dst_size, "%F %T", p);
}

static void helpthen(const char *name)
{
    printf("Usage: %s [OPTION]...<tty port device>\n"
           "A serial recoder for Linux and other unix-like system.\n\n"
           "-b --baudrate : set baudrate\n"
           "-C --capturefile=FILE : start capturing to FILE\n"
           "-t --timestame : enable timestame\n"
           "-h --help : show help\n",
        name);
}
