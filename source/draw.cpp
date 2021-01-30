#include <string>
#include "citro2d.h"

#include "hid.hpp"
#include "draw.hpp"
#include "error.hpp"
#include "external_font.hpp"
#include "log.hpp"
#include "types.hpp"
#include "setting_menu.hpp"
#include "explorer.hpp"
#include "menu.hpp"
#include "file.hpp"

bool draw_do_not_draw = false;
bool draw_pre_battery_charge = false;
bool draw_pre_log_show = false;
bool draw_pre_error_show = false;
bool draw_pre_p_touch_state = false;
bool draw_pre_h_touch_state = false;
int draw_pre_touch_x = 0;
int draw_pre_touch_y = 0;
int draw_pre_battery_level_raw = -1;
int draw_pre_wifi_state = -1;
Hid_info draw_pre_key;
int draw_fps = 0;
double draw_frametime = 0.0;
std::string draw_pre_battery_level = "";
std::string draw_pre_status = "";
std::string draw_part_text[2][1024];
C2D_Font system_fonts[4];
C3D_RenderTarget* screen[3];
C2D_SpriteSheet sheet_texture[128];
C2D_Image Wifi_icon_image[9];
C2D_Image Battery_level_icon_image[21];
C2D_Image Battery_charge_icon_image[1];
C2D_Image Square_image[1];
C2D_Image ui_image[4];
std::string draw_japanese_kanji[3000];
std::string draw_simple_chinese[6300];
C2D_ImageTint texture_tint, dammy_tint, black_or_white_tint, white_or_black_tint, white_tint, weak_white_tint, red_tint, weak_red_tint, aqua_tint, weak_aqua_tint, yellow_tint, weak_yellow_tint, blue_tint, weak_blue_tint, black_tint, weak_black_tint;
TickCounter draw_frame_time_timer;

int Draw_query_fps(void)
{
	return draw_fps;
}

double Draw_query_frametime(void)
{
	return draw_frametime;
}

void Draw_reset_fps(void)
{
	draw_fps = 0;
}

void Draw_rgba_to_abgr(u8* buf, u32 width, u32 height)
{
	// RGBA -> ABGR
	for (u32 row = 0; row < width; row++) {
		for (u32 col = 0; col < height; col++) {
			u32 z = (row + col * width) * 4;

			u8 r = *(u8*)(buf + z);
			u8 g = *(u8*)(buf + z + 1);
			u8 b = *(u8*)(buf + z + 2);
			u8 a = *(u8*)(buf + z + 3);

			*(buf + z) = a;
			*(buf + z + 1) = b;
			*(buf + z + 2) = g;
			*(buf + z + 3) = r;
		}
	}
}

extern "C" void memcpy_asm_4b(u8*, u8*);
/*#include <unistd.h>
extern "C" uint8_t read_b(void);
extern "C" uint8_t read_g(void);
extern "C" uint8_t read_r(void);
extern "C" uint16_t set_yuv(uint8_t, uint8_t, uint8_t);*/

