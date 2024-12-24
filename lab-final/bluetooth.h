// bluetooth.h - 蓝牙通信
#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

#include "ui.h"

extern int bluetooth_fd;

int bluetooth_init(void);
int bluetooth_is_connected(void);
int bluetooth_send_move(TouchResult *move);
int bluetooth_receive_move(TouchResult *move);
void bluetooth_close(void);

#endif