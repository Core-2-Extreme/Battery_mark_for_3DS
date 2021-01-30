#include <3ds.h>
#include <string>

#include "headers.hpp"

#include "menu.hpp"
#include "setting_menu.hpp"

bool err_error_display;
bool err_previous_error_display;
std::string err_error_summary = "N/A";
std::string err_previous_error_summary = "N/A";
std::string err_error_description = "N/A";
std::string err_previous_error_description = "N/A";
std::string err_error_place = "N/A";
std::string err_previous_error_place = "N/A";
std::string err_error_code = "0x0";
std::string err_previous_error_code = "0x0";

bool Err_query_error_show_flag(void)
{
	return err_error_display;
}

std::string Err_query_error_data(int error_num)
{
	if (error_num == ERR_SUMMARY)
		return err_error_summary;
	else if (error_num == ERR_DESCRIPTION)
		return err_error_description;
	else if (error_num == ERR_PLACE)
		return err_error_place;
	else if (error_num == ERR_CODE)
		return err_error_code;
	else
		return "";
}

std::string Err_query_template_summary(long error_code)
{
	if(error_code == 0)
		return "[Success] ";
	if(error_code == OUT_OF_MEMORY)
		return "[Error] Out of memory. ";
	else if(error_code == OUT_OF_LINEAR_MEMORY)
		return "[Error] Out of linear memory. ";
	else if(error_code == GAS_RETURNED_NOT_SUCCESS)
		return "[Error] GAS returned NOT success. ";
	else if(error_code == STB_IMG_RETURNED_NOT_SUCCESS)
		return "[Error] stb image returned NOT success. ";
	else if(error_code == BUFFER_SIZE_IS_TOO_SMALL)
		return "[Error] Buffer size is too small. ";
	else if(error_code == WRONG_PARSING_POS)
		return "[Error] Wrong parsing position. ";
	else if(error_code == TOO_MANY_MESSAGES)
		return "[Error] Too many messages. ";
	else if(error_code == INVALID_ARG)
		return "[Error] Invalid arg. ";
	else if(error_code == BAD_ICON_INFO)
		return "[Error] Bad icon info. ";
	else if(error_code == NO_ICON_INFO)
		return "[Error] No icon info. ";
	else if(error_code == FFMPEG_RETURNED_NOT_SUCCESS)
		return "[Error] ffmpeg returned NOT success. ";
	else if(error_code == INVALID_PORT_NUM)
		return "[Error] Invalid port num was specified. ";
	else
		return "[Error] ";
}

std::string Err_query_template_detail(long error_code)
{
	if(error_code == 0)
		return "";
	if(error_code == OUT_OF_MEMORY)
		return "Couldn't allocate memory.";
	else if(error_code == OUT_OF_LINEAR_MEMORY)
		return "Couldn't allocate linear memory.";
	else if(error_code == BUFFER_SIZE_IS_TOO_SMALL)
		return "Buffer size is too small.\nPlease increase buffer size from settings.";
	else if(error_code == WRONG_PARSING_POS)
		return "Wrong parsing position.";
	else if(error_code == TOO_MANY_MESSAGES)
		return "Parsing aborted due to too many messages.";
	else if(error_code == INVALID_ARG)
		return "Invalid arg was specified.";
	else if(error_code == BAD_ICON_INFO)
		return "Bad icon info.";
	else if(error_code == NO_ICON_INFO)
		return "There is No icon info.";
	else
		return "";
}


bool Err_query_need_reflesh(void)
{
	bool need = false;

	if(err_previous_error_display != err_error_display)
	{
		err_previous_error_display = err_error_display;
		need = true;
	}

	if(err_error_summary != err_previous_error_summary)
	{
		err_previous_error_summary = err_error_summary;
		need = true;
	}

	if(err_error_description != err_previous_error_description)
	{
		err_previous_error_description = err_error_description;
		need = true;
	}

	if(err_error_place != err_previous_error_place)
	{
		err_previous_error_place = err_error_place;
		need = true;
	}

	if(err_error_code != err_previous_error_code)
	{
		err_previous_error_code = err_error_code;
		need = true;
	}

	return need;
}

void Err_save_error(void)
{
	File_save_to_file(Menu_query_time(0) + ".txt", (u8*)(err_error_summary + "\n" + err_error_description + "\n" + err_error_place + "\n" + err_error_code).c_str(), (err_error_summary + "\n" + err_error_description + "\n" + err_error_place + "\n" + err_error_code).length()
, Sem_query_main_dir() + "error/", true);
	err_error_display = false;
}

void Err_set_error_code(long error_code)
{
	if (error_code == 1234567890)
		err_error_code = "N/A";
	else
		err_error_code = Err_dec_to_hex_string(error_code);
}

void Err_set_error_data(int error_num, std::string error_data)
{
	if (error_num == ERR_SUMMARY)
		err_error_summary = error_data;
	else if (error_num == ERR_DESCRIPTION)
		err_error_description = error_data;
	else if (error_num == ERR_PLACE)
		err_error_place = error_data;
}

void Err_set_error_message(std::string summary, std::string description, std::string place, long error_code)
{
	Err_clear_error_message();
	err_error_summary = summary;
	err_error_description = description;
	err_error_place = place;
	if (error_code == 1234567890)
		err_error_code = "N/A";
	else
		err_error_code = Err_dec_to_hex_string(error_code);
}

void Err_set_error_show_flag(bool flag)
{
	err_error_display = flag;
}

void Err_clear_error_message(void)
{
	err_error_summary = "N/A";
	err_error_description = "N/A";
	err_error_place = "N/A";
	err_error_code = Err_dec_to_hex_string(0);
}

std::string Err_dec_to_hex_string(long dec)
{
	char hex_string[128];
	sprintf(hex_string, "0x%lx", dec);
	return hex_string;
}