void Draw_yuv420p_to_rgb565(unsigned char *data, unsigned char *data_1, unsigned char *data_2, unsigned char *rgb, int width, int height)
{
    int index = 0;
	int uv_pos = 0;
	int y_pos = 0;
    unsigned char *ybase = data;
    unsigned char *ubase = data_1;//&data[width * height];
    unsigned char *vbase = data_2;//&data[width * height * 5 / 4];
		//    unsigned char *vbase = &data[(width * height) + (width * height / 4)];
		uint8_t Y[4], U, V, r[4], g[4], b[4];
		if(width % 4 != 0)
		{
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
						//YYYYYYYYUUVV
						Y[0] = ybase[x + y * width];
						U = ubase[y / 2 * width / 2 + (x / 2)];
						V = vbase[y / 2 * width / 2 + (x / 2)];
						b[0] = YUV2B(Y[0], U);
						g[0] = YUV2G(Y[0], U, V);
						r[0] = YUV2R(Y[0], V);
						b[0] = b[0] >> 3;
						g[0] = g[0] >> 2;
						r[0] = r[0] >> 3;
						rgb[index++] = (g[0] & 0b00000111) << 5 | b[0];
						rgb[index++] = (g[0] & 0b00111000) >> 3 | (r[0] & 0b00011111) << 3;
				}
			}
		}
		else
		{
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x += 4)
				{
					//YYYYYYYYUUVV
					uv_pos = y / 2 * width / 2 + (x / 2);
					y_pos = x + y * width;
					U = ubase[uv_pos];
					V = vbase[uv_pos];
					Y[0] = ybase[y_pos];
					Y[1] = ybase[y_pos + 1];
					Y[2] = ybase[y_pos + 2];
					Y[3] = ybase[y_pos + 3];

					/*rgb[index] = set_yuv(Y[0], U, V);
					index += 2;
					rgb[index] = set_yuv(Y[1], U, V);
					index += 2;
					rgb[index] = set_yuv(Y[2], U, V);
					index += 2;
					rgb[index] = set_yuv(Y[3], U, V);
					index += 2;*/

					b[0] = YUV2B(Y[0], U);
					g[0] = YUV2G(Y[0], U, V);
					r[0] = YUV2R(Y[0], V);
					b[1] = YUV2B(Y[1], U);
					g[1] = YUV2G(Y[1], U, V);
					r[1] = YUV2R(Y[1], V);
					b[2] = YUV2B(Y[2], U);
					g[2] = YUV2G(Y[2], U, V);
					r[2] = YUV2R(Y[2], V);
					b[3] = YUV2B(Y[3], U);
					g[3] = YUV2G(Y[3], U, V);
					r[3] = YUV2R(Y[3], V);

					/*
					for(int i = 0; i < 4; i++)
					{
						b[i] = b[i] >> 3;
						g[i] = g[i] >> 2;
						r[i] = r[i] >> 3;
						rgb[index++] = (g[i] & 0b00000111) << 5 | b[i];
						rgb[index++] = (g[i] & 0b00111000) >> 3 | (r[i] & 0b00011111) << 3;
					}*/
					b[0] = b[0] >> 3;
					g[0] = g[0] >> 2;
					r[0] = r[0] >> 3;
					b[1] = b[1] >> 3;
					g[1] = g[1] >> 2;
					r[1] = r[1] >> 3;
					b[2] = b[2] >> 3;
					g[2] = g[2] >> 2;
					r[2] = r[2] >> 3;
					b[3] = b[3] >> 3;
					g[3] = g[3] >> 2;
					r[3] = r[3] >> 3;

					rgb[index++] = (g[0] & 0b00000111) << 5 | b[0];
					rgb[index++] = (g[0] & 0b00111000) >> 3 | (r[0] & 0b00011111) << 3;
					rgb[index++] = (g[1] & 0b00000111) << 5 | b[1];
					rgb[index++] = (g[1] & 0b00111000) >> 3 | (r[1] & 0b00011111) << 3;
					rgb[index++] = (g[2] & 0b00000111) << 5 | b[2];
					rgb[index++] = (g[2] & 0b00111000) >> 3 | (r[2] & 0b00011111) << 3;
					rgb[index++] = (g[3] & 0b00000111) << 5 | b[3];
					rgb[index++] = (g[3] & 0b00111000) >> 3 | (r[3] & 0b00011111) << 3;
				}
			}
			/*for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x += 4)
				{
						//YYYYYYYYUUVV
						Y = ybase[x + y * width];
						U = ubase[y / 2 * width / 2 + (x / 2)];
						V = vbase[y / 2 * width / 2 + (x / 2)];
						rgb[index++] = YUV2B(Y, U, V);
						rgb[index++] = YUV2G(Y, U, V);
						rgb[index++] = YUV2R(Y, U, V);

						Y = ybase[x + 1 + y * width];
						rgb[index++] = YUV2B(Y, U, V);
						rgb[index++] = YUV2G(Y, U, V);
						rgb[index++] = YUV2R(Y, U, V);

						Y = ybase[x + 2 + y * width];
						rgb[index++] = YUV2B(Y, U, V);
						rgb[index++] = YUV2G(Y, U, V);
						rgb[index++] = YUV2R(Y, U, V);

						Y = ybase[x + 3 + y * width];
						rgb[index++] = YUV2B(Y, U, V);
						rgb[index++] = YUV2G(Y, U, V);
						rgb[index++] = YUV2R(Y, U, V);
				}
			}*/
		}

		//640*360 27.8ms +-0.5ms
    /*for (int y = 0; y < height; y++)
		{
        for (int x = 0; x < width; x ++)
				{
            //YYYYYYYYUUVV
						Y = ybase[x + y * width];
						U = ubase[y / 2 * width / 2 + (x / 2)];
            V = vbase[y / 2 * width / 2 + (x / 2)];

						rgb[index++] = Y + 1.77 * (U - 128); //B
						rgb[index++] = Y - 0.34 * (U - 128) - 0.71 * (V - 128); //G
						rgb[index++] = Y + 1.40 * (V - 128); //R
        }
    }*/

		//640*360 14ms +-0.5ms
		/*for (int y = 0; y < height; y++)
		{
				for (int x = 0; x < width; x ++)
				{
						//YYYYYYYYUUVV
						Y = ybase[x + y * width];
						U = ubase[y / 2 * width / 2 + (x / 2)];
						V = vbase[y / 2 * width / 2 + (x / 2)];

						rgb[index++] = YUV2B(Y, U, V);
						rgb[index++] = YUV2G(Y, U, V);
						rgb[index++] = YUV2R(Y, U, V);
				}
		}*/

		//640*360 15.6ms +-0.5ms
		/*for (int y = 0; y < height; y++)
		{
				for (int x = 0; x < width; x ++)
				{
						//YYYYYYYYUUVV
						Y = ybase[x + y * width];
						if(x % 4 == 0)
						{
							U = ubase[y / 2 * width / 2 + (x / 2)];
							V = vbase[y / 2 * width / 2 + (x / 2)];
						}

						rgb[index++] = YUV2B(Y, U, V);
						rgb[index++] = YUV2G(Y, U, V);
						rgb[index++] = YUV2R(Y, U, V);
				}
		}*/

		//640*360 10ms +-0.5ms
		/*for (int y = 0; y < height; y++)
		{
				for (int x = 0; x < width; x += 4)
				{
						//YYYYYYYYUUVV
						Y = ybase[x + y * width];
						U = ubase[y / 2 * width / 2 + (x / 2)];
						V = vbase[y / 2 * width / 2 + (x / 2)];
						rgb[index++] = YUV2B(Y, U, V);
						rgb[index++] = YUV2G(Y, U, V);
						rgb[index++] = YUV2R(Y, U, V);

						Y = ybase[x + 1 + y * width];
						rgb[index++] = YUV2B(Y, U, V);
						rgb[index++] = YUV2G(Y, U, V);
						rgb[index++] = YUV2R(Y, U, V);

						Y = ybase[x + 2 + y * width];
						rgb[index++] = YUV2B(Y, U, V);
						rgb[index++] = YUV2G(Y, U, V);
						rgb[index++] = YUV2R(Y, U, V);

						Y = ybase[x + 3 + y * width];
						rgb[index++] = YUV2B(Y, U, V);
						rgb[index++] = YUV2G(Y, U, V);
						rgb[index++] = YUV2R(Y, U, V);
				}
		}*/
}

