#include "bluetooth.h"
#include "assert.h"

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

static int bluetooth_tty_init(const char *dev)
{
    int fd = open(dev, O_RDWR|O_NOCTTY|O_NONBLOCK); /*非阻塞模式*/
    if(fd < 0){
        printf("bluetooth_tty_init open %s error(%d): %s\n", dev, errno, strerror(errno));
        return -1;
    }
    return fd;
}

int bluetooth_init(void) {
    // bluetooth_fd = bluetooth_tty_init("/dev/rfcomm0");
    bluetooth_fd = 1;
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
        myWrite_nonblock(bluetooth_fd, buf, packet_size);
    #endif

    printf("send: x=%2d, y=%2d, packet_size = %d\n", x, y, packet_size);
    assert(packet_size == PACKET_SIZE);
    return 0;
}

int bluetooth_receive_move(TouchResult *move) {
    char buf[64];
    int actual_size = 0;

    #ifdef DEBUG
        if (debug_move_index < debug_moves_count) {
            move->board_x = debug_moves[debug_move_index].x;
            move->board_y = debug_moves[debug_move_index].y;
            sprintf(buf, "%2d %2d", move->board_x, move->board_y);
            actual_size = PACKET_SIZE;
            debug_move_index++;
        } else {
            printf("no input (debug sequence ended)\n");
            return 0;
        }
    #else
        actual_size = myRead_nonblock(bluetooth_fd, buf, PACKET_SIZE);
        if(actual_size == 0) {
            printf("no input\n");
            return actual_size;
        }
        int x, y;
        sscanf(buf, "%d %d", &x, &y);
        move->board_x = x;
        move->board_y = y;
    #endif

    printf("receive: x=%2d, y=%2d, packet_size = %d\n", move->board_x, move->board_y, actual_size);
    assert(actual_size == PACKET_SIZE);
    return actual_size;
}

void bluetooth_close(void) {
    close(bluetooth_fd);
}