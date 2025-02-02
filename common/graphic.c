#include "common.h"
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include "math.h"

static int LCD_FB_FD;
static int *LCD_FB_BUF = NULL;
static int DRAW_BUF[SCREEN_WIDTH*SCREEN_HEIGHT];

static struct area {
	int x1, x2, y1, y2;
} update_area = {0,0,0,0};

#define AREA_SET_EMPTY(pa) do {\
	(pa)->x1 = SCREEN_WIDTH;\
	(pa)->x2 = 0;\
	(pa)->y1 = SCREEN_HEIGHT;\
	(pa)->y2 = 0;\
} while(0)

void fb_init(char *dev)
{
	// 先清空终端输出，以免对图形渲染造成干扰
	system("clear");

	int fd;
	struct fb_fix_screeninfo fb_fix;
	struct fb_var_screeninfo fb_var;

	if(LCD_FB_BUF != NULL) return; /*already done*/

	//进入终端图形模式
	fd = open("/dev/tty0",O_RDWR,0);
	ioctl(fd, KDSETMODE, KD_GRAPHICS);
	close(fd);

	//First: Open the device
	if((fd = open(dev, O_RDWR)) < 0){
		printf("Unable to open framebuffer %s, errno = %d\n", dev, errno);
		return;
	}
	if(ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix) < 0){
		printf("Unable to FBIOGET_FSCREENINFO %s\n", dev);
		return;
	}
	if(ioctl(fd, FBIOGET_VSCREENINFO, &fb_var) < 0){
		printf("Unable to FBIOGET_VSCREENINFO %s\n", dev);
		return;
	}

	printf("framebuffer info: bits_per_pixel=%u,size=(%d,%d),virtual_pos_size=(%d,%d)(%d,%d),line_length=%u,smem_len=%u\n",
		fb_var.bits_per_pixel, fb_var.xres, fb_var.yres, fb_var.xoffset, fb_var.yoffset,
		fb_var.xres_virtual, fb_var.yres_virtual, fb_fix.line_length, fb_fix.smem_len);

	//Second: mmap
	void *addr = mmap(NULL, fb_fix.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(addr == (void *)-1){
		printf("failed to mmap memory for framebuffer.\n");
		return;
	}

	if((fb_var.xoffset != 0) ||(fb_var.yoffset != 0))
	{
		fb_var.xoffset = 0;
		fb_var.yoffset = 0;
		if(ioctl(fd, FBIOPAN_DISPLAY, &fb_var) < 0) {
			printf("FBIOPAN_DISPLAY framebuffer failed\n");
		}
	}

	LCD_FB_FD = fd;
	LCD_FB_BUF = addr;

	//set empty
	AREA_SET_EMPTY(&update_area);
	return;
}

static void _copy_area(int *dst, int *src, struct area *pa)
{
	int x, y, w, h;
	x = pa->x1; w = pa->x2-x;
	y = pa->y1; h = pa->y2-y;
	src += y*SCREEN_WIDTH + x;
	dst += y*SCREEN_WIDTH + x;
	while(h-- > 0){
		memcpy(dst, src, w*4);
		src += SCREEN_WIDTH;
		dst += SCREEN_WIDTH;
	}
}

static int _check_area(struct area *pa)
{
	if(pa->x2 == 0) return 0; //is empty

	if(pa->x1 < 0) pa->x1 = 0;
	if(pa->x2 > SCREEN_WIDTH) pa->x2 = SCREEN_WIDTH;
	if(pa->y1 < 0) pa->y1 = 0;
	if(pa->y2 > SCREEN_HEIGHT) pa->y2 = SCREEN_HEIGHT;

	if((pa->x2 > pa->x1) && (pa->y2 > pa->y1))
		return 1; //no empty

	//set empty
	AREA_SET_EMPTY(pa);
	return 0;
}

void fb_update(void)
{
	if(_check_area(&update_area) == 0) return; //is empty
	_copy_area(LCD_FB_BUF, DRAW_BUF, &update_area);
	AREA_SET_EMPTY(&update_area); //set empty
	return;
}

/*======================================================================*/

static void * _begin_draw(int x, int y, int w, int h)
{
	int x2 = x+w;
	int y2 = y+h;
	if(update_area.x1 > x) update_area.x1 = x;
	if(update_area.y1 > y) update_area.y1 = y;
	if(update_area.x2 < x2) update_area.x2 = x2;
	if(update_area.y2 < y2) update_area.y2 = y2;
	return DRAW_BUF;
}

