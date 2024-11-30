#include <stdio.h>
#include "../common/common.h"

#define COLOR_BACKGROUND	FB_COLOR(0x00, 0x00 ,0x00)
#define COLOR_RED    FB_COLOR(0xff, 0x00, 0x00)
#define COLOR_ORANGE FB_COLOR(0xff, 0xa5, 0x00)
#define COLOR_YELLOW FB_COLOR(0xff, 0xff, 0x00)
#define COLOR_GREEN  FB_COLOR(0x00, 0xff, 0x00)
#define COLOR_BLUE   FB_COLOR(0x00, 0x00, 0xff)

// 手指状态结构体
typedef struct {
    int active;	// 当前手指是否点击
    int x;	// 最后的x坐标
    int y;	// 最后的y坐标
} FingerState;

static FingerState fingers[5] = {0};
static int colors[5] = {COLOR_RED, COLOR_ORANGE, COLOR_YELLOW, COLOR_GREEN, COLOR_BLUE};
static int touch_fd;

static void touch_event_cb(int fd)
{
    int type, x, y, finger;
    type = touch_read(fd, &x, &y, &finger);
    
    if (finger < 0 || finger >= 5) return;

    switch(type) {
    case TOUCH_PRESS:
        fingers[finger].active = 1;
        fingers[finger].x = x;
        fingers[finger].y = y;
        fb_draw_circle(x, y, 50, 5, colors[finger]);
        fb_update();
        break;
        
    case TOUCH_MOVE:
		// 只更新发生变化的区域
		fb_draw_circle(fingers[finger].x, fingers[finger].y, 50, 5, COLOR_BACKGROUND);
		fingers[finger].x = x;
		fingers[finger].y = y;
		fb_draw_circle(x, y, 50, 5, colors[finger]);
		fb_update();
        break;
        
    case TOUCH_RELEASE:
		fb_draw_circle(fingers[finger].x, fingers[finger].y, 50, 5, COLOR_BACKGROUND);
		fingers[finger].active = 0;
		fb_update();
        break;
        
    case TOUCH_ERROR:
        printf("close touch fd\n");
        close(fd);
        task_delete_file(fd);
        return;
    }
}

int main(int argc, char *argv[])
{
	fb_init("/dev/fb0");
	fb_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BACKGROUND);
	// fb_draw_circle(SCREEN_WIDTH, SCREEN_HEIGHT, 50, 5, COLOR_RED);
	fb_update();

	//打开多点触摸设备文件, 返回文件fd
	touch_fd = touch_init("/dev/input/event1");
	//添加任务, 当touch_fd文件可读时, 会自动调用touch_event_cb函数
	task_add_file(touch_fd, touch_event_cb);
	
	task_loop(); //进入任务循环
	return 0;
}
