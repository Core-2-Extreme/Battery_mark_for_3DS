#pragma once
#include "types.hpp"

#define EXFONT_NUM_OF_FONT_NAME 50

void Exfont_init(void);

std::string Exfont_query_font_name(int exfont_num);

void Exfont_set_msg(int msg_num, std::string msg);

std::string Exfont_text_sort(std::string sorce_part_string[], int max_loop);

void Exfont_text_parse(std::string sorce_string, std::string part_string[], int max_loop, int* out_element);

void Exfont_draw_external_fonts(std::string string, float texture_x, float texture_y, float texture_size_x, float texture_size_y, float red, float green, float blue, float alpha, float* out_width, float* out_height);

Result_with_string Exfont_load_exfont(int exfont_num);

void Exfont_unload_exfont(int exfont_num);