void Draw_rgb565_to_abgr888_rgb888(u8* rgb565_buffer, u8* rgba8888_buffer, u32 width, u32 height, bool rgb_888)
{
	int count = 0;
	for (int i = 0; i < (int)width * (int)height * 2; i += 2)
	{
		uint8_t b = (rgb565_buffer[i] & 0x1F);
		uint8_t r = ((rgb565_buffer[i + 1] >> 3) & 0x1F);
		uint8_t g_0 = ((rgb565_buffer[i + 1]) & 0x7);
		uint8_t g_1 = ((rgb565_buffer[i]) & 0xE0);
		uint8_t g = 0;

		if (BitVal(g_0, 2))
			SetBit(g, 5);
		if (BitVal(g_0, 1))
			SetBit(g, 4);
		if (BitVal(g_0, 0))
			SetBit(g, 3);
		if (BitVal(g_1, 7))
			SetBit(g, 2);
		if (BitVal(g_1, 6))
			SetBit(g, 1);
		if (BitVal(g_1, 5))
			SetBit(g, 0);

		b = 255 / 31 * b;
		g = 255 / 63 * g;
		r = 255 / 31 * r;

		if (rgb_888)
		{
			memset((void*)(rgba8888_buffer + count), r, 0x1);
			memset((void*)(rgba8888_buffer + count + 1), g, 0x1);
			memset((void*)(rgba8888_buffer + count + 2), b, 0x1);
			count += 3;
		}
		else
		{
			memset((void*)(rgba8888_buffer + count), 255, 0x1);
			memset((void*)(rgba8888_buffer + count + 1), b, 0x1);
			memset((void*)(rgba8888_buffer + count + 2), g, 0x1);
			memset((void*)(rgba8888_buffer + count + 3), r, 0x1);
			count += 4;
		}
	}
}

int Draw_convert_to_pos(int height, int width, int img_height, int img_width, int pixel_size)
{
	int pos = img_width * height;
	if(pos == 0)
		pos = img_width;

	pos -= (img_width - width) - img_width;
	return pos * pixel_size;
}


