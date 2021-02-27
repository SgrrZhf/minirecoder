#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "serial.h"

int serial_set_param(int fd, speed_t rate, SerialDatabits_t databits,
    SerialStopbits_t stopbits, SerialParity_t parity)
{
    struct termios opt;
    tcgetattr(fd, &opt);
    tcflush(fd, TCIOFLUSH);

    cfsetispeed(&opt, rate);
    cfsetospeed(&opt, rate);
    if (tcsetattr(fd, TCSANOW, &opt) != 0) {
        return -1;
    }

    opt.c_cflag &= ~CSIZE;
    switch (databits) {
    case kDataBits5:
        opt.c_cflag |= CS5;
        break;
    case kDataBits6:
        opt.c_cflag |= CS6;
        break;
    case kDataBits7:
        opt.c_cflag |= CS7;
        break;
    case kDataBits8:
        opt.c_cflag |= CS8;
        break;
    }

    switch (stopbits) {
    case kStopbits1:
        opt.c_cflag &= ~CSTOPB;
        break;
    case kStopbits2:
        opt.c_cflag |= CSTOPB;
        break;
    }

    switch (parity) {
    case kParityNone:
        opt.c_cflag &= ~PARENB;
        opt.c_iflag &= ~INPCK;
        break;
    case kParityOdd:
        opt.c_cflag |= (PARODD | PARENB);
        opt.c_iflag |= INPCK;
        break;
    case kParityEven:
        opt.c_cflag |= PARENB;
        opt.c_cflag &= ~PARODD;
        opt.c_iflag |= INPCK;
        break;
    }

    opt.c_iflag &= ~ICRNL; //必须要有，否则会隔行打印
    opt.c_cc[VMIN] = 0; //阻塞式读取
    opt.c_cc[VTIME] = 0; //没有数据就一直阻塞

    if (tcsetattr(fd, TCSANOW, &opt) != 0) {
        return -1;
    }
    tcflush(fd, TCIOFLUSH);
    return 0;
}

const static int name_arr[] = {
    50,
    75,
    110,
    134,
    150,
    200,
    300,
    600,
    1200,
    1800,
    2400,
    4800,
    9600,
    19200,
    38400,
    57600,
    115200,
    230400,
    460800,
    500000,
    576000,
    921600,
    1000000,
    1152000,
    1500000,
    2000000,
    250000,
    3000000,
    35000000,
    4000000,
};

const static speed_t baudrate_arr[] = {
    B50,
    B75,
    B110,
    B134,
    B150,
    B200,
    B300,
    B600,
    B1200,
    B1800,
    B2400,
    B4800,
    B9600,
    B19200,
    B38400,
    B57600,
    B115200,
    B230400,
    B460800,
    B500000,
    B576000,
    B921600,
    B1000000,
    B1152000,
    B1500000,
    B2000000,
    B2500000,
    B3000000,
    B3500000,
    B4000000,
};

speed_t serial_parse_baudrate(int baudrate)
{
    for (size_t i = 0; i < sizeof(name_arr) / sizeof(name_arr[0]); i++) {
        if (baudrate == name_arr[i]) {
            return baudrate_arr[i];
        }
    }
    return 0;
}
