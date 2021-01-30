#pragma once

#define ERR_SUMMARY 0
#define ERR_DESCRIPTION 1
#define ERR_PLACE 2
#define ERR_CODE 3

#define OUT_OF_MEMORY -1
#define OUT_OF_LINEAR_MEMORY -2
#define GAS_RETURNED_NOT_SUCCESS -3
#define STB_IMG_RETURNED_NOT_SUCCESS -4
#define BUFFER_SIZE_IS_TOO_SMALL -5
#define WRONG_PARSING_POS -6
#define TOO_MANY_MESSAGES -7
#define INVALID_ARG -8
#define BAD_ICON_INFO -9
#define NO_ICON_INFO -10
#define FFMPEG_RETURNED_NOT_SUCCESS -11
#define INVALID_PORT_NUM -12

bool Err_query_error_show_flag(void);

std::string Err_query_error_data(int error_num);

void Err_save_error(void);

void Err_set_error_code(long error_code);

void Err_set_error_data(int error_num, std::string error_data);

bool Err_query_need_reflesh(void);

std::string Err_query_template_summary(long error_code);

std::string Err_query_template_detail(long error_code);

void Err_set_error_message(std::string summary, std::string description, std::string place, long error_code);

void Err_set_error_show_flag(bool flag);

void Err_clear_error_message(void);

std::string Err_dec_to_hex_string(long dec);