Result_with_string Draw_create_texture(C3D_Tex* c3d_tex, Tex3DS_SubTexture* c3d_subtex, u8* buf, u32 size, u32 width, u32 height, int pixel_size, int parse_start_width, int parse_start_height, int tex_size_x, int tex_size_y, GPU_TEXCOLOR format)
{
	bool init_result = false;
	u32 x_max;
	u32 y_max;
	int increase_list_x[1024]; //= { 4, 12, 4, 44, }
	int increase_list_y[1024]; //= { 2, 6, 2, 22, 2, 6, 2, tex_size_x * 8 - 42, };
	int count[2] = { 0, 0, };
	int buf_pos = 0;
	int c3d_pos = 0;
	int c3d_offset = 0;
	Result_with_string result;
	for(int i = 0; i <= tex_size_x; i+=4)
	{
		increase_list_x[i] = 4 * pixel_size;
		increase_list_x[i + 1] = 12 * pixel_size;
		increase_list_x[i + 2] = 4 * pixel_size;
		increase_list_x[i + 3] = 44 * pixel_size;
	}
	for(int i = 0; i <= tex_size_y; i+=8)
	{
		increase_list_y[i] = 2 * pixel_size;
		increase_list_y[i + 1] = 6 * pixel_size;
		increase_list_y[i + 2] = 2 * pixel_size;
		increase_list_y[i + 3] = 22 * pixel_size;
		increase_list_y[i + 4] = 2 * pixel_size;
		increase_list_y[i + 5] = 6 * pixel_size;
		increase_list_y[i + 6] = 2 * pixel_size;
		increase_list_y[i + 7] = (tex_size_x * 8 - 42) * pixel_size;
	}
	/*
	for(int i = 0; i < 8; i++)
		increase_list_y[i] *= pixel_size;
	*/

	if (parse_start_width > (int)width || parse_start_height > (int)height)
	{
		result.code = WRONG_PARSING_POS;
		result.string = Err_query_template_detail(WRONG_PARSING_POS);
		return result;
	}

	init_result = C3D_TexInit(c3d_tex, (u16)tex_size_x, (u16)tex_size_y, format);
	if (!init_result)
	{
		result.code = OUT_OF_LINEAR_MEMORY;
		result.string = Err_query_template_summary(OUT_OF_LINEAR_MEMORY);
		result.error_description = Err_query_template_detail(OUT_OF_LINEAR_MEMORY);
		return result;
	}

	//memset(c3d_tex->data, 0xFF, c3d_tex->size);
	C3D_TexSetFilter(c3d_tex, GPU_LINEAR, GPU_LINEAR);
	
	y_max = height - (u32)parse_start_height;
	x_max = width - (u32)parse_start_width;
	if ((u32)tex_size_y < y_max)
		y_max = tex_size_y;
	if ((u32)tex_size_x < x_max)
		x_max = tex_size_x;

	c3d_subtex->width = (u16)x_max;
	c3d_subtex->height = (u16)y_max;
	c3d_subtex->left = 0.0f;
	c3d_subtex->top = 1.0f;
	c3d_subtex->right = x_max / (float)tex_size_x;
	c3d_subtex->bottom = 1.0 - y_max / (float)tex_size_y;

	if(pixel_size == 2)
	{
		for(u32 k = 0; k < y_max; k++)
		{
			for(u32 i = 0; i < x_max; i += 2)
			{
				memcpy_asm_4b(&(((u8*)c3d_tex->data)[c3d_pos + c3d_offset]), &(((u8*)buf)[Draw_convert_to_pos(k + parse_start_height, i + parse_start_width, height, width, pixel_size)]));
				//memcpy(&((u8*)c3d_tex->data)[c3d_pos + c3d_offset], &((u8*)buf)[Draw_convert_to_pos(k + parse_start_height, i + parse_start_width, height, width, pixel_size)], pixel_size * 2);
				c3d_pos += increase_list_x[count[0]];
				count[0]++;
			}
			count[0] = 0;
			c3d_pos = 0;
			c3d_offset += increase_list_y[count[1]];
			count[1]++;
		}
	}
	else if(pixel_size == 4)
	{
		for(u32 k = 0; k < y_max; k++)
		{
			for(u32 i = 0; i < x_max; i += 2)
			{
				memcpy_asm_4b(&(((u8*)c3d_tex->data)[c3d_pos + c3d_offset]), &(((u8*)buf)[Draw_convert_to_pos(k + parse_start_height, i + parse_start_width, height, width, pixel_size)]));
				memcpy_asm_4b(&(((u8*)c3d_tex->data)[c3d_pos + c3d_offset + 4]), &(((u8*)buf)[Draw_convert_to_pos(k + parse_start_height, i + parse_start_width, height, width, pixel_size) + 4]));

				//memcpy(&((u8*)c3d_tex->data)[c3d_pos + c3d_offset], &((u8*)buf)[Draw_convert_to_pos(k + parse_start_height, i + parse_start_width, height, width, pixel_size)], pixel_size * 2);
				c3d_pos += increase_list_x[count[0]];
				count[0]++;
			}
			count[0] = 0;
			c3d_pos = 0;
			c3d_offset += increase_list_y[count[1]];
			count[1]++;
		}
	}

/*
	for(u32 k = 0; k < y_max; k++)
	{
		for(u32 i = 0; i < x_max; i += 2)
		{
			memcpy(&((u8*)c3d_tex->data)[c3d_pos + c3d_offset], &((u8*)buf)[Draw_convert_to_pos(k + parse_start_height, i + parse_start_width, height, width, pixel_size)], pixel_size * 2);
			c3d_pos += increase_list_x[count[0]];
			count[0]++;
			if(count[0] >= 4)
				count[0] = 0;
		}
		count[0] = 0;
		c3d_pos = 0;
		c3d_offset += increase_list_y[count[1]];
		count[1]++;
		if(count[1] >= 8)
			count[1] = 0;
	}

	*/

	/*for (u32 y = 0; y < y_max; y++)
	{
		for (u32 x = 0; x < x_max; x++)
		{
			u32 dst_pos = ((((y >> 3)* ((u32)tex_size_x >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3)))* pixel_size;
			if (dst_pos <= c3d_tex->size)
			{
				//Log_log_save("", std::to_string(dst_pos), 1234567890, false);
				u32 src_pos = (((y + parse_start_height) * width + (x + parse_start_width))) * pixel_size;
				memcpy(&((u8*)c3d_tex->data)[dst_pos], &((u8*)buf)[src_pos], pixel_size);
				//sleep(25000);
			}
		}
	}*/

	c3d_tex->border = 0xFFFFFF;
	C3D_TexSetWrap(c3d_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
	C3D_TexFlush(c3d_tex);

	return result;
}

void Draw_set_do_not_draw_flag(bool flag)
{
	draw_do_not_draw = flag;
}

void Draw(std::string text, float x, float y, float text_size_x, float text_size_y, float r, float g, float b, float a)
{
	bool reverse = false;
	bool found = false;
	bool font_loaded[2] = { Sem_query_loaded_system_font_flag(0), Sem_query_loaded_system_font_flag(1), };//JPN, CHN
	float width = 0, height = 0, original_x, y_offset;
	int previous_num = -3;
	int memcmp_result = -1;
	int count = 0;
	int characters = 0;
	int font_num_list[2][1024];
	std::string sample[8] = { "\u0000", "\u000A", "\u4DFF", "\uA000", "\u312F", "\u3190", "\uABFF", "\uD7B0", };
	C2D_Text c2d_text;
	C2D_TextBuf c2d_buf;
	original_x = x;
	c2d_buf = C2D_TextBufNew(4096);

	Exfont_text_parse(text, draw_part_text[0], 1023, &characters);
	Exfont_text_parse(Exfont_text_sort(draw_part_text[0], 1023), draw_part_text[0], 1023, &characters);

	for (int i = 0; i < characters; i++)
	{
		reverse = false;
		if (memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[0].c_str(), 0x1) == 0)
		{
			font_num_list[0][i] = -2;
			break;
		}
		else if (memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[1].c_str(), 0x1) == 0)
		{
			font_num_list[0][i] = -1;
			continue;
		}

		if((memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[2].c_str(), 0x3) > 0 && memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[3].c_str(), 0x3) < 0)
		|| (memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[4].c_str(), 0x3) > 0 && memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[5].c_str(), 0x3) < 0)
		|| (memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[6].c_str(), 0x3) > 0 && memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[7].c_str(), 0x3) < 0))
		{
			if(memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[2].c_str(), 0x3) > 0 && memcmp((void*)draw_part_text[0][i].c_str(), (void*)sample[3].c_str(), 0x3) < 0)
			{
				found = false;
				reverse = false;
				memcmp_result = 1;

				if(font_loaded[0])
				{
					for(int s = 0;;)
					{
						if(!reverse)
							s += 100;
						else
							s--;

						if((s < 0 || s > 3000) && reverse)
							break;
						else if(s > 3000)
						{
							reverse = true;
							s = 3000;
						}
						else
							memcmp_result = memcmp((void*)draw_part_text[0][i].c_str(), (void*)draw_japanese_kanji[s].c_str(), 3);

						if(memcmp_result == 0)
						{
							font_num_list[0][i] = 0; //JPN
							found = true;
							break;
						}
						else if(memcmp_result < 0)
							reverse = true;
					}
				}

				if(!found)
				{
					reverse = false;
					memcmp_result = 1;

					if(font_loaded[1])
					{
						for(int s = 0;;)
						{
							if(!reverse)
								s += 100;
							else
								s--;

							if((s < 0 || s > 6300) && reverse)
								break;
							else if(s > 6300)
							{
								reverse = true;
								s = 6300;
							}
							else
								memcmp_result = memcmp((void*)draw_part_text[0][i].c_str(), (void*)draw_simple_chinese[s].c_str(), 3);

							if(memcmp_result == 0)
							{
								font_num_list[0][i] = 1; //CHN
								found = true;
								break;
							}
							else if(memcmp_result < 0)
								reverse = true;
						}
					}
				}

				if(!found)
				  font_num_list[0][i] = 3; //TWN
			}
			else
				font_num_list[0][i] = 2; //KOR
		}
		else
			font_num_list[0][i] = 4;
	}

	draw_part_text[1][0] = "";
	previous_num = font_num_list[0][0];
	for (int i = 0; i < characters; i++)
	{
		if(font_num_list[0][i] == -2)
		{
			font_num_list[1][count + 1] = font_num_list[0][i];
			break;
		}
		else if(previous_num != font_num_list[0][i] || font_num_list[0][i] == -1)
		{
			count++;
			draw_part_text[1][count] = "";
		}

		draw_part_text[1][count] += draw_part_text[0][i];
		font_num_list[1][count] = font_num_list[0][i];
		previous_num = font_num_list[0][i];
	}

	for (int i = 0; i <= count; i++)
	{
		if (font_num_list[1][i] == -2)
			break;
		else if (font_num_list[1][i] == -1)
		{
			y += 20.0 * text_size_y;
			x = original_x;
			continue;
		}

		if(!Sem_query_loaded_external_font_flag(0) || (font_num_list[1][i] >= 0 && font_num_list[1][i] <= 3))
		{
			if(!Sem_query_loaded_external_font_flag(0))
				system_fonts[font_num_list[1][i]] = 0;

			C2D_TextBufClear(c2d_buf);
			if(font_num_list[1][i] == 1)
				y_offset = 3 * text_size_y;
			else if(font_num_list[1][i] == 3)
				y_offset = 5 * text_size_y;
			else
				y_offset = 0;

			C2D_TextFontParse(&c2d_text, system_fonts[font_num_list[1][i]], c2d_buf, draw_part_text[1][i].c_str());
			C2D_TextOptimize(&c2d_text);
			C2D_TextGetDimensions(&c2d_text, text_size_x, text_size_y, &width, &height);
			C2D_DrawText(&c2d_text, C2D_WithColor, x, y + y_offset, 0.0, text_size_x, text_size_y, C2D_Color32f(r, g, b, a));
			x += width;
		}
		else if(font_num_list[1][i] == 4)
		{
			Exfont_draw_external_fonts(draw_part_text[1][i], x, y, text_size_x * 1.56, text_size_y * 1.56, r, g, b, a, &width, &height);
			x += width;
		}
	}
	C2D_TextBufDelete(c2d_buf);
}

