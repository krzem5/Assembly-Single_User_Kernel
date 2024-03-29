#ifndef _UI_DISPLAY_H_
#define _UI_DISPLAY_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



#define UI_DISPLAY_MODE_IS_VALID(mode) (!!((mode)->freq))

#define UI_DISPLAY_FRAMEBUFFER_FORMAT_NONE 0
#define UI_DISPLAY_FRAMEBUFFER_FORMAT_BGRX 1
#define UI_DISPLAY_FRAMEBUFFER_FORMAT_RGBX 2
#define UI_DISPLAY_FRAMEBUFFER_FORMAT_XBGR 3
#define UI_DISPLAY_FRAMEBUFFER_FORMAT_XRGB 4

#define UI_DISPLAY_FRAMEBUFFER_FORMAT_MIN UI_DISPLAY_FRAMEBUFFER_FORMAT_BGRX
#define UI_DISPLAY_FRAMEBUFFER_FORMAT_MAX UI_DISPLAY_FRAMEBUFFER_FORMAT_XRGB



typedef u64 ui_display_handle_t;



typedef u64 ui_framebuffer_handle_t;



typedef struct _UI_DISPLAY_MODE{
	u32 width;
	u32 height;
	u32 freq;
} ui_display_mode_t;



typedef struct _UI_DISPLAY_DATA{
	u32 index;
	ui_display_mode_t mode;
} ui_display_data_t;



typedef struct _UI_DISPLAY_INFO{
	char manufacturer[4];
	u32 manufacturer_product_code;
	u32 serial_number;
	u8 video_interface;
	u32 screen_width_cm;
	u32 screen_height_cm;
	char name[14];
	u32 mode_count;
	ui_display_mode_t modes[];
} ui_display_info_t;



typedef struct _UI_DISPLAY_FRAMEBUFFER{
	u32 width;
	u32 height;
	u32 format;
} ui_display_framebuffer_t;



ui_display_handle_t ui_display_iter_start(void);



ui_display_handle_t ui_display_iter_next(ui_display_handle_t handle);



sys_error_t ui_display_get_data(ui_display_handle_t handle,ui_display_data_t* out);



sys_error_t ui_display_get_info(ui_display_handle_t handle,ui_display_info_t* buffer,u32 buffer_length);



sys_error_t ui_display_get_display_framebuffer(ui_display_handle_t handle);



sys_error_t ui_display_get_framebuffer_config(ui_framebuffer_handle_t handle,ui_display_framebuffer_t* out);



sys_error_t ui_display_flush_display_framebuffer(ui_display_handle_t handle);



#endif
