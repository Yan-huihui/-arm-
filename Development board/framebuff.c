#include "framebuff.h"

struct fb fbinfo;
int init_fb(char *devname)
{
	fbinfo.fb = open(devname, O_RDWR);
	if (-1 == fbinfo.fb)
	{
		perror("fail open fb");
		return -1;
	}
	
	struct fb_var_screeninfo vinfo;

	int ret = ioctl(fbinfo.fb, FBIOGET_VSCREENINFO, &vinfo);
	if (ret < 0)
	{
		perror("fail ioctl");
		return -1;
	}
	printf("x = %d, y = %d\n", vinfo.xres, vinfo.yres);
	printf("x_vtl = %d, y_vtl = %d\n", vinfo.xres_virtual, vinfo.yres_virtual);
	printf("bits_per_pixel = %d\n", vinfo.bits_per_pixel);

	fbinfo.fbx = vinfo.xres_virtual;
	fbinfo.fby = vinfo.yres_virtual;
	fbinfo.bits_per_pixel = vinfo.bits_per_pixel;

	size_t size = vinfo.xres_virtual*vinfo.yres_virtual*vinfo.bits_per_pixel/8;

	void * pmem = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fbinfo.fb, 0);
	if ((void *)-1 == pmem)
	{
		perror("fail mmap");
		return -1;
	}
		
	if (RGB_888_FMT == vinfo.bits_per_pixel / 8)
	{
		fbinfo.rgb_fmt = RGB_888_FMT;
		fbinfo.pmem_888 = pmem;
	}
	else if (RGB_565_FMT == vinfo.bits_per_pixel / 8)
	{
		fbinfo.rgb_fmt = RGB_565_FMT;
		fbinfo.pmem_565 = pmem;
	}
	else
	{
		//other rgb fmt
	}


	return 0;
}

int uninit_fb()
{
	if (RGB_888_FMT == fbinfo.rgb_fmt)
	{
		munmap(fbinfo.pmem_888, fbinfo.fbx*fbinfo.fby*fbinfo.bits_per_pixel/8);
	}
	else if(RGB_565_FMT == fbinfo.rgb_fmt)
	{
		munmap(fbinfo.pmem_565, fbinfo.fbx*fbinfo.fby*fbinfo.bits_per_pixel/8);	
	}
	close(fbinfo.fb);
	return 0;
}

void draw_point(int x, int y, unsigned int col)
{
	if (x >= fbinfo.fbx || y >= fbinfo.fby)
	{
		return ;
	}
	
	if (RGB_888_FMT == fbinfo.rgb_fmt)
	{
		*(fbinfo.pmem_888 + y*fbinfo.fbx + x) = col;
	}
	else if (RGB_565_FMT == fbinfo.rgb_fmt)
	{
		*(fbinfo.pmem_565 + y*fbinfo.fbx + x) = col;
	}

	return ;
}

void draw_line(int x0, int y0, int x1, int y1, int color)
{
	int x, y;
	if(0 != (x1 - x0))
	{
		int a = (y1 - y0) / (x1 - x0);
		int b = y1 - (a * x1);
		for(x = 0; x < fbinfo.fbx; ++x)
		{
			y = (a * x) + b;
			draw_point(x, y, color);
		}
	}
	else
	{
		for(y = 0; y < fbinfo.fby; ++y)
		{
			draw_point(x0, y, color);
		}
	}
}

void draw_clear(int color)
{	
	int x, y;
	for(y = 0; y < fbinfo.fby; ++y)
	{
		for(x = 0; x < fbinfo.fbx; ++x)
		{
			draw_point(x, y, color);
		}
	}
}

void draw_gb2312(int x, int y, unsigned char * font_yan, int high, int len, unsigned int fgcol, unsigned int bkcol)
{
	int i, j, k;
	unsigned char temp;
	for(i = 0; i < high; i++)
	{
		for(k = 0; k < len; k++)
		{
			temp =font_yan[i * len + k];
			for(j = 0; j < 8; j++)
			{
				if(temp & 0x80)
				{
					draw_point(8*k+x+j, y+i, fgcol);
				}
				else
				{
					draw_point(8*k+x+j, y+i, bkcol);
				}
				temp = temp << 1;
			}
		}
	}
}

unsigned char A[16*21/8] = {
/*--  文字:  A  --*/
/*--  仿宋16;  此字体下对应的点阵为：宽x高=11x21   --*/
/*--  宽度不是8的倍数，现调整为：宽度x高度=16x21  --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x0E,0x00,0x1E,0x00,0x1E,0x00,0x1E,0x00,
0x1F,0x00,0x37,0x00,0x37,0x00,0x33,0x00,0x3F,0x80,0x63,0x80,0x63,0x80,0x63,0x80,
0x61,0xC0,0xF3,0xE0,0x00,0x00,0x00,0x00,0x00,0x00};

int show_utf8(UTF8_INFO *info, int x, int y, char* zi, u32 col, u32 col1)
{
    unsigned long  out = 0 ;
    int ret = enc_utf8_to_unicode_one((unsigned char*)zi,&out);

    unsigned char* data = get_utf_data(info,out);
    unsigned char temp = 0 ;
    unsigned int i,j,k;
    unsigned int num = 0;
    for(i=0;i<info->height;i++)
    {
        for(j=0;j<info->width/8;j++)
        {
            temp = data[num++];
            for(k=0;k<8;k++)
            {
                if(0x80&temp)
                {
                    draw_point( x+k+j*8, y+i, col);
                }
                else
                {

                    //draw_point( x+k+j*8, y+i, col1);
                }
                temp= temp<<1;
            }
        }
    }
    return ret;
}

int show_utf8_str(UTF8_INFO *info, int arg_x, int arg_y,  char* zi, u32 col, u32 col1)
{
    char* temp = zi;
    unsigned int x = arg_x ;
    unsigned int y =  arg_y;

    while(*temp != '\0')
    {
        int ret = show_utf8(info, x, y, temp, col, col1);
        x += info->width;
        if(x > fbinfo.fbx)
        {
            x = 0;
            y += info->height;
            if(y > fbinfo.fby)
            {
                y = 0;
            }
        }
        temp += ret;
    }
    return 0;
}
void draw_bmp(int x, int y, int w, int h, char *filename)
{
	int fd = open(filename, O_RDONLY);
	if (-1 == fd)
	{
		perror("fail open bmp");
		return ;
	}
	
	unsigned char *buff = malloc(w*h*3);
	read(fd, buff, 54);
	read(fd, buff, w*h*3);
	
	unsigned char *p = buff;
	int i,j;
	for (i = h-1; i >= 0; i--)
	{
		for (j = 0; j < w; j++)
		{
			unsigned char r, g, b;
			b = *p++;
			g = *p++;
			r = *p++;
		
			if (RGB_888_FMT == fbinfo.rgb_fmt)
			{
				unsigned int col = 0;
				col = (r<<16) | (g << 8) | (b << 0);
				draw_point(j, i, col);
			}
			else if (RGB_565_FMT == fbinfo.rgb_fmt)
			{
				unsigned short col = 0;
				col = ((r>>3)<<11) | ((g>>2)<<5) | ((b>>3)<<0);
				draw_point(j, i, col);
			}
		}
	}
    free(buff);
}