void fb_draw_pixel(int x, int y, int color)
{
	if(x<0 || y<0 || x>=SCREEN_WIDTH || y>=SCREEN_HEIGHT) return;
	int *buf = _begin_draw(x,y,1,1);
/*---------------------------------------------------*/
	*(buf + y*SCREEN_WIDTH + x) = color;
/*---------------------------------------------------*/
	return;
}

void fb_draw_rect(int x, int y, int w, int h, int color)
{
	if(x < 0) { w += x; x = 0;}
	if(x+w > SCREEN_WIDTH) { w = SCREEN_WIDTH-x;}
	if(y < 0) { h += y; y = 0;}
	if(y+h >SCREEN_HEIGHT) { h = SCREEN_HEIGHT-y;}
	if(w<=0 || h<=0) return;
	int *buf = _begin_draw(x,y,w,h);
/*---------------------------------------------------*/
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            *(buf + (y + i) * SCREEN_WIDTH + (x + j)) = color;
        }
    }
/*---------------------------------------------------*/
	return;
}

void fb_draw_line(int x1, int y1, int x2, int y2, int color)
{
/*---------------------------------------------------*/
    int x_min = x1 < x2 ? x1 : x2;
    int y_min = y1 < y2 ? y1 : y2;
    int x_max = x1 > x2 ? x1 : x2;
    int y_max = y1 > y2 ? y1 : y2;
    int *buf = _begin_draw(x_min, y_min, x_max - x_min, y_max - y_min);

	// x、y的总变化量
	float dx = x2 - x1;
    float dy = y2 - y1;
	// 计算步数，取变化量较大者
    float steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    // x、y每步的增量
    float x_inc = dx / steps;
    float y_inc = dy / steps;

    float x = x1;
    float y = y1;

    for(int i = 0; i <= steps; i++) {
        *(buf + (int)y * SCREEN_WIDTH + (int)x) = color;
		x += x_inc;
    	y += y_inc;
    }
/*---------------------------------------------------*/
	return;
}

void fb_draw_image(int x, int y, fb_image *image, int color)
{
	if(image == NULL) return;

	int ix = 0; //image x
	int iy = 0; //image y
	int w = image->pixel_w; //draw width
	int h = image->pixel_h; //draw height

	if(x<0) {w+=x; ix-=x; x=0;}
	if(y<0) {h+=y; iy-=y; y=0;}

	if(x+w > SCREEN_WIDTH) {
		w = SCREEN_WIDTH - x;
	}
	if(y+h > SCREEN_HEIGHT) {
		h = SCREEN_HEIGHT - y;
	}
	if((w <= 0)||(h <= 0)) return;

	int *buf = _begin_draw(x,y,w,h);
/*---------------------------------------------------------------*/
	char *dst = (char *)(buf + y*SCREEN_WIDTH + x);
	char *src; //不同的图像颜色格式定位不同
/*---------------------------------------------------------------*/

	int alpha;
	int ww;

	if(image->color_type == FB_COLOR_RGB_8880) /*lab3: jpg*/
	{
		src = image->content + iy * w * 4 + ix * 4;
		for (int i = 0; i < h; i++) {
			// for (int j = 0; j < w; j++) {
			// 	int* src_ = src + i * w * 4 + j * 4;
			// 	int* dst_ = dst + i * SCREEN_WIDTH * 4 + j * 4;
			// 	*dst_ = *src_;
			// }
			memcpy(dst + i * SCREEN_WIDTH * 4, src + i * w * 4, w * 4);
		}
	}
	else if(image->color_type == FB_COLOR_RGBA_8888) /*lab3: png*/
	{
		src = image->content + iy * w * 4 + ix * 4;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				char* src_ = src + i * w * 4 + j * 4;	// B G R A
				char* dst_ = dst + i * SCREEN_WIDTH * 4 + j * 4;
				alpha = src_[3];
				switch (alpha) {
					case 0:
						break;
					case 255:
						dst_[0] = src_[0];
						dst_[1] = src_[1];
						dst_[2] = src_[2];
						break;
					default:
		                dst_[0] += (((src_[0] - dst_[0]) * alpha) >> 8);
                        dst_[1] += (((src_[1] - dst_[1]) * alpha) >> 8);
                        dst_[2] += (((src_[2] - dst_[2]) * alpha) >> 8);
                        break;
				}
			}
		}
	}
	else if(image->color_type == FB_COLOR_ALPHA_8) /*lab3: font*/
	{
		char r = color >> 16;
		char g = color >> 8;
		char b = color;

		src = image->content + iy * w + ix;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				char* src_ = src + i * w + j;	// A
				char* dst_ = dst + i * SCREEN_WIDTH * 4 + j * 4;
				alpha = *src_;
				switch (alpha) {
					case 0:
						break;
					case 255:
						dst_[0] = b;
						dst_[1] = g;
						dst_[2] = r;
						break;
					default:
		                dst_[0] += (((b - dst_[0]) * alpha) >> 8);
                        dst_[1] += (((g - dst_[1]) * alpha) >> 8);
                        dst_[2] += (((r - dst_[2]) * alpha) >> 8);
                        break;
				}
			}
		}
	}
