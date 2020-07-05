#include <stdio.h>
#include <termios.h>

typedef enum {
    kDataBits5 = 0,
    kDataBits6,
    kDataBits7,
    kDataBits8,
} SerialDatabits_t;

typedef enum {
    kStopbits1 = 0,
    kStopbits2,
} SerialStopbits_t;

typedef enum {
    kParityNone = 0,
    kParityEven,
    kParityOdd,
} SerialParity_t;

typedef struct {
    speed_t baudrate;
    SerialDatabits_t databits;
    SerialStopbits_t stopbits;
    SerialParity_t parity;
} SerialConfig_t;

/**
 * @brief 设置串口通信速率
 *
 * @param fd file descriptor
 * @param rate
 * @param databits
 * @param stopbits
 * @param parity
 *
 * @return 0 if success, -1 if failed
 */
int serial_set_param(int fd, speed_t rate, SerialDatabits_t databits,
    SerialStopbits_t stopbits, SerialParity_t parity);

/**
 * @brief 将数字形式的波特率转换为 termios 可识别风格
 *
 * @param baudrate
 *
 * @return 
 */
speed_t serial_parse_baudrate(int baudrate);