Result_with_string Draw_load_texture(std::string file_name, int sheet_map_num, C2D_Image return_image[], int start_num, int num_of_array)
{
	size_t num_of_images;
	bool function_fail = false;
	Result_with_string load_texture_result;

	sheet_texture[sheet_map_num] = C2D_SpriteSheetLoad(file_name.c_str());
	if (sheet_texture[sheet_map_num] == NULL)
	{
		load_texture_result.code = -1;
		load_texture_result.string = "[Error] Couldn't load texture file : " + file_name + " ";
		function_fail = true;
	}

	if (!function_fail)
	{
		num_of_images = C2D_SpriteSheetCount(sheet_texture[sheet_map_num]);
		if ((int)num_of_images < num_of_array)
		{
			load_texture_result.code = -2;
			load_texture_result.string = "[Error] num of arry " + std::to_string(num_of_array) + " is bigger than spritesheet has num of image(s) " + std::to_string(num_of_images) + " ";
			function_fail = true;
		}
	}

	if (!function_fail)
	{
		for (int i = 0; i <= (num_of_array - 1); i++)
			return_image[start_num + i] = C2D_SpriteSheetGetImage(sheet_texture[sheet_map_num], i);
	}
	return load_texture_result;
}

bool Draw_query_need_reflesh(void)
{
	Hid_info key;
	Hid_query_key_state(&key);

	if(Sem_query_settings(SEM_DEBUG_MODE) || draw_pre_p_touch_state != key.p_touch || draw_pre_h_touch_state != key.h_touch
		|| draw_pre_touch_x != key.touch_x || draw_pre_touch_y != key.touch_y	|| draw_pre_battery_charge != Menu_query_battery_charge()
		|| draw_pre_wifi_state != Menu_query_wifi_state() || draw_pre_battery_level_raw != Menu_query_battery_level_raw()
		|| draw_pre_battery_level != Menu_query_battery_level() || draw_pre_status != Menu_query_status(true)
		|| draw_pre_log_show != Log_query_log_show_flag() || draw_pre_error_show != Err_query_error_show_flag()
		|| (Log_query_log_show_flag() && Log_query_need_reflesh()) || (Err_query_error_show_flag() && Err_query_need_reflesh()))
	{
		draw_pre_log_show = Log_query_log_show_flag();
		draw_pre_error_show = Err_query_error_show_flag();
		draw_pre_battery_charge = Menu_query_battery_charge();
		draw_pre_wifi_state = Menu_query_wifi_state();
		draw_pre_battery_level_raw = Menu_query_battery_level_raw();
		draw_pre_battery_level = Menu_query_battery_level();
		draw_pre_status = Menu_query_status(true);
		draw_pre_p_touch_state = key.p_touch;
		draw_pre_h_touch_state = key.h_touch;
		draw_pre_touch_x = key.touch_x;
		draw_pre_touch_y = key.touch_y;
		return true;
	}
	else
		return false;
}

