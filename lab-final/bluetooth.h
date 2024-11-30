// bluetooth.h - 蓝牙通信
#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

typedef struct {
    int from_x, from_y;
    int to_x, to_y;
} ChessMove;

void bluetooth_init(void);
bool bluetooth_is_connected(void);
bool bluetooth_send_move(ChessMove *move);
bool bluetooth_receive_move(ChessMove *move);
void bluetooth_close(void);

#endif