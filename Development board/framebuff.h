#ifndef FRAMEBUFF_H
#define FRAMEBUFF_H


#include <stdio.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "utf.h"
#define RGB_888_FMT 4
#define RGB_565_FMT 2

struct fb
{
	int fb;
	unsigned int fbx;
	unsigned int fby;
	unsigned int bits_per_pixel;
	unsigned int *pmem_888;
	unsigned short *pmem_565;
	int rgb_fmt;
};

extern unsigned char A[16*21/8];
int init_fb(char *devname);
int uninit_fb(void);
void draw_point(int x, int y, unsigned int col);
void draw_line(int x0, int y0, int x1, int y1, int color);
void draw_clear(int color);
void draw_gb2312(int x, int y, unsigned char * font_yan, int high, int len, unsigned int fgcol, unsigned int bkcol);
void draw_bmp(int x, int y, int w, int h, char *filename);
int show_utf8(UTF8_INFO *info, int x, int y, char* zi, u32 col, u32 col1);
int show_utf8_str(UTF8_INFO *info, int arg_x, int arg_y,  char* zi, u32 col, u32 col1);

#endif