void Draw_touch_pos(void)
{
	Hid_info key;
	Hid_query_key_state(&key);
	if(key.p_touch || key.h_touch)
		Draw("●", key.touch_x, key.touch_y, 0.20, 0.20, 1.0, 0.0, 0.0, 1.0);
}

void Draw_top_ui(void)
{
	Draw_texture(Square_image, black_tint, 0, 0.0, 0.0, 400.0, 15.0);
	Draw_texture(Wifi_icon_image, dammy_tint, Menu_query_wifi_state(), 360.0, 0.0, 15.0, 15.0);
	Draw_texture(Battery_level_icon_image, dammy_tint, Menu_query_battery_level_raw() / 5, 315.0, 0.0, 30.0, 15.0);
	Draw_texture(ui_image, dammy_tint, (2 + Sem_query_settings(SEM_ECO_MODE)), 345.0, 0.0, 15.0, 15.0);
	if (Menu_query_battery_charge())
		Draw_texture(Battery_charge_icon_image, dammy_tint, 0, 295.0, 0.0, 20.0, 15.0);
	Draw(Menu_query_status(false), 0.0, 0.0, 0.45, 0.45, 0.0, 1.0, 0.0, 1.0);
	Draw(Menu_query_battery_level(), 322.5, 1.25, 0.4, 0.4, 0.0, 0.0, 0.0, 0.8);

	if (Sem_query_settings(SEM_DEBUG_MODE))
		Draw_debug_info();

	if (Log_query_log_show_flag())
		Draw_log(false);
}

void Draw_bot_ui(void)
{
	Draw_texture(Square_image, black_tint, 0, 0.0, 225.0, 320.0, 15.0);
	Draw("▽", 155.0, 220.0, 0.75, 0.75, 0.75, 0.75, 0.75, 1.0);
	if (Err_query_error_show_flag())
		Draw_error();
}

void Draw_texture(C2D_Image image[], int num, float x, float y, float x_size, float y_size)
{
	Draw_texture(image, dammy_tint, num, x, y, x_size, y_size);
}

void Draw_texture(C2D_Image image[], C2D_ImageTint tint, int num, float x, float y, float x_size, float y_size)
{
	C2D_DrawParams c2d_parameter =
	{
		{
			x,
			y,
			x_size,
			y_size
		},
		{
			0,
			0
		},
		0.0f,
		0.0f
	};

	if (!(image[num].tex == NULL))
	{
		if(tint.corners[0].color == dammy_tint.corners[0].color)
			C2D_DrawImage(image[num], &c2d_parameter, NULL);
		else
			C2D_DrawImage(image[num], &c2d_parameter, &tint);
	}
}

void Draw_error(void)
{
	Draw_texture(Square_image, aqua_tint, 0, 20.0, 30.0, 280.0, 150.0);
	Draw_texture(Square_image, weak_yellow_tint, 0, 150.0, 150.0, 20.0, 20.0);
	Draw_texture(Square_image, weak_yellow_tint, 0, 200.0, 150.0, 40.0, 20.0);

	Draw("Summary : ", 22.5, 40.0, 0.45, 0.45, 1.0, 0.0, 0.0, 1.0);
	Draw(Err_query_error_data(ERR_SUMMARY), 22.5, 50.0, 0.45, 0.45, 0.0, 0.0, 0.0, 1.0);
	Draw("Description : ", 22.5, 60.0, 0.45, 0.45, 1.0, 0.0, 0.0, 1.0);
	Draw(Err_query_error_data(ERR_DESCRIPTION), 22.5, 70.0, 0.4, 0.4, 0.0, 0.0, 0.0, 1.0);
	Draw("Place : ", 22.5, 90.0, 0.45, 0.45, 1.0, 0.0, 0.0, 1.0);
	Draw(Err_query_error_data(ERR_PLACE), 22.5, 100.0, 0.45, 0.45, 0.0, 0.0, 0.0, 1.0);
	Draw("Error code : ", 22.5, 110.0, 0.45, 0.45, 1.0, 0.0, 0.0, 1.0);
	Draw(Err_query_error_data(ERR_CODE), 22.5, 120.0, 0.45, 0.45, 0.0, 0.0, 0.0, 1.0);
	Draw("OK(A)", 152.5, 152.5, 0.375, 0.375, 0.0, 0.0, 0.0, 1.0);
	Draw("SAVE(X)", 202.5, 152.5, 0.375, 0.375, 0.0, 0.0, 0.0, 1.0);
}

void Draw_progress(std::string message)
{
	if (draw_do_not_draw)
		return;

	for(int i = 0; i < 2; i++)
	{
		Draw_frame_ready();
		if (Sem_query_settings(SEM_NIGHT_MODE))
			Draw_screen_ready_to_draw(0, true, 0, 0.0, 0.0, 0.0);
		else
			Draw_screen_ready_to_draw(0, true, 0, 1.0, 1.0, 1.0);

		Draw(message, 80.0, 110.0, 0.75, 0.75, 0.0, 1.0, 0.0, 1.0);

		Draw_apply_draw();
	}
}

