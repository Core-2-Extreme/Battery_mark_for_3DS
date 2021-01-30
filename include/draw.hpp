#pragma once
#include "citro2d.h"
#include "types.hpp"

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)
// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)

#define BitVal(data,y) ( (data>>y) & 1)      //Return Data.Y value
#define SetBit(data,y)    data |= (1 << y)    //Set Data.Y   to 1
#define ClearBit(data,y)  data &= ~(1 << y)   //Clear Data.Y to 0

extern C2D_Image Wifi_icon_image[9];
extern C2D_Image Battery_level_icon_image[21];
extern C2D_Image Battery_charge_icon_image[1];
extern C2D_Image Square_image[1];
extern C2D_Image ui_image[4];
extern C2D_ImageTint texture_tint, dammy_tint, black_or_white_tint, white_or_black_tint, white_tint, weak_white_tint, red_tint, weak_red_tint, aqua_tint, weak_aqua_tint, yellow_tint, weak_yellow_tint, blue_tint, weak_blue_tint, black_tint, weak_black_tint;

int Draw_query_fps(void);

double Draw_query_frametime(void);

void Draw_reset_fps(void);

void Draw_rgba_to_abgr(u8* buf, u32 width, u32 height);

void Draw_yuv420p_to_rgb565(unsigned char *data, unsigned char *data_1, unsigned char *data_2, unsigned char *rgb, int width, int height);

void Draw_rgb565_to_abgr888_rgb888(u8* rgb565_buffer, u8* rgba8888_buffer, u32 width, u32 height, bool rgb_888);

Result_with_string Draw_create_texture(C3D_Tex* c3d_tex, Tex3DS_SubTexture* c3d_subtex, u8* buf, u32 size, u32 width, u32 height, int pixel_size, int parse_start_width, int parse_start_height, int tex_size_x, int tex_size_y, GPU_TEXCOLOR format);

void Draw_set_do_not_draw_flag(bool flag);

void Draw(std::string text, float x, float y, float text_size_x, float text_size_y, float r, float g, float b, float a);

Result_with_string Draw_load_texture(std::string file_name, int sheet_map_num, C2D_Image return_image[], int start_num, int num_of_array);

bool Draw_query_need_reflesh(void);

void Draw_touch_pos(void);

void Draw_top_ui(void);

void Draw_bot_ui(void);

void Draw_texture(C2D_Image image[], int num, float x, float y, float x_size, float y_size);

void Draw_texture(C2D_Image image[], C2D_ImageTint tint, int image_num, float x, float y, float x_size, float y_size);

void Draw_error(void);

void Draw_progress(std::string message);

void Draw_log(bool force_draw);

void Draw_debug_info(void);

void Draw_init(void);

Result_with_string Draw_load_kanji_samples(void);

Result_with_string Draw_load_system_font(int system_font_num);

void Draw_free_system_font(int system_font_num);

void Draw_free_texture(int sheet_map_num);

void Draw_exit(void);

void Draw_frame_ready(void);

void Draw_screen_ready_to_draw(int screen_num, bool screen_clear, int screen_clear_ver, float red, float green, float blue);

void Draw_apply_draw(void);

//bool Moded_C3D_TexInitWithParams(C3D_Tex* tex, C3D_TexCube* cube, C3D_TexInitParams p);