/*---------------------------------------------------------------*/
	return;
}

void fb_draw_border(int x, int y, int w, int h, int color)
{
	if(w<=0 || h<=0) return;
	fb_draw_rect(x, y, w, 1, color);
	if(h > 1) {
		fb_draw_rect(x, y+h-1, w, 1, color);
		fb_draw_rect(x, y+1, 1, h-2, color);
		if(w > 1) fb_draw_rect(x+w-1, y+1, 1, h-2, color);
	}
}

/** draw a text string **/
void fb_draw_text(int x, int y, char *text, int font_size, int color)
{
	fb_image *img;
	fb_font_info info;
	int i=0;
	int len = strlen(text);
	while(i < len)
	{
		img = fb_read_font_image(text+i, font_size, &info);
		if(img == NULL) break;
		fb_draw_image(x+info.left, y-info.top, img, color);
		fb_free_image(img);

		x += info.advance_x;
		i += info.bytes;
	}
	return;
}


/**
 * 画一个圆（Bresenham算法）
 * @param x0: 圆心横坐标
 * @param y0: 圆心纵坐标
 * @param r: 圆的半径
 * @param b: 圆的宽度
 * @param color: 圆的颜色
 */
void fb_draw_circle(int x0, int y0, int r, int b, int color) {
    int *buf = _begin_draw(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    void plot(int x, int y) {
        if(x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
            *(buf + y * SCREEN_WIDTH + x) = color;
        }
    }

    for(int w = 0; w < b; w++) {
        int radius = r - w;
        int x = 0;
        int y = radius;
        int d = 3 - 2 * radius;

        while(x <= y) {
            // 在每个八分圆弧上画更多的点
            plot(x0 + x, y0 + y);
            plot(x0 + x + 1, y0 + y); // 填补横向间隙
            plot(x0 - x, y0 + y);
            plot(x0 - x - 1, y0 + y);
            plot(x0 + x, y0 - y);
            plot(x0 + x + 1, y0 - y);
            plot(x0 - x, y0 - y);
            plot(x0 - x - 1, y0 - y);

            plot(x0 + y, y0 + x);
            plot(x0 + y, y0 + x + 1); // 填补纵向间隙
            plot(x0 - y, y0 + x);
            plot(x0 - y, y0 + x + 1);
            plot(x0 + y, y0 - x);
            plot(x0 + y, y0 - x - 1);
            plot(x0 - y, y0 - x);
            plot(x0 - y, y0 - x - 1);

            if(d < 0) {
                d = d + 4 * x + 6;
            } else {
                d = d + 4 * (x - y) + 10;
                y--;
            }
            x++;
        }
    }
}

/**
 * 画一个圆（弃用，效率低）
 * @param x0: 圆心横坐标
 * @param y0: 圆心纵坐标
 * @param r: 圆的半径
 * @param b: 圆的宽度
 * @param color: 圆的颜色
 */
void fb_draw_circle_deprecated(int x0, int y0, int r, int b, int color) {
	// (x - x0) * (x - x0) + (y - y0) * (y - y0) = r * r
	int x_min = x0 - r > 0 ? x0 - r : 0;
	int x_max = x0 + r < SCREEN_WIDTH ? x0 + r : SCREEN_WIDTH;
	int y_min = y0 - r > 0 ? y0 - r : 0;
	int y_max = y0 + r < SCREEN_HEIGHT ? y0 + r : SCREEN_HEIGHT;

	int *buf = _begin_draw(x_min, y_min, x_max - x_min, y_max - y_min);
	float x = x_min;
	while (x <= x_max) {
		for (int i = 0; i < b; i ++) {
			float y1 = y0 + sqrt((r - i) * (r - i) - (x - x0) * (x - x0));
			float y2 = 2 * y0 - y1;
			// 上半部分
			if (y1 > 0 && y1 < SCREEN_HEIGHT ) {
				*(buf + (int)(y1) * SCREEN_WIDTH + (int)x) = color;
			}

			// 下半部分
			if (y2 > 0 && y2 < SCREEN_HEIGHT) {
				*(buf + (int)(y2) * SCREEN_WIDTH + (int)x) = color;
			}
		}
		x+=0.01;
	}
	return;
}