void Draw_log(bool force_draw)
{
	if(force_draw)
	{
		Draw_frame_ready();
		if (Sem_query_settings(SEM_NIGHT_MODE))
			Draw_screen_ready_to_draw(0, true, 0, 0.0, 0.0, 0.0);
		else
			Draw_screen_ready_to_draw(0, true, 0, 1.0, 1.0, 1.0);
	}

	for (int i = 0; i < 23; i++)
		Draw(Log_query_log(Log_query_y() + i), Log_query_x(), 10.0 + (i * 10), 0.4, 0.4, 0.0, 1.0, 0.0, 1.0);

	if(force_draw)
		Draw_apply_draw();
}

void Draw_debug_info(void)
{
	float text_red;
	float text_green;
	float text_blue;
	float text_alpha;
	Hid_info key;
	Hid_query_key_state(&key);

	if (Sem_query_settings(SEM_NIGHT_MODE))
	{
		text_red = 1.0;
		text_green = 1.0;
		text_blue = 1.0;
		text_alpha = 0.75;
	}
	else
	{
		text_red = 0.0;
		text_green = 0.0;
		text_blue = 0.0;
		text_alpha = 1.0;
	}

	Draw_texture(Square_image, weak_blue_tint, 0, 0.0, 20.0, 70.0, 140.0);
	Draw_texture(Square_image, weak_blue_tint, 0, 0.0, 160.0, 110.0, 50.0);
	Draw("A　 p: " + std::to_string(key.p_a) + " h: " + std::to_string(key.h_a), 0.0, 20.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("B　 p: " + std::to_string(key.p_b) + " h: " + std::to_string(key.h_b), 0.0, 30.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("X　 p: " + std::to_string(key.p_x) + " h: " + std::to_string(key.h_x), 0.0, 40.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("Y　 p: " + std::to_string(key.p_y) + " h: " + std::to_string(key.h_y), 0.0, 50.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("L　 p: " + std::to_string(key.p_l) + " h: " + std::to_string(key.h_l), 0.0, 60.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("R　 p: " + std::to_string(key.p_r) + " h: " + std::to_string(key.h_r), 0.0, 70.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("C↓ p: " + std::to_string(key.p_c_down) + " h: " + std::to_string(key.h_c_down), 0.0, 80.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("C→ p: " + std::to_string(key.p_c_right) + " h: " + std::to_string(key.h_c_right), 0.0, 90.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("C↑ p: " + std::to_string(key.p_c_up) + " h: " + std::to_string(key.h_c_up), 0.0, 100.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("C← p: " + std::to_string(key.p_c_left) + " h: " + std::to_string(key.h_c_left), 0.0, 110.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("D↓ p: " + std::to_string(key.p_d_down) + " h: " + std::to_string(key.h_d_down), 0.0, 120.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("D→ p: " + std::to_string(key.p_d_right) + " h: " + std::to_string(key.h_d_right), 0.0, 130.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("D↑ p: " + std::to_string(key.p_d_up) + " h: " + std::to_string(key.h_d_up), 0.0, 140.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("D← p: " + std::to_string(key.p_d_left) + " h: " + std::to_string(key.h_d_left), 0.0, 150.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("touch x: " + std::to_string(key.touch_x) + ", y: " + std::to_string(key.touch_y), 0.0, 160.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("CPU: " + std::to_string(C3D_GetProcessingTime()).substr(0, 5) + "ms", 0.0, 170.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("GPU: " + std::to_string(C3D_GetDrawingTime()).substr(0, 5) + "ms", 0.0, 180.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("RAM: " + std::to_string((double)Menu_query_free_ram() / 10.0).substr(0, 5) + " MB", 0.0, 190.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
	Draw("linear RAM: " + std::to_string((double)Menu_query_free_linear_ram() / 1024.0 / 1024.0).substr(0, 5) +" MB", 0.0, 200.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);
}

void Draw_init(void)
{
	C2D_Prepare();
	screen[0] = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	screen[1] = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	screen[2] = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
	C2D_TargetClear(screen[0], C2D_Color32f(0, 0, 0, 0));
	C2D_TargetClear(screen[1], C2D_Color32f(0, 0, 0, 0));
	C2D_TargetClear(screen[2], C2D_Color32f(0, 0, 0, 0));
	dammy_tint.corners[0].color = 56738247;
	if (Sem_query_settings(SEM_NIGHT_MODE))
		C2D_PlainImageTint(&texture_tint, C2D_Color32f(1.0, 1.0, 1.0, 0.75), true);
	else
		C2D_PlainImageTint(&texture_tint, C2D_Color32f(0.0, 0.0, 0.0, 1.0), true);

	C2D_PlainImageTint(&white_tint, C2D_Color32f(1.0, 1.0, 1.0, 1.0), true);
	C2D_PlainImageTint(&weak_white_tint, C2D_Color32f(1.0, 1.0, 1.0, 0.3), true);
	C2D_PlainImageTint(&red_tint, C2D_Color32f(1.0, 0.0, 0.0, 1.0), true);
	C2D_PlainImageTint(&weak_red_tint, C2D_Color32f(1.0, 0.0, 0.0, 0.3), true);
	C2D_PlainImageTint(&aqua_tint, C2D_Color32f(0.0, 0.75, 1.0, 1.0), true);
	C2D_PlainImageTint(&weak_aqua_tint, C2D_Color32f(0.0, 0.75, 1.0, 0.3), true);
	C2D_PlainImageTint(&yellow_tint, C2D_Color32f(0.5, 0.5, 0.0, 1.0), true);
	C2D_PlainImageTint(&weak_yellow_tint, C2D_Color32f(0.5, 0.5, 0.0, 0.3), true);
	C2D_PlainImageTint(&blue_tint, C2D_Color32f(0.0, 0.0, 1.0, 1.0), true);
	C2D_PlainImageTint(&weak_blue_tint, C2D_Color32f(0.0, 0.0, 1.0, 0.3), true);
	C2D_PlainImageTint(&black_tint, C2D_Color32f(0.0, 0.0, 0.0, 1.0), true);
	C2D_PlainImageTint(&weak_black_tint, C2D_Color32f(0.0, 0.0, 0.0, 0.3), true);
	osTickCounterStart(&draw_frame_time_timer);
}

Result_with_string Draw_load_kanji_samples(void)
{
	int characters = 0;
	u8* fs_buffer = (u8*)malloc(0x8000);
	u32 read_size = 0;
	Result_with_string result;

	memset((void*)fs_buffer, 0x0, 0x8000);
	result = File_load_from_rom("kanji.txt", fs_buffer, 0x8000, &read_size, "romfs:/gfx/font/sample/");
	if(result.code == 0)
		Exfont_text_parse((char*)fs_buffer, draw_japanese_kanji, 3000, &characters);

	memset((void*)fs_buffer, 0x0, 0x8000);
	result = File_load_from_rom("hanyu_s.txt", fs_buffer, 0x8000, &read_size, "romfs:/gfx/font/sample/");
	if(result.code == 0)
		Exfont_text_parse((char*)fs_buffer, draw_simple_chinese, 6300, &characters);

	free(fs_buffer);
	return result;
}

Result_with_string Draw_load_system_font(int system_font_num)
{
	Result_with_string result;
	if (system_font_num == 0)
		system_fonts[0] = C2D_FontLoadSystem(CFG_REGION_JPN);
	else if (system_font_num == 1)
		system_fonts[1] = C2D_FontLoadSystem(CFG_REGION_CHN);
	else if (system_font_num == 2)
		system_fonts[2] = C2D_FontLoadSystem(CFG_REGION_KOR);
	else if (system_font_num == 3)
		system_fonts[3] = C2D_FontLoadSystem(CFG_REGION_TWN);
	else
	{
		result.code = INVALID_ARG;
		result.string = Err_query_template_summary(INVALID_ARG);
		result.error_description = Err_query_template_detail(INVALID_ARG);
		return result;
	}

	if(system_fonts[system_font_num] == NULL)
	{
		result.code = -1;
		result.string = "[Error] Couldn't load font file : " + std::to_string(system_font_num) + " ";
	}
	return result;
}

void Draw_free_system_font(int system_font_num)
{
	if (system_font_num >= 0 && system_font_num <= 3)
	{
		if (system_fonts[system_font_num] != NULL)
		{
			C2D_FontFree(system_fonts[system_font_num]);
			system_fonts[system_font_num] = NULL;
		}
	}
}

void Draw_free_texture(int sheet_map_num)
{
	if (sheet_texture[sheet_map_num] != NULL)
	{
		C2D_SpriteSheetFree(sheet_texture[sheet_map_num]);
		sheet_texture[sheet_map_num] = NULL;
	}
}

void Draw_exit(void)
{
	for (int i = 0; i < 128; i++)
		Draw_free_texture(i);
	for (int i = 0; i < 4; i++)
		Draw_free_system_font(i);
}

void Draw_frame_ready(void)
{
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
}

void Draw_screen_ready_to_draw(int screen_num, bool screen_clear, int screen_clear_ver, float red, float green, float blue)
{
	if (screen_num >= 0 && screen_num <= 2)
	{
		C2D_SceneBegin(screen[screen_num]);
		if (screen_clear)
			C2D_TargetClear(screen[screen_num], C2D_Color32f(red, green, blue, 0));
	}
}

void Draw_apply_draw(void)
{
	C3D_FrameEnd(0);
	draw_fps++;
	osTickCounterUpdate(&draw_frame_time_timer);
	draw_frametime = osTickCounterRead(&draw_frame_time_timer);
}

/*static inline size_t fmtSize(GPU_TEXCOLOR fmt)
{
	switch (fmt)
	{
	case GPU_RGBA8:
		return 32;
	case GPU_RGB8:
		return 24;
	case GPU_RGBA5551:
	case GPU_RGB565:
	case GPU_RGBA4:
	case GPU_LA8:
	case GPU_HILO8:
		return 16;
	case GPU_L8:
	case GPU_A8:
	case GPU_LA4:
	case GPU_ETC1A4:
		return 8;
	case GPU_L4:
	case GPU_A4:
	case GPU_ETC1:
		return 4;
	default:
		return 0;
	}
}

bool Moded_C3D_TexInitWithParams(C3D_Tex* tex, C3D_TexCube* cube, C3D_TexInitParams p)
{
	u32 size = fmtSize(p.format);
	if (!size) return false;
	size *= (u32)p.width * p.height / 8;
	u32 total_size = C3D_TexCalcTotalSize(size, p.maxLevel);

	tex->data = aligned_alloc(0x80, total_size);
	if (!tex->data) return false;

	tex->width = p.width;
	tex->height = p.height;
	tex->param = GPU_TEXTURE_MODE(p.type);
	if (p.format == GPU_ETC1)
		tex->param |= GPU_TEXTURE_ETC1_PARAM;
	if (p.type == GPU_TEX_SHADOW_2D || p.type == GPU_TEX_SHADOW_CUBE)
		tex->param |= GPU_TEXTURE_SHADOW_PARAM;
	tex->fmt = p.format;
	tex->size = size;
	tex->border = 0;
	tex->lodBias = 0;
	tex->maxLevel = p.maxLevel;
	tex->minLevel = 0;
	return true;
}*/
