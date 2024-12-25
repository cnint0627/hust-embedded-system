#include "bluetooth.h"
#include "assert.h"
#include <termios.h>
#include <fcntl.h>

int bluetooth_fd = -1;
#define PACKET_SIZE 6

// #define DEBUG

// 添加调试相关定义
#ifdef DEBUG
    // 预定义的移动序列
    static const struct {
        int x;
        int y;
    } debug_moves[] = {
        {4, 6},
        {4, 5},
        {2, 6},
        {2, 5},
        {0, 6},
        {0, 5},
    };
    static int debug_move_index = 0;
    static const int debug_moves_count = sizeof(debug_moves) / sizeof(debug_moves[0]);
#endif


ssize_t read_blocking(int fd, char *buf, size_t size) {
    size_t total_read = 0; // 已读取字节数
    while (total_read < size) {
        ssize_t n = read(fd, buf + total_read, size - total_read);

        if (n > 0) {
            total_read += n; // 更新已读取字节数
        } else if (n == 0) {
            // 没有数据可读，继续轮询
            continue;
        } else if (n == -1) {
            if (errno == EINTR) {
                // 读取被信号中断，继续尝试
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 非阻塞模式下无数据可读，继续轮询
                continue;
            } else {
                // 发生其他错误，返回错误码
                perror("read_blocking error");
                return -1;
            }
        }
    }
    return total_read; // 返回总读取字节数
}

ssize_t read_nonblocking(int fd, char *buf, size_t size) {
    ssize_t n = read(fd, buf, size);

    if (n > 0) {
        // 成功读取数据，返回读取的字节数
        return n;
    } else if (n == 0) {
        // 文件末尾，无更多数据
        return 0;
    } else if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 非阻塞模式下无数据可读
            return 0;
        } else if (errno == EINTR) {
            // 读取被信号中断，返回 0 表示暂时无数据
            return 0;
        } else {
            // 发生其他错误，打印错误信息并返回 -1
            perror("read_nonblocking error");
            return -1;
        }
    }

    return 0; // 默认返回 0，作为防御性代码（通常不会执行到这里）
}


ssize_t write_blocking(int fd, const char *buf, size_t size) {
    size_t total_written = 0; // 已写入字节数
    while (total_written < size) {
        ssize_t written = write(fd, buf + total_written, size - total_written);

        if (written > 0) {
            total_written += written; // 更新已写入字节数
        } else if (written == -1) {
            if (errno == EINTR) {
                // 写入被信号中断，继续尝试
                continue;
            } else {
                // 发生其他错误，返回错误码
                perror("write_blocking error");
                return -1;
            }
        }
    }
    return total_written; // 返回总写入字节数
}

static int bluetooth_tty_init(const char *dev)
{
    int fd = open(dev, O_RDWR|O_NOCTTY|O_NONBLOCK); /*非阻塞模式*/
    if(fd < 0){
        printf("bluetooth_tty_init open %s error(%d): %s\n", dev, errno, strerror(errno));
        return -1;
    }

    if (tcflush(fd, TCIOFLUSH) == -1) {
        perror("tcflush error");
        return -1;
    }
    return fd;
}

int bluetooth_init(void) {
    printf("bluetooth_init\n");
    bluetooth_fd = bluetooth_tty_init("/dev/rfcomm0");
    // bluetooth_fd = 1;
    if(bluetooth_fd == -1) return 0;
    #ifdef DEBUG
        debug_move_index = 0;  // 初始化调试序列索引
    #endif
    return bluetooth_fd;
}

int bluetooth_is_connected(void) {
    // TODO
    return bluetooth_fd != -1;
}

int bluetooth_send_move(TouchResult *move) {
    int x = move->board_x;
    int y = move->board_y;
    char buf[64];
    // 固定长度 10 5\n
    sprintf(buf, "%2d %2d\n", x, y);
    int packet_size = strlen(buf);
    #ifndef DEBUG
        // myWrite_nonblock(bluetooth_fd, buf, packet_size);
        write_blocking(bluetooth_fd, buf, packet_size);
    #endif

    printf("send: x=%2d, y=%2d, packet_size = %d\n", x, y, packet_size);
    assert(packet_size == PACKET_SIZE);
    return 0;
}

// int bluetooth_receive_move(TouchResult *move) {
//     char buf[64] = {0};
//     int actual_size = 0;

//     #ifdef DEBUG
//         if (debug_move_index < debug_moves_count) {
//             move->board_x = debug_moves[debug_move_index].x;
//             move->board_y = debug_moves[debug_move_index].y;
//             sprintf(buf, "%2d %2d", move->board_x, move->board_y);
//             actual_size = PACKET_SIZE;
//             debug_move_index++;
//         } else {
//             printf("no input (debug sequence ended)\n");
//             return 0;
//         }
//     #else
//         // actual_size = myRead_nonblock(bluetooth_fd, buf, PACKET_SIZE);
//         // actual_size = read_blocking(bluetooth_fd, buf, PACKET_SIZE);
//         // printf("read_nonblocking: ");
//         actual_size = read_nonblocking(bluetooth_fd, buf, PACKET_SIZE);
//         if(actual_size == 0) {
//             printf("no input\n");
//             return 0;
//         }
//         if (actual_size != PACKET_SIZE) {
//             printf("actual_size err: %d\n", actual_size);
//             return 0;
//         }
//         int x, y;
//         sscanf(buf, "%d %d", &x, &y);
//         move->board_x = x;
//         move->board_y = y;
//     #endif

//     printf("receive: x=%2d, y=%2d, actual_size = %d\n", move->board_x, move->board_y, actual_size);
//     // assert(actual_size == PACKET_SIZE);

//     return actual_size;
// }

int bluetooth_receive_move(TouchResult *move) {
    char buf[64] = {0};
    char *cur_pos = buf;
    int remaining = sizeof(buf) - 1;  // 保留一个字节给\0
    int total = 0;

    // 读取直到遇到换行符或缓冲区满
    while (remaining > 0) {
        int n = read(bluetooth_fd, cur_pos, remaining);

        if (n < 0) {
            if (errno == EINTR) {
                continue;  // 被信号中断，继续读取
            }
            // perror("read error");
            return 0;
        }

        if (n == 0) {  // 无数据可读
            if (total == 0) {
                printf("no input\n");
                return 0;
            }
            break;
        }

        // 查找换行符
        char *newline = memchr(cur_pos, '\n', n);
        if (newline) {
            total += (newline - cur_pos + 1);
            *newline = '\0';  // 替换换行符为字符串结束符

            // 解析坐标
            int x, y;
            if (sscanf(buf, "%d %d", &x, &y) == 2) {
                move->board_x = x;
                move->board_y = y;
                move->valid = true;
                printf("receive: x=%2d, y=%2d, total=%d\n", x, y, total);
                return total;
            } else {
                printf("parse error: %s\n", buf);
                return 0;
            }
        }

        // 更新位置和剩余空间
        cur_pos += n;
        remaining -= n;
        total += n;

        if (remaining == 0) {
            printf("buffer overflow\n");
            return 0;
        }
    }

    printf("incomplete data\n");
    return 0;
}

void bluetooth_close(void) {
    close(bluetooth_fd);
}