#ifndef __SAMSUNG_SYSLSI_APDEV_S3C_LCD_H__
#define __SAMSUNG_SYSLSI_APDEV_S3C_LCD_H__

typedef struct {
    int Bpp;
    int LeftTop_x;
    int LeftTop_y;
    int Width;
    int Height;
} s3c_win_info_t;

typedef struct {
    int width;
    int height;
    int bpp;
    int offset;
    int v_width;
    int v_height;
}vs_info_t;

struct s3c_fb_dma_info
{
    unsigned int map_dma_f1;
    unsigned int map_dma_f2;
};

#define FBIO_WAITFORVSYNC _IOW('F', 0x20, u_int32_t)

#define SET_VS_START 		_IO('F', 103)
#define SET_VS_STOP 		_IO('F', 104)
#define SET_VS_INFO 		_IOW('F', 105, vs_info_t)
#define SET_VS_MOVE 		_IOW('F', 106, u_int)

#define SET_OSD_INFO	_IOW('F', 209, s3c_win_info_t)
#define SET_OSD_START	_IO('F', 201)
#define SET_OSD_STOP	_IO('F', 202)

#define GET_FB_INFO			_IOR('F', 307, struct s3c_fb_dma_info)
#define SET_FB_CHANGE_REQ		_IOW('F', 308, int)
#define SET_VSYNC_INT			_IOW('F', 309, int)

#define FB_DEV_NAME		"/dev/fb0"
#define FB_DEV_NAME1	"/dev/fb1"
#define FB_DEV_NAME2	"/dev/fb2"
#define FB_DEV_NAME3	"/dev/fb3"
#define FB_DEV_NAME4	"/dev/fb4"

#define BytesPerPixel			4
#define BitsPerPixel			24

#endif //__SAMSUNG_SYSLSI_APDEV_S3C_LCD_H__
