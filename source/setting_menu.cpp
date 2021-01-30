#include <3ds.h>
#include <string>
#include <unistd.h>

#include "headers.hpp"

#include "setting_menu.hpp"

#include "menu.hpp"
#include "battery_mark.hpp"
#include "battery_mark_ranking.hpp"

/*For draw*/
bool sem_need_reflesh = false;
bool sem_pre_use_default_font = true;
bool sem_pre_use_system_specific_font = false;
bool sem_pre_use_external_font = false;
bool sem_pre_loaded_external_font[EXFONT_NUM_OF_FONT_NAME];
bool sem_pre_load_external_font_request = false;
bool sem_pre_unload_external_font_request = false;
bool sem_pre_show_patch_note_request = false;
bool sem_pre_select_ver_request = false;
bool sem_pre_allow_send_app_info = false;
bool sem_pre_debug_mode = false;
bool sem_pre_night_mode = false;
bool sem_pre_eco_mode = true;
u32 sem_pre_dled_size = 0;
int sem_pre_lcd_brightness = 100;
int sem_pre_time_to_turn_off_lcd = 1500;
int sem_pre_lcd_brightness_before_turn_off = 20;
int sem_pre_update_progress = -1;
int sem_pre_check_update_progress = 0;
int sem_pre_selected_edition_num = 0;
int sem_pre_selected_menu_mode = 0;
double sem_pre_scroll_speed = 0.5;
double sem_pre_y_offset = 0.0;
std::string sem_pre_lang = "en";
/*---------------------------------------------*/

bool sem_loaded_external_font[EXFONT_NUM_OF_FONT_NAME];
bool sem_load_external_font[EXFONT_NUM_OF_FONT_NAME];
bool sem_check_update_thread_run = false;
bool sem_worker_thread_run = false;
bool sem_reload_msg_request = false;
bool sem_load_system_font_request = false;
bool sem_load_external_font_request = false;
bool sem_unload_external_font_request = false;
bool sem_change_wifi_state_request = false;
bool sem_delete_line_img_cache_request = false;
bool sem_delete_line_audio_cache_request = false;
bool sem_delete_line_vid_cache_request = false;
bool setting_main_run = false;
bool new_version_available = false;
bool sem_already_init = false;
bool sem_main_run = false;
bool sem_thread_suspend = false;
bool sem_check_update_request = false;
bool sem_show_patch_note_request = false;
bool sem_select_ver_request = false;
bool sem_available_ver[8];
bool sem_dl_file_request = false;
bool sem_allow_send_app_info = false;
bool sem_debug_mode = false;
bool sem_night_mode = false;
bool sem_flash_mode = false;
bool sem_eco_mode = true;
bool sem_wifi_enabled = false;
bool sem_system_setting_menu_show = false;
bool sem_scroll_bar_selected = false;
bool sem_scroll_mode = false;
bool sem_unload_system_font_request = false;
bool sem_bar_selected[9];
bool sem_button_selected[EXFONT_NUM_OF_FONT_NAME];
bool sem_loaded_system_font[4] = { false, false, false, false, };
bool sem_load_system_font[4] = { false, false, false, false, };
u32 sem_dled_size = 0;
int sem_lcd_brightness = 100;
int sem_time_to_turn_off_lcd = 1500;
int sem_lcd_brightness_before_turn_off = 20;
int sem_selected_menu_mode = 0;
int sem_update_progress = -1;
int sem_check_update_progress = 0;
int sem_selected_edition_num = 0;
int sem_current_app_ver = 1;
int sem_num_of_app_start = 0;
int sem_system_region = 0;
int sem_installed_size = 0;
int sem_total_cia_size = 0;
double sem_scroll_speed = 0.5;
double sem_y_offset = 0.0;
double sem_y_max = 0.0;
double sem_touch_x_move_left = 0.0;
double sem_touch_y_move_left = 0.0;
std::string sem_lang = "en";
std::string sem_msg[SEM_NUM_OF_MSG];
std::string sem_newest_ver_data[18];
std::string sem_update_dir = "Battery_mark";
std::string sem_update_file = "Battery_mark_for_3DS";
std::string sem_update_url = "https://script.google.com/macros/s/AKfycbz8Nsobjwc9IslWkIEB_2gyc76bTyJFwod2SZJbeVuhu_HbfQiszty8/exec";
std::string	sem_setting_folder = "/Battery_mark/";
std::string sem_init_string = "Sem/Init";
std::string sem_exit_string = "Sem/Exit";
std::string sem_worker_thread_string = "Sem/Worker thread";
std::string sem_check_update_string = "Sem/Update thread";

C2D_Image sem_help_image[7];
Thread sem_check_update_thread, sem_worker_thread;

Result_with_string Sem_load_setting(std::string file_name, std::string dir_name, int item_num, std::string out_data[])
{
	u32 read_size;
	u8* fs_buffer;
	Result_with_string result;
	fs_buffer = (u8*)malloc(0x2000);
	memset(fs_buffer, 0x0, 0x2000);

	result = File_load_from_file(file_name, fs_buffer, 0x2000, &read_size, dir_name);

	if (result.code == 0)
		result = Sem_parse_file((char*)fs_buffer, item_num, out_data);

	free(fs_buffer);
	return result;
}

Result_with_string Sem_parse_file(std::string source_data, int num_of_items, std::string out_data[])
{
	Result_with_string result;

	size_t parse_start_num = 0;
	size_t parse_end_num = 0;
	std::string parse_start_text;
	std::string parse_end_text;

	for (int i = 0; i < num_of_items; i++)
	{
		parse_start_text = "<" + std::to_string(i) + ">";
		parse_start_num = source_data.find(parse_start_text);
		parse_end_text = "</" + std::to_string(i) + ">";
		parse_end_num = source_data.find(parse_end_text);

		if (parse_end_num == std::string::npos || parse_start_num == std::string::npos)
		{
			result.code = -9948;
			result.string = "[Error] Failed to load settings. error pos : " + std::to_string(i) + " ";
			break;
		}

		parse_start_num += parse_start_text.length();
		parse_end_num -= parse_start_num;
		out_data[i] = source_data.substr(parse_start_num, parse_end_num);
	}

	return result;
}

std::string Sem_convert_seconds_to_time(double input_seconds)
{
	int hours = 0;
	int minutes = 0;
	int seconds = 0;
	long count = 0;
	std::string time = "";

	for(count = 0; count < (int)input_seconds; count++)
	{
		if(seconds + 1 >= 60)
		{
			if(minutes + 1 >= 60)
			{
				seconds = 0;
				minutes = 0;
				hours++;
			}
			else
			{
				seconds = 0;
				minutes++;
			}
		}
		else
			seconds++;
	}

	if(hours != 0)
		time += std::to_string(hours) + ":";

	if(minutes < 10)
		time += "0" + std::to_string(minutes) + ":";
	else
		time += std::to_string(minutes) + ":";

	if(seconds < 10)
		time += "0" + std::to_string(seconds);
	else
		time += std::to_string(seconds);

	time += std::to_string(input_seconds - count + 1).substr(1, 2);
	return time;
}

std::string Sem_encode_to_escape(std::string in_data)
{
	int string_length = in_data.length();
	std::string check;
	std::string return_data = "";

	for(int i = 0; i < string_length; i++)
	{
		check = in_data.substr(i, 1);
		if(check == "\n")
			return_data += "\\n";
		else if(check == "\u0022")
			return_data += "\\\u0022";
		else if(check == "\u005c")
			return_data += "\\\u005c";
		else
			return_data += in_data.substr(i, 1);
	}

	return return_data;
}

bool Sem_query_init_flag(void)
{
	return sem_already_init;
}

bool Sem_query_running_flag(void)
{
	return sem_main_run;
}

int Sem_query_app_ver(void)
{
	return sem_current_app_ver;
}

std::string Sem_query_main_dir(void)
{
	return sem_setting_folder;
}

std::string Sem_query_lang(void)
{
	return sem_lang;
}

bool Sem_query_loaded_external_font_flag(int external_font_num)
{
	if (external_font_num >= 0 && external_font_num <= EXFONT_NUM_OF_FONT_NAME)
		return sem_loaded_external_font[external_font_num];
	else
		return false;
}

bool Sem_query_loaded_system_font_flag(int system_font_num)
{
	if(system_font_num == sem_system_region)
		return true;
	else if (system_font_num >= 0 && system_font_num <= 3)
		return sem_load_system_font[system_font_num];
	else
		return false;
}

bool Sem_query_operation_flag(int operation_num)
{
	if (operation_num == SEM_CHECK_UPDATE_REQUEST)
		return sem_check_update_request;
	else if (operation_num == SEM_SHOW_PATCH_NOTE_REQUEST)
		return sem_show_patch_note_request;
	else if (operation_num == SEM_SELECT_VER_REQUEST)
		return sem_select_ver_request;
	else if (operation_num == SEM_DL_FILE_REQUEST)
		return sem_dl_file_request;
	else if (operation_num == SEM_LOAD_SYSTEM_FONT_REQUEST)
		return sem_load_system_font_request;
	else if (operation_num == SEM_LOAD_EXTERNAL_FONT_REQUEST)
		return sem_load_external_font_request;
	else if (operation_num == SEM_UNLOAD_EXTERNAL_FONT_REQUEST)
		return sem_unload_external_font_request;
	else if (operation_num == SEM_RELOAD_MSG_REQUEST)
		return sem_reload_msg_request;
	else if (operation_num == SEM_CHANGE_WIFI_STATE_REQUEST)
		return sem_change_wifi_state_request;
	else
		return false;
}

bool Sem_query_settings(int item_num)
{
	if (item_num == SEM_NIGHT_MODE)
		return sem_night_mode;
	else if (item_num == SEM_FLASH_MODE)
		return sem_flash_mode;
	else if (item_num == SEM_DEBUG_MODE)
		return sem_debug_mode;
	else if (item_num == SEM_ALLOW_SEND_APP_INFO)
		return sem_allow_send_app_info;
	else if (item_num == SEM_WIFI_ENABLED)
		return sem_wifi_enabled;
	else if(item_num == SEM_SYSTEM_SETTING_MENU_SHOW)
		return sem_system_setting_menu_show;
	else if(item_num == SEM_ECO_MODE)
		return sem_eco_mode;
	else
		return false;
}

int Sem_query_settings_i(int item_num)
{
	if(item_num == SEM_LCD_BRIGHTNESS)
		return sem_lcd_brightness;
	else if(item_num == SEM_TIME_TO_TURN_OFF_LCD)
		return sem_time_to_turn_off_lcd;
	else if(item_num == SEM_LCD_BRIGHTNESS_BEFORE_TURN_OFF)
		return sem_lcd_brightness_before_turn_off;
	else if(item_num == SEM_NUM_OF_APP_START)
		return sem_num_of_app_start;
	else
	return -1;
}

double Sem_query_settings_d(int item_num)
{
	if(item_num == SEM_SCROLL_SPEED)
		return sem_scroll_speed;
	else
		return -1;
}

void Sem_set_load_external_font_request(int external_font_num, bool flag)
{
	if (external_font_num >= 0 && external_font_num <= EXFONT_NUM_OF_FONT_NAME)
		sem_load_external_font[external_font_num] = flag;
}

void Sem_set_load_system_font_request(int system_font_num, bool flag)
{
	if (system_font_num >= 0 && system_font_num <= 3)
		sem_load_system_font[system_font_num] = flag;
}

void Sem_set_msg(int msg_num, std::string msg)
{
	if (msg_num >= 0 && msg_num < SEM_NUM_OF_MSG)
		sem_msg[msg_num] = msg;
}

void Sem_set_operation_flag(int operation_num, bool flag)
{
	if (operation_num == SEM_CHECK_UPDATE_REQUEST)
		sem_check_update_request = flag;
	else if (operation_num == SEM_SHOW_PATCH_NOTE_REQUEST)
		sem_show_patch_note_request = flag;
	else if (operation_num == SEM_SELECT_VER_REQUEST)
		sem_select_ver_request = flag;
	else if (operation_num == SEM_DL_FILE_REQUEST)
		sem_dl_file_request = flag;
	else if (operation_num == SEM_LOAD_SYSTEM_FONT_REQUEST)
		sem_load_system_font_request = flag;
	else if (operation_num == SEM_LOAD_EXTERNAL_FONT_REQUEST)
		sem_load_external_font_request = flag;
	else if (operation_num == SEM_UNLOAD_EXTERNAL_FONT_REQUEST)
		sem_unload_external_font_request = flag;
	else if (operation_num == SEM_RELOAD_MSG_REQUEST)
		sem_reload_msg_request = flag;
	else if (operation_num == SEM_CHANGE_WIFI_STATE_REQUEST)
		sem_change_wifi_state_request = flag;
}

void Sem_set_settings(int item_num, bool flag)
{
	if (item_num == SEM_NIGHT_MODE)
		sem_night_mode = flag;
	else if (item_num == SEM_FLASH_MODE)
		sem_flash_mode = flag;
	else if (item_num == SEM_DEBUG_MODE)
		sem_debug_mode = flag;
	else if (item_num == SEM_ALLOW_SEND_APP_INFO)
		sem_allow_send_app_info = flag;
	else if (item_num == SEM_WIFI_ENABLED)
		sem_wifi_enabled = flag;
	else if(item_num == SEM_SYSTEM_SETTING_MENU_SHOW)
		sem_system_setting_menu_show = flag;
	else if(item_num == SEM_ECO_MODE)
		sem_eco_mode = flag;
}

void Sem_set_settings_i(int item_num, int value)
{
	if(item_num == SEM_LCD_BRIGHTNESS)
		sem_lcd_brightness = value;
	else if(item_num == SEM_TIME_TO_TURN_OFF_LCD)
		sem_time_to_turn_off_lcd = value;
	else if(item_num == SEM_LCD_BRIGHTNESS_BEFORE_TURN_OFF)
		sem_lcd_brightness_before_turn_off = value;
	else if(item_num == SEM_NUM_OF_APP_START)
		sem_num_of_app_start = value;
}

void Sem_set_settings_d(int item_num, double value)
{
	if(item_num == SEM_SCROLL_SPEED)
		sem_scroll_speed = value;
}

void Sem_set_color(double in_red, double in_green, double in_blue, double in_alpha, double* out_red, double* out_green, double* out_blue, double* out_alpha, int num_of_out)
{
	for (int i = 0; i < num_of_out; i++)
	{
		out_red[i] = in_red;
		out_blue[i] = in_blue;
		out_green[i] = in_green;
		out_alpha[i] = in_alpha;
	}
}

void Sem_suspend(void)
{
	Menu_resume();
	sem_thread_suspend = true;
	sem_main_run = false;
}

void Sem_resume(void)
{
	sem_thread_suspend = false;
	sem_main_run = true;
	sem_need_reflesh = true;
	Menu_suspend();
}

void Sem_init(void)
{
	Log_log_save(sem_init_string, "Initializing...", 1234567890, FORCE_DEBUG);
	u8 region = 0;
	int log_num = 0;
	std::string data[10];
	Result_with_string result;

	sem_lang = "en";
	sem_lcd_brightness = 100;
	sem_time_to_turn_off_lcd = 1500;
	sem_lcd_brightness_before_turn_off = 15;
	sem_system_setting_menu_show = false;
	sem_scroll_speed = 0.5;
	sem_allow_send_app_info = false;
	sem_num_of_app_start = 0;
	sem_night_mode = false;
	sem_eco_mode = true;

	Draw_progress("[Sem] Loading settings...");
	log_num = Log_log_save(sem_init_string , "Sem_load_setting()...", 1234567890, FORCE_DEBUG);
	result = Sem_load_setting("Sem_setting.txt", sem_setting_folder, 9, data);
	Log_log_add(log_num, result.string, result.code, FORCE_DEBUG);
	if(result.code == 0)
	{
		sem_lang = data[0];
		sem_lcd_brightness = atoi(data[1].c_str());
		sem_time_to_turn_off_lcd = atoi(data[2].c_str());
		sem_lcd_brightness_before_turn_off = atoi(data[3].c_str());
		sem_scroll_speed = strtod(data[4].c_str(), NULL);
		sem_allow_send_app_info = (data[5] == "1");
		sem_num_of_app_start = atoi(data[6].c_str());
		sem_night_mode = (data[7] == "1");
		sem_eco_mode = (data[8] == "1");

		if(sem_lang != "jp" && sem_lang != "en")
			sem_lang = "en";
		if(sem_lcd_brightness < 15 || sem_lcd_brightness > 163)
			sem_lcd_brightness = 100;
		if(sem_time_to_turn_off_lcd < 100 || sem_time_to_turn_off_lcd > 3090)
			sem_time_to_turn_off_lcd = 1500;
		if(sem_lcd_brightness_before_turn_off < 15 || sem_lcd_brightness_before_turn_off > 163)
			sem_lcd_brightness_before_turn_off = 15;
		if(sem_scroll_speed < 0.033 || sem_scroll_speed > 1.030)
			sem_scroll_speed = 0.5;
		if(sem_num_of_app_start < 0)
			sem_num_of_app_start = 0;
	}

	result.code = CFGU_SecureInfoGetRegion(&region);
	if(result.code == 0)
	{
		if(region == CFG_REGION_CHN)
			sem_system_region = 1;
		else if(region == CFG_REGION_KOR)
			sem_system_region = 2;
		else if(region == CFG_REGION_TWN)
			sem_system_region = 3;
		else
			sem_system_region = 0;
	}

	Draw_progress("[Sem] Starting threads...");
	sem_check_update_thread_run = true;
	sem_worker_thread_run = true;
	sem_check_update_thread = threadCreate(Sem_check_update_thread, (void*)(""), STACKSIZE, PRIORITY_NORMAL, 0, false);
	sem_worker_thread = threadCreate(Sem_worker_thread, (void*)(""), STACKSIZE, PRIORITY_NORMAL, 0, false);

	Sem_resume();
	sem_already_init = true;
	Log_log_save(sem_init_string, "Initialized.", 1234567890, FORCE_DEBUG);
}

void Sem_exit(void)
{
	Log_log_save(sem_exit_string, "Exiting...", 1234567890, FORCE_DEBUG);
	u64 time_out = 10000000000;
	int log_num;
	sem_num_of_app_start++;
	std::string data = "<0>" + sem_lang + "</0><1>" + std::to_string(sem_lcd_brightness) + "</1><2>" + std::to_string(sem_time_to_turn_off_lcd)
	+ "</2><3>" + std::to_string(sem_lcd_brightness_before_turn_off) + "</3><4>" + std::to_string(sem_scroll_speed) + "</4><5>" + std::to_string(sem_allow_send_app_info)
	+ "</5><6>" + std::to_string(sem_num_of_app_start) + "</6><7>" + std::to_string(sem_night_mode) + "</7><8>" + std::to_string(sem_eco_mode)
	+ "</8>";
	Result_with_string result;

	Draw_progress("[Sem] Exiting...");
	sem_already_init = false;
	sem_thread_suspend = false;
	sem_check_update_thread_run = false;
	sem_worker_thread_run = false;

	log_num = Log_log_save(sem_exit_string, "File_save_to_file()...", 1234567890, FORCE_DEBUG);
	result = File_save_to_file("Sem_setting.txt", (u8*)data.c_str(), data.length(), sem_setting_folder, true);
	Log_log_add(log_num, result.string, result.code, FORCE_DEBUG);

	log_num = Log_log_save(sem_exit_string, "threadJoin()0/1...", 1234567890, FORCE_DEBUG);
	result.code = threadJoin(sem_check_update_thread, time_out);
	if (result.code == 0)
		Log_log_add(log_num, Err_query_template_summary(0), result.code, FORCE_DEBUG);
	else
		Log_log_add(log_num, Err_query_template_summary(-1024), result.code, FORCE_DEBUG);

	log_num = Log_log_save(sem_exit_string, "threadJoin()1/1...", 1234567890, FORCE_DEBUG);
	result.code = threadJoin(sem_worker_thread, time_out);
	if (result.code == 0)
		Log_log_add(log_num, Err_query_template_summary(0), result.code, FORCE_DEBUG);
	else
		Log_log_add(log_num, Err_query_template_summary(-1024), result.code, FORCE_DEBUG);

	threadFree(sem_check_update_thread);
	threadFree(sem_worker_thread);

	Log_log_save(sem_exit_string, "Exited.", 1234567890, FORCE_DEBUG);
}

void Sem_main(void)
{
	int msg_num = 0;
	double draw_x;
	double draw_y;
	double size_x_offset;
	double text_red, text_green, text_blue, text_alpha;
	double red[EXFONT_NUM_OF_FONT_NAME], green[EXFONT_NUM_OF_FONT_NAME], blue[EXFONT_NUM_OF_FONT_NAME], alpha[EXFONT_NUM_OF_FONT_NAME];
	C2D_ImageTint draw_tint[2];
	Hid_info key;

	if (sem_night_mode)
	{
		text_red = 1.0;
		text_green = 1.0;
		text_blue = 1.0;
		text_alpha = 0.75;
		white_or_black_tint = white_tint;
	}
	else
	{
		text_red = 0.0;
		text_green = 0.0;
		text_blue = 0.0;
		text_alpha = 1.0;
		white_or_black_tint = black_tint;
	}
	Sem_set_color(text_red, text_green, text_blue, text_alpha, red, green, blue, alpha, EXFONT_NUM_OF_FONT_NAME);

	for (int i = 0; i < EXFONT_NUM_OF_FONT_NAME; i++)
	{
		if (sem_pre_loaded_external_font[i] != sem_loaded_external_font[i])
		{
			sem_need_reflesh = true;
			break;
		}
	}

	if(sem_need_reflesh || sem_pre_y_offset != sem_y_offset || sem_pre_lang != sem_lang || sem_pre_night_mode != sem_night_mode
	|| sem_pre_lcd_brightness != sem_lcd_brightness || sem_pre_time_to_turn_off_lcd != sem_time_to_turn_off_lcd
	|| sem_pre_lcd_brightness_before_turn_off != sem_lcd_brightness_before_turn_off
	|| sem_pre_scroll_speed != sem_scroll_speed	|| sem_pre_unload_external_font_request != sem_unload_external_font_request
	|| sem_pre_load_external_font_request != sem_load_external_font_request || sem_pre_allow_send_app_info != sem_allow_send_app_info
	|| sem_pre_debug_mode != sem_debug_mode || sem_pre_eco_mode != sem_eco_mode || sem_pre_select_ver_request != sem_select_ver_request
	|| sem_pre_show_patch_note_request != sem_show_patch_note_request	|| sem_pre_check_update_progress != sem_check_update_progress
	|| sem_pre_update_progress != sem_update_progress || sem_pre_selected_edition_num != sem_selected_edition_num
	|| sem_pre_selected_menu_mode != sem_selected_menu_mode || sem_pre_dled_size != sem_dled_size)
	{
		for (int i = 0; i < EXFONT_NUM_OF_FONT_NAME; i++)
			sem_pre_loaded_external_font[i] = sem_loaded_external_font[i];

		sem_pre_y_offset = sem_y_offset;
		sem_pre_lang = sem_lang;
		sem_pre_night_mode = sem_night_mode;
		sem_pre_lcd_brightness = sem_lcd_brightness;
		sem_pre_time_to_turn_off_lcd = sem_time_to_turn_off_lcd;
		sem_pre_lcd_brightness_before_turn_off = sem_lcd_brightness_before_turn_off;
		sem_pre_scroll_speed = sem_scroll_speed;
		sem_pre_unload_external_font_request = sem_unload_external_font_request;
		sem_pre_load_external_font_request = sem_load_external_font_request;
		sem_pre_allow_send_app_info = sem_allow_send_app_info;
		sem_pre_debug_mode = sem_debug_mode;
		sem_pre_eco_mode = sem_eco_mode;
		sem_pre_select_ver_request = sem_select_ver_request;
		sem_pre_show_patch_note_request = sem_show_patch_note_request;
		sem_pre_check_update_progress = sem_check_update_progress;
		sem_pre_update_progress = sem_update_progress;
		sem_pre_selected_edition_num = sem_selected_edition_num;
		sem_pre_selected_menu_mode = sem_selected_menu_mode;
		sem_pre_dled_size = sem_dled_size;
		sem_need_reflesh = true;
	}

	Hid_query_key_state(&key);
	Log_main();
	if(Draw_query_need_reflesh() || !sem_eco_mode)
		sem_need_reflesh = true;

	Hid_key_flag_reset();

	if(sem_need_reflesh)
	{
		Draw_frame_ready();
		if (sem_night_mode)
			Draw_screen_ready_to_draw(0, true, 2, 0.0, 0.0, 0.0);
		else
			Draw_screen_ready_to_draw(0, true, 2, 1.0, 1.0, 1.0);

		Draw_top_ui();

		if (sem_night_mode)
			Draw_screen_ready_to_draw(1, true, 2, 0.0, 0.0, 0.0);
		else
			Draw_screen_ready_to_draw(1, true, 2, 1.0, 1.0, 1.0);

		if (sem_selected_menu_mode >= 1 && sem_selected_menu_mode <= 8)
		{
			draw_y = 0.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				Draw_texture(Square_image, weak_red_tint, 0, 0.0, draw_y + sem_y_offset, 40.0, 25.0);
				Draw(sem_msg[38], 0.0, draw_y + sem_y_offset + 5.0, 0.6, 0.6, text_red, text_green, text_blue, text_alpha);
			}
		}

		if (sem_selected_menu_mode == 5 || sem_selected_menu_mode == 6)
		{
			Draw_texture(Square_image, white_or_black_tint, 0, 312.5, 0.0, 7.5, 15.0);
			Draw_texture(Square_image, white_or_black_tint, 0, 312.5, 215.0, 7.5, 10.0);
			Draw_texture(Square_image, blue_tint, 0, 312.5, 15.0 + (195 * (sem_y_offset / sem_y_max)), 7.5, 5.0);
		}

		if (sem_selected_menu_mode == 0)
		{
			draw_y = 0.0;

			for (int i = 0; i < 8; i++)
			{
				msg_num = i;
				if(i == 7)
					msg_num = 69;

				Draw_texture(Square_image, weak_aqua_tint, 0, 0.0, draw_y + sem_y_offset, 240.0, 20.0);
				Draw(sem_msg[msg_num], 0.0, draw_y + sem_y_offset - 2.5, 0.75, 0.75, text_red, text_green, text_blue, text_alpha);
				draw_y += 25.0;
			}
		}
		else if (sem_selected_menu_mode == 1)
		{
			//Check for updates
			draw_y = 25.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				Draw_texture(Square_image, weak_aqua_tint, 0, 10.0, draw_y + sem_y_offset, 240.0, 20.0);
				Draw(sem_msg[7], 10.0, draw_y + sem_y_offset - 2.5, 0.75, 0.75, text_red, text_green, text_blue, text_alpha);
			}
		}
		else if (sem_selected_menu_mode == 2)
		{
			//Languages
			draw_y = 25.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				if (sem_lang == "en")
				{
					red[0] = 1.0;
					blue[0] = 0.0;
					green[0] = 0.0;
					alpha[0] = 1.0;
				}
				else
				{
					red[1] = 1.0;
					blue[1] = 0.0;
					green[1] = 0.0;
					alpha[1] = 1.0;
				}

				for (int i = 0; i < 2; i++)
				{
					Draw_texture(Square_image, weak_aqua_tint, 0, 10.0, draw_y + sem_y_offset + (25.0 * i), 240.0, 20.0);
					Draw(sem_msg[8 + i], 10.0, draw_y + sem_y_offset + (25.0 * i), 0.75, 0.75, red[i], green[i], blue[i], alpha[i]);
				}
			}
		}
		else if (sem_selected_menu_mode == 3)
		{
			//Night mode
			draw_y = 25.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				if (sem_night_mode)
				{
					red[0] = 1.0;
					blue[0] = 0.0;
					green[0] = 0.0;
					alpha[0] = 1.0;
				}
				else
				{
					red[1] = 1.0;
					blue[1] = 0.0;
					green[1] = 0.0;
					alpha[1] = 1.0;
				}
				if (sem_flash_mode)
				{
					red[2] = 1.0;
					blue[2] = 0.0;
					green[2] = 0.0;
					alpha[2] = 1.0;
				}
				size_x_offset = 0.0;

				Draw(sem_msg[10], 0.0, draw_y + sem_y_offset, 0.5, 0.5, text_red, text_green, text_blue, text_alpha);
				for (int i = 0; i < 3; i++)
				{
					if (i == 2)
						size_x_offset = -50.0;

					Draw_texture(Square_image, weak_aqua_tint, 0, 10.0 + (i * 100.0), draw_y + sem_y_offset + 15.0, 90.0 + size_x_offset, 20.0);
					Draw(sem_msg[11 + i], 10.0 + (i * 100.0), draw_y + sem_y_offset + 12.5, 0.65, 0.65, red[i], green[i], blue[i], alpha[i]);
				}
			}

			//Screen brightness
			draw_y = 65.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				Draw(sem_msg[15] + std::to_string(sem_lcd_brightness), 0.0, draw_y + sem_y_offset, 0.5, 0.5, text_red, text_green, text_blue, text_alpha);
				Draw_texture(Square_image, weak_red_tint, 0, 10.0, draw_y + sem_y_offset + 22.5, 300.0, 5.0);
				Draw_texture(Square_image, white_or_black_tint, 0, (sem_lcd_brightness - 10) * 2, draw_y + sem_y_offset + 15.0, 4.0, 20.0);
			}

			//Time to turn off LCDs
			draw_y = 105.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				Draw(sem_msg[16] + std::to_string(sem_time_to_turn_off_lcd / 10) + sem_msg[17], 0.0, draw_y + sem_y_offset, 0.5, 0.5, text_red, text_green, text_blue, text_alpha);
				Draw_texture(Square_image, weak_red_tint, 0, 10.0, draw_y + sem_y_offset + 22.5, 300.0, 5.0);
				Draw_texture(Square_image, white_or_black_tint, 0, (sem_time_to_turn_off_lcd / 10), draw_y + sem_y_offset + 15.0, 4.0, 20.0);
			}

			//Screen brightness before turn off LCDs
			draw_y = 145.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				Draw(sem_msg[18] + std::to_string(sem_lcd_brightness_before_turn_off), 0.0, draw_y + sem_y_offset, 0.5, 0.5, text_red, text_green, text_blue, text_alpha);
				Draw_texture(Square_image, weak_red_tint, 0, 10.0, draw_y + sem_y_offset + 22.5, 300.0, 5.0);
				Draw_texture(Square_image, white_or_black_tint, 0, (sem_lcd_brightness_before_turn_off - 10) * 2, draw_y + sem_y_offset + 15.0, 4.0, 20.0);
			}
		}
		else if (sem_selected_menu_mode == 4)
		{
			//Scroll speed
			draw_y = 25.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				Draw(sem_msg[19] + std::to_string(sem_scroll_speed), 0.0, draw_y + sem_y_offset, 0.5, 0.5, text_red, text_green, text_blue, text_alpha);
				Draw_texture(Square_image, weak_red_tint, 0, 10.0, draw_y + sem_y_offset + 22.5, 300.0, 5.0);
				Draw_texture(Square_image, white_or_black_tint, 0, (sem_scroll_speed * 300), draw_y + sem_y_offset + 15.0, 4.0, 20.0);
			}
		}
		else if (sem_selected_menu_mode == 5)
		{
			//Font
			/*draw_y = 25.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				if (sem_use_default_font)
				{
					red[0] = 1.0;
					blue[0] = 0.0;
					green[0] = 0.0;
					alpha[0] = 1.0;
				}
				else if (sem_use_system_specific_font)
				{
					red[2] = 1.0;
					blue[2] = 0.0;
					green[2] = 0.0;
					alpha[2] = 1.0;
				}
				else if (sem_use_external_font)
				{
					red[1] = 1.0;
					blue[1] = 0.0;
					green[1] = 0.0;
					alpha[1] = 1.0;
				}

				for (int i = 0; i < 3; i++)
				{
					Draw_texture(Square_image, weak_aqua_tint, 0, 10.0 + (i * 100.0), draw_y + sem_y_offset + 15.0, 90.0, 20.0);
					Draw(sem_msg[20 + i], 10.0 + (i * 100.0), draw_y + sem_y_offset + 12.5, 0.4, 0.4, red[i], green[i], blue[i], alpha[i]);
				}
			}

			//Font, System specific
			if (sem_use_system_specific_font)
			{
				draw_x = 10.0;
				draw_y = 65.0;
				Sem_set_color(text_red, text_green, text_blue, text_alpha, red, green, blue, alpha, 4);
				if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
				{
					for (int i = 0; i < 4; i++)
					{

						if (sem_selected_lang_num == i)
						{
							red[i] = 1.0;
							green[i] = 0.0;
							blue[i] = 0.0;
							if (sem_load_system_font_request)
								alpha[i] = 0.3;
							else
								alpha[i] = 1.0;
						}
						else if (sem_load_system_font_request)
							alpha[i] = 0.3;

						Draw_texture(Square_image, weak_aqua_tint, 0, draw_x, draw_y + sem_y_offset + 15.0, 70.0, 20.0);
						Draw(sem_msg[23 + i], draw_x, draw_y + sem_y_offset + 12.5, 0.75, 0.75, red[i], green[i], blue[i], alpha[i]);

						draw_x += 75.0;
					}
				}
			}
			else if (sem_use_external_font)//Font, External
			{*/

			draw_x = 10.0;
			draw_y = 15.0;

			for (int i = 0; i < 4; i++)
			{
				if (sem_loaded_system_font[i] || i == sem_system_region)
				{
					red[i] = 1.0;
					green[i] = 0.0;
					blue[i] = 0.0;
					if (sem_load_system_font_request)
						alpha[i] = 0.3;
					else
						alpha[i] = 1.0;
				}
				else if (sem_load_system_font_request)
					alpha[i] = 0.3;

				Draw_texture(Square_image, weak_aqua_tint, 0, draw_x, draw_y + sem_y_offset + 15.0, 200.0, 20.0);
				Draw(sem_msg[23 + i], draw_x, draw_y + sem_y_offset + 12.5, 0.75, 0.75, red[i], green[i], blue[i], alpha[i]);
				draw_y += 20.0;
			}

				draw_x = 10.0;
				draw_y = 115.0;
				Sem_set_color(text_red, text_green, text_blue, text_alpha, red, green, blue, alpha, 1);
				if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
				{
					draw_tint[0] = weak_red_tint;
					draw_tint[1] = weak_yellow_tint;
					if (sem_unload_external_font_request || sem_load_external_font_request)
						alpha[0] = 0.3;

					for (int i = 0; i < 2; i++)
					{
						Draw_texture(Square_image, draw_tint[i], 0, draw_x, draw_y + sem_y_offset + 15.0, 100.0, 20.0);
						Draw(sem_msg[27 + i], draw_x, draw_y + sem_y_offset + 12.5, 0.65, 0.65, red[0], green[0], blue[0], alpha[0]);

						draw_x += 100.0;
					}
				}

				draw_x = 10.0;
				draw_y = 150.0;
				Sem_set_color(text_red, text_green, text_blue, text_alpha, red, green, blue, alpha, EXFONT_NUM_OF_FONT_NAME);
				for (int i = 0; i < EXFONT_NUM_OF_FONT_NAME; i++)
				{
					if (sem_loaded_external_font[i])
					{
						red[i] = 1.0;
						green[i] = 0.0;
						blue[i] = 0.0;

						if(sem_unload_external_font_request || sem_load_external_font_request)
							alpha[i] = 0.3;
						else
							alpha[i] = 1.0;
					}
					else if (sem_unload_external_font_request || sem_load_external_font_request)
						alpha[i] = 0.3;

					if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
					{
						Draw_texture(Square_image, weak_aqua_tint, 0, draw_x, draw_y + sem_y_offset, 200.0, 20.0);
						Draw(Exfont_query_font_name(i), draw_x, draw_y + sem_y_offset - 2.5, 0.45, 0.45, red[i], green[i], blue[i], alpha[i]);
					}
					draw_y += 20.0;
				}
//			}
		}
		else if (sem_selected_menu_mode == 6)
		{
			//Buffer size
		}
		else if (sem_selected_menu_mode == 7)
		{
			//Allow send app info
			draw_x = 10.0;
			draw_y = 25.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				if (sem_allow_send_app_info)
				{
					red[0] = 1.0;
					blue[0] = 0.0;
					green[0] = 0.0;
					alpha[0] = 1.0;
				}
				else
				{
					red[1] = 1.0;
					blue[1] = 0.0;
					green[1] = 0.0;
					alpha[1] = 1.0;
				}

				Draw(sem_msg[34], (draw_x - 10.0), draw_y + sem_y_offset, 0.5, 0.5, text_red, text_green, text_blue, text_alpha);
				for (int i = 0; i < 2; i++)
				{
					Draw_texture(Square_image, weak_aqua_tint, 0, draw_x, draw_y + sem_y_offset + 15.0, 90.0, 20.0);
					Draw(sem_msg[35 + i], draw_x, draw_y + sem_y_offset + 12.5, 0.75, 0.75, red[i], green[i], blue[i], alpha[i]);

					draw_x += 100.0;
				}
			}

			//Debug mode
			draw_x = 10.0;
			draw_y = 65.0;
			Sem_set_color(text_red, text_green, text_blue, text_alpha, red, green, blue, alpha, 2);
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				if (sem_debug_mode)
				{
					red[0] = 1.0;
					blue[0] = 0.0;
					green[0] = 0.0;
					alpha[0] = 1.0;
				}
				else
				{
					red[1] = 1.0;
					blue[1] = 0.0;
					green[1] = 0.0;
					alpha[1] = 1.0;
				}

				Draw(sem_msg[37], (draw_x - 10.0), draw_y + sem_y_offset, 0.5, 0.5, text_red, text_green, text_blue, text_alpha);
				for (int i = 0; i < 2; i++)
				{
					Draw_texture(Square_image, weak_aqua_tint, 0, draw_x, draw_y + sem_y_offset + 15.0, 90.0, 20.0);
					Draw(sem_msg[11 + i], draw_x, draw_y + sem_y_offset + 12.5, 0.75, 0.75, red[i], green[i], blue[i], alpha[i]);

					draw_x += 100.0;
				}
			}
		}
		else if (sem_selected_menu_mode == 8)
		{
			//Eco mode
			draw_x = 10.0;
			draw_y = 25.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				if (sem_eco_mode)
				{
					red[0] = 1.0;
					blue[0] = 0.0;
					green[0] = 0.0;
					alpha[0] = 1.0;
				}
				else
				{
					red[1] = 1.0;
					blue[1] = 0.0;
					green[1] = 0.0;
					alpha[1] = 1.0;
				}

				Draw(sem_msg[70], (draw_x - 10.0), draw_y + sem_y_offset, 0.5, 0.5, text_red, text_green, text_blue, text_alpha);
				for (int i = 0; i < 2; i++)
				{
					Draw_texture(Square_image, weak_aqua_tint, 0, draw_x, draw_y + sem_y_offset + 15.0, 90.0, 20.0);
					Draw(sem_msg[11 + i], draw_x, draw_y + sem_y_offset + 12.5, 0.75, 0.75, red[i], green[i], blue[i], alpha[i]);

					draw_x += 100.0;
				}
			}
		}

		if (sem_show_patch_note_request)
		{
			Draw_texture(Square_image, aqua_tint, 0, 15.0, 15.0, 290.0, 200.0);
			Draw_texture(Square_image, weak_aqua_tint, 0, 15.0, 200.0, 145.0, 15.0);
			Draw_texture(Square_image, weak_white_tint, 0, 160.0, 200.0, 145.0, 15.0);

			if(sem_check_update_progress == 0)
				Draw(sem_msg[39], 17.5, 15.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);
			else if(sem_check_update_progress == 2)
				Draw(sem_msg[40], 17.5, 15.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);
			else if (sem_check_update_progress == 1)
			{
				Draw(sem_msg[41 + new_version_available], 17.5, 15.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);
				Draw(sem_newest_ver_data[9], 17.5, 45.0, 0.45, 0.45, 0.0, 0.0, 0.0, 1.0);
			}
			Draw(sem_msg[46], 17.5, 200.0, 0.4, 0.4, 0.0, 0.0, 0.0, 1.0);
			Draw(sem_msg[45], 162.5, 200.0, 0.45, 0.45, 0.0, 0.0, 0.0, 1.0);
		}
		if (sem_select_ver_request)
		{
			Draw_texture(Square_image, aqua_tint, 0, 15.0, 15.0, 290.0, 200.0);
			Draw_texture(Square_image, weak_white_tint, 0, 15.0, 200.0, 145.0, 15.0);
			Draw_texture(Square_image, weak_aqua_tint, 0, 160.0, 200.0, 145.0, 15.0);

			draw_y = 15.0;
			for (int i = 0; i < 8; i++)
			{
				if(sem_available_ver[i] && sem_selected_edition_num == i)
					Draw(sem_msg[47 + i], 17.5, draw_y, 0.45, 0.45, 1.0, 0.0, 0.0, 1.0);
				else if (sem_available_ver[i])
					Draw(sem_msg[47 + i], 17.5, draw_y, 0.45, 0.45, 0.0, 0.0, 0.0, 1.0);
				else
					Draw(sem_msg[47 + i] + sem_msg[55], 17.5, draw_y, 0.45, 0.45, 1.0, 1.0, 1.0, 0.25);

				draw_y += 10.0;
			}

			for(int i = 0; i < 4; i++)
				Draw(sem_msg[56 + i], 17.5, 100.0 + (i * 10.0), 0.4, 0.4, 0.0, 0.0, 0.0, 1.0);

			if (sem_selected_edition_num == 0)
			{
				Draw(sem_msg[60], 17.5, 140.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);
				Draw("sdmc:/3ds/" + sem_update_dir + "_ver_" + sem_newest_ver_data[0] + "/" + sem_update_file + ".3dsx", 17.5, 150.0, 0.45, 0.45, 1.0, 0.0, 0.0, 1.0);
			}

			if(sem_update_progress == 0)
				Draw(std::to_string(sem_dled_size / 1024.0 / 1024.0).substr(0, 4) + "MB(" + std::to_string(sem_dled_size / 1024) + "KB)", 17.5, 180.0, 0.4, 0.4, 0.0, 0.0, 0.0, 1.0);
			else if(sem_update_progress == 1)
				Draw(std::to_string(sem_installed_size / 1024.0 / 1024.0).substr(0, 4) + "MB/" + std::to_string(sem_total_cia_size / 1024.0 / 1024.0).substr(0, 4) + "MB", 17.5, 180.0, 0.4, 0.4, 0.0, 0.0, 0.0, 1.0);

			if (sem_update_progress >= 0 && sem_update_progress <= 3)
				Draw(sem_msg[61 + sem_update_progress], 17.5, 160.0, 0.75, 0.75, 0.0, 0.0, 0.0, 1.0);

			if (sem_update_progress == 2)
			{
				Draw(sem_msg[65], 17.5, 180.0, 0.45, 0.45, 0.0, 0.0, 0.0, 1.0);
				Draw_texture(Square_image, yellow_tint, 0, 250.0, 180.0, 55.0, 20.0);
				Draw(sem_msg[73], 250.0, 180.0, 0.375, 0.375, 0.0, 0.0, 0.0, 1.0);
			}

			if (sem_available_ver[sem_selected_edition_num])
				Draw(sem_msg[66], 162.5, 200.0, 0.4, 0.4, 0.0, 0.0, 0.0, 1.0);
			else
				Draw(sem_msg[66], 162.5, 200.0, 0.4, 0.4, 1.0, 1.0, 1.0, 0.25);

			Draw(sem_msg[45], 17.5, 200.0, 0.45, 0.45, 0.0, 0.0, 0.0, 1.0);
		}
		Draw_bot_ui();
		Draw_touch_pos();

		Draw_apply_draw();
		sem_need_reflesh = false;
	}
	else
		gspWaitForVBlank();

	if (Err_query_error_show_flag())
	{
		if (key.p_a || (key.p_touch && key.touch_x >= 150 && key.touch_x <= 169 && key.touch_y >= 150 && key.touch_y <= 169))
			Err_set_error_show_flag(false);
		else if(key.p_x || (key.p_touch && key.touch_x >= 200 && key.touch_x <= 239 && key.touch_y >= 150 && key.touch_y <= 169))
			Err_save_error();
	}
	else
	{
		if (key.p_touch || key.h_touch)
		{
			sem_touch_x_move_left = 0;
			sem_touch_y_move_left = 0;

			if(sem_scroll_mode)
			{
				sem_touch_x_move_left = key.touch_x_move;
				sem_touch_y_move_left = key.touch_y_move;
			}
		}
		else
		{
			sem_scroll_mode = false;
			sem_scroll_bar_selected = false;
			sem_touch_x_move_left -= (sem_touch_x_move_left * 0.025);
			sem_touch_y_move_left -= (sem_touch_y_move_left * 0.025);
			for(int i = 0; i < 9; i++)
				sem_bar_selected[i] = false;
			for(int i = 0; i < EXFONT_NUM_OF_FONT_NAME; i++)
				sem_button_selected[i] = false;
			if (sem_touch_x_move_left < 0.5 && sem_touch_x_move_left > -0.5)
				sem_touch_x_move_left = 0;
			if (sem_touch_y_move_left < 0.5 && sem_touch_y_move_left > -0.5)
				sem_touch_y_move_left = 0;
		}

		if (key.p_start || (key.p_touch && key.touch_x >= 110 && key.touch_x <= 230 && key.touch_y >= 220 && key.touch_y <= 240))
			Sem_suspend();
		else if (sem_show_patch_note_request)
		{
			if (key.p_b || (key.p_touch && key.touch_x >= 160 && key.touch_x <= 304 && key.touch_y >= 200 && key.touch_y < 215))
				sem_show_patch_note_request = false;
			if (key.p_a || (key.p_touch && key.touch_x >= 15 && key.touch_x <= 159 && key.touch_y >= 200 && key.touch_y < 215))
			{
				sem_show_patch_note_request = false;
				sem_select_ver_request = true;
			}
		}
		else if (sem_select_ver_request && !sem_dl_file_request)
		{
			if (key.p_b || (key.p_touch && key.touch_x >= 15 && key.touch_x <= 159 && key.touch_y >= 200 && key.touch_y < 215))
			{
				sem_show_patch_note_request = true;
				sem_select_ver_request = false;
			}
			else if ((key.p_x || (key.p_touch && key.touch_x >= 160 && key.touch_x <= 304 && key.touch_y >= 200 && key.touch_y < 215)) && sem_available_ver[sem_selected_edition_num])
				sem_dl_file_request = true;
			else if(key.p_touch && key.touch_x >= 250 && key.touch_x <= 304 && key.touch_y >= 180 && key.touch_y <= 199 && sem_update_progress == 2)
				Menu_set_must_exit_flag(true);

			for (int i = 0; i < 8; i++)
			{
				if (key.p_touch && key.touch_x >= 17 && key.touch_x <= 250 && key.touch_y >= 15 + (i * 10) && key.touch_y <= 24 + (i * 10))
				{
					sem_selected_edition_num = i;
					break;
				}
			}
		}
		else if(!sem_dl_file_request)
		{
			if (key.p_touch && key.touch_x >= 0 && key.touch_x <= 40 && key.touch_y >= 0 + sem_y_offset && key.touch_y <= 24 + sem_y_offset
			&& sem_selected_menu_mode >= 1 && sem_selected_menu_mode <= 8)
			{
				sem_y_offset = 0.0;
				sem_y_max = 0.0;
				sem_selected_menu_mode = 0;
			}
			else
			{
				if(sem_selected_menu_mode == 5 || sem_selected_menu_mode == 6)//Scroll bar
				{
					if (key.h_c_down || key.h_c_up)
						sem_y_offset += (double)key.cpad_y * sem_scroll_speed * 0.0625;

					if (key.h_touch && sem_scroll_bar_selected)
						sem_y_offset *= (key.touch_y - 15.0) / 195.0;

					if (key.p_touch && key.touch_x >= 305 && key.touch_x <= 320 && key.touch_y >= 15)
						sem_scroll_bar_selected = true;

					sem_y_offset -= sem_touch_y_move_left * sem_scroll_speed;
				}

				if (sem_selected_menu_mode == 0)
				{
					for (int i = 0; i < 8; i++)
					{
						if (key.p_touch && key.touch_x >= 0 && key.touch_x <= 240 && key.touch_y >= 0 + (i * 25) && key.touch_y <= 19 + (i * 25))
						{
							sem_y_offset = 0.0;
							sem_selected_menu_mode = i + 1;
							if(i + 1 == 6)
								sem_y_max = -200.0;
							else if (i + 1 == 5)
								sem_y_max = -950.0;

							break;
						}
					}
				}
				else if (sem_selected_menu_mode == 1)//Check for updates
				{
					if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 239 && key.touch_y >= 25 && key.touch_y <= 44)
					{
						sem_check_update_request = true;
						sem_show_patch_note_request = true;
					}
				}
				else if (sem_selected_menu_mode == 2 && !sem_reload_msg_request)//Language
				{
					if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 249 && key.touch_y >= 25 && key.touch_y <= 44)
					{
						sem_lang = "en";
						sem_reload_msg_request = true;
					}
					else if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 249 && key.touch_y >= 50 && key.touch_y <= 69)
					{
						sem_lang = "jp";
						sem_reload_msg_request = true;
					}
				}
				else if (sem_selected_menu_mode == 3)//LCD
				{
					if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 99 && key.touch_y >= 40 && key.touch_y <= 59)
					{
						C2D_PlainImageTint(&texture_tint, C2D_Color32f(1.0, 1.0, 1.0, 0.75), true);
						sem_night_mode = true;
					}
					else if (key.p_touch && key.touch_x >= 110 && key.touch_x <= 199 && key.touch_y >= 40 && key.touch_y <= 59)
					{
						C2D_PlainImageTint(&texture_tint, C2D_Color32f(0.0, 0.0, 0.0, 1.0), true);
						sem_night_mode = false;
					}
					else if (key.p_touch && key.touch_x >= 210 && key.touch_x <= 249 && key.touch_y >= 40 && key.touch_y <= 59)
						sem_flash_mode = !sem_flash_mode;
					else if (key.h_touch && sem_bar_selected[0] && key.touch_x >= 10 && key.touch_x <= 309)
					{
						sem_lcd_brightness = (key.touch_x / 2) + 10;
						Menu_set_operation_flag(MENU_CHANGE_BRIGHTNESS_REQUEST, true);
					}
					else if (key.h_touch && sem_bar_selected[1] && key.touch_x >= 10 && key.touch_x <= 309)
						sem_time_to_turn_off_lcd = key.touch_x * 10;
					else if (key.h_touch && sem_bar_selected[2] && key.touch_x >= 10 && key.touch_x <= 309)
						sem_lcd_brightness_before_turn_off = (key.touch_x / 2) + 10;
					else if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 309 && key.touch_y >= 80 && key.touch_y <= 99)
						sem_bar_selected[0] = true;
					else if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 309 && key.touch_y >= 120 && key.touch_y <= 139)
						sem_bar_selected[1] = true;
					else if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 309 && key.touch_y >= 160 && key.touch_y <= 179)
						sem_bar_selected[2] = true;
				}
				else if (sem_selected_menu_mode == 4)//Scroll speed
				{
					if (key.h_touch && sem_bar_selected[3] && key.touch_x >= 10 && key.touch_x <= 309)
						sem_scroll_speed = (double)key.touch_x / 300;
					else if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 309 && key.touch_y >= 35 && key.touch_y <= 54)
						sem_bar_selected[3] = true;
				}
				else if (sem_selected_menu_mode == 5)//Font
				{
					sem_scroll_mode = true;
					for (int i = 0; i < EXFONT_NUM_OF_FONT_NAME; i++)
					{
						if(sem_button_selected[i])
							sem_scroll_mode = false;
					}


					if (key.p_touch && key.touch_y <= 219 && key.touch_x >= 10 && key.touch_x <= 209 && key.touch_y >= 30 + sem_y_offset && key.touch_y <= 109 + sem_y_offset && !sem_load_system_font_request)
					{
						sem_scroll_mode = false;
						for (int i = 0; i < 4; i++)
						{
							if (key.touch_y >= 30 + sem_y_offset + (i * 20) && key.touch_y <= 49 + sem_y_offset + (i * 20))
							{
								sem_button_selected[i] = true;
								if (sem_loaded_system_font[i])
								{
									sem_load_system_font[i] = false;
									sem_unload_system_font_request = true;
								}
								else
								{
									sem_load_system_font[i] = true;
									sem_load_system_font_request = true;
								}
								break;
							}
						}
					}
					else if (key.p_touch && key.touch_y <= 219 && key.touch_x >= 10 && key.touch_x <= 209 && key.touch_y >= 150 + sem_y_offset && key.touch_y <= 1149 + sem_y_offset && !Sem_query_operation_flag(SEM_LOAD_EXTERNAL_FONT_REQUEST) && !Sem_query_operation_flag(SEM_UNLOAD_EXTERNAL_FONT_REQUEST))
					{
						sem_scroll_mode = false;
						for (int i = 0; i < EXFONT_NUM_OF_FONT_NAME; i++)
						{
							if (key.touch_y >= 150 + sem_y_offset + (i * 20) && key.touch_y <= 169 + sem_y_offset + (i * 20))
							{
								sem_button_selected[i] = true;
								if (sem_loaded_external_font[i])
								{
									sem_load_external_font[i] = false;
									sem_unload_external_font_request = true;
								}
								else
								{
									sem_load_external_font[i] = true;
									sem_load_external_font_request = true;
								}
								break;
							}
						}
					}
					else if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 109 && key.touch_y >= 130 + sem_y_offset && key.touch_y <= 149 + sem_y_offset && !Sem_query_operation_flag(SEM_LOAD_EXTERNAL_FONT_REQUEST) && !Sem_query_operation_flag(SEM_UNLOAD_EXTERNAL_FONT_REQUEST))
					{
						sem_scroll_mode = false;
						for (int i = 0; i < EXFONT_NUM_OF_FONT_NAME; i++)
							sem_load_external_font[i] = true;
						
						sem_button_selected[0] = true;
						sem_load_external_font_request = true;
					}
					else if (key.p_touch && key.touch_x >= 110 && key.touch_x <= 209 && key.touch_y >= 130 + sem_y_offset && key.touch_y <= 149 + sem_y_offset && !Sem_query_operation_flag(SEM_LOAD_EXTERNAL_FONT_REQUEST) && !Sem_query_operation_flag(SEM_UNLOAD_EXTERNAL_FONT_REQUEST))
					{
						sem_scroll_mode = false;
						for (int i = 0; i < EXFONT_NUM_OF_FONT_NAME; i++)
							sem_load_external_font[i] = false;

						sem_button_selected[0] = true;
						sem_unload_external_font_request = true;
					}
				}
				else if (sem_selected_menu_mode == 6)//Memory
				{
					
				}
				else if (sem_selected_menu_mode == 7)//Advanced settings
				{
					if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 99 && key.touch_y >= 40 && key.touch_y <= 59)
						sem_allow_send_app_info = true;
					else if (key.p_touch && key.touch_x >= 100 && key.touch_x <= 199 && key.touch_y >= 40 && key.touch_y <= 59)
						sem_allow_send_app_info = false;
					else if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 99 && key.touch_y >= 80 && key.touch_y <= 99)
						sem_debug_mode = true;
					else if (key.p_touch && key.touch_x >= 100 && key.touch_x <= 199 && key.touch_y >= 80 && key.touch_y <= 99)
						sem_debug_mode = false;
				}
				else if (sem_selected_menu_mode == 8)//Battery
				{
					if (key.p_touch && key.touch_x >= 10 && key.touch_x <= 99 && key.touch_y >= 40 && key.touch_y <= 59)
						sem_eco_mode = true;
					else if (key.p_touch && key.touch_x >= 100 && key.touch_x <= 199 && key.touch_y >= 40 && key.touch_y <= 59)
						sem_eco_mode = false;
				}
			}

			if (sem_y_offset >= 0)
				sem_y_offset = 0.0;
			else if (sem_y_offset <= sem_y_max)
				sem_y_offset = sem_y_max;
		}
	}
}

Result_with_string Sem_load_msg(std::string lang)
{
	u8* fs_buffer = NULL;
	u32 read_size;
	std::string setting_data[128];
	Result_with_string result;
	fs_buffer = (u8*)malloc(0x2000);

	result = File_load_from_rom("sem_" + lang + ".txt", fs_buffer, 0x2000, &read_size, "romfs:/gfx/msg/");
	if (result.code != 0)
	{
		free(fs_buffer);
		return result;
	}

	result = Sem_parse_file((char*)fs_buffer, SEM_NUM_OF_MSG, setting_data);
	if (result.code != 0)
	{
		free(fs_buffer);
		return result;
	}

	for (int k = 0; k < SEM_NUM_OF_MSG; k++)
		Sem_set_msg(k, setting_data[k]);

	free(fs_buffer);
	return result;
}

void Sem_worker_thread(void* arg)
{
	Log_log_save(sem_worker_thread_string , "Thread started.", 1234567890, false);
	int log_num;
	Result_with_string result;

	while (sem_worker_thread_run)
	{
		if (sem_reload_msg_request)
		{
			log_num = Log_log_save(sem_worker_thread_string, "Sem_load_msg()...", 1234567890, false);
			result = Sem_load_msg(sem_lang);
			Log_log_add(log_num, result.string, result.code, false);

			log_num = Log_log_save(sem_worker_thread_string, "Bmark_load_msg()...", 1234567890, false);
			result = Bmark_load_msg(sem_lang);
			Log_log_add(log_num, result.string, result.code, false);
			
			log_num = Log_log_save(sem_worker_thread_string, "Bmr_load_msg()...", 1234567890, false);
			result = Bmr_load_msg(sem_lang);
			Log_log_add(log_num, result.string, result.code, false);
			
			sem_reload_msg_request = false;
		}
		else if (sem_change_wifi_state_request)
		{
			if (sem_wifi_enabled)
			{
				result.code = Wifi_disable();
				if (result.code == 0 || result.code == (int)0xC8A06C0D)
					sem_wifi_enabled = false;
			}
			else
			{
				result.code = Wifi_enable();
				if (result.code == 0 || result.code == (int)0xC8A06C0D)
					sem_wifi_enabled = true;
			}

			sem_change_wifi_state_request= false;
		}
		else if (sem_load_external_font_request)
		{
			for (int i = 0; i < EXFONT_NUM_OF_FONT_NAME; i++)
			{
				if (sem_load_external_font[i] && !sem_loaded_external_font[i])
				{
					log_num = Log_log_save(sem_worker_thread_string, "Exfont_load_exfont()...", 1234567890, FORCE_DEBUG);
					result = Exfont_load_exfont(i);
					Log_log_add(log_num, result.string, result.code, false);

					if(result.code == 0)
						sem_loaded_external_font[i] = true;
					else
						sem_loaded_external_font[i] = false;
				}
			}

			sem_load_external_font_request = false;
		}
		else if (sem_unload_external_font_request)
		{
			//cannnot unload basic latin font
			for (int i = 1; i < EXFONT_NUM_OF_FONT_NAME; i++)
			{
				if (!sem_load_external_font[i] && sem_loaded_external_font[i])
				{
					Exfont_unload_exfont(i);
					sem_loaded_external_font[i] = false;
				}
			}
			sem_unload_external_font_request = false;
		}
		else if (sem_load_system_font_request)
		{
			for(int i = 0; i < 4; i++)
			{
				if(!sem_loaded_system_font[i] && sem_load_system_font[i] && i != sem_system_region)
				{
					log_num = Log_log_save(sem_worker_thread_string, "Draw_load_system_font()...", 1234567890, FORCE_DEBUG);
					result = Draw_load_system_font(i);
					Log_log_add(log_num, result.string, result.code, false);
					if(result.code == 0)
						sem_loaded_system_font[i] = true;
					else
						sem_loaded_system_font[i] = false;
				}
			}
			sem_load_system_font_request = false;
		}
		else if (sem_unload_system_font_request)
		{
			for(int i = 0; i < 4; i++)
			{
				if(sem_loaded_system_font[i] && !sem_load_system_font[i] && i != sem_system_region)
				{
					Draw_free_system_font(i);
					sem_loaded_system_font[i] = false;
				}
			}
			sem_unload_system_font_request = false;
		}
		else
			usleep(ACTIVE_THREAD_SLEEP_TIME);
	}

	Log_log_save(sem_worker_thread_string , "Thread exit.", 1234567890, false);
	threadExit(0);
}

void Sem_check_update_thread(void* arg)
{
	Log_log_save(sem_check_update_string, "Thread started.", 1234567890, false);

	u8* buffer = NULL;
	u32 status_code = 0;
	u32 write_size = 0;
	u32 read_size = 0;
	u64 offset = 0;
	int log_num = 0;
	int newest_ver = 0;
	size_t parse_start_pos = std::string::npos;
	size_t parse_end_pos = std::string::npos;
	std::string dir_path = "";
	std::string file_name = "";
	std::string url = "";
	std::string last_url = "";
	std::string parse_cache = "";
	std::string parse_start[18] = {"<newest>", "<3dsx_available>", "<cia_32mb_ram_available>",
	"<cia_64mb_ram_available>", "<cia_72mb_ram_available>", "<cia_80mb_ram_available>",
	"<cia_96mb_ram_available>", "<cia_124mb_ram_available>", "<cia_178mb_ram_available>",
	"<patch_note>", "<3dsx_url>", "<32mbcia_url>", "<64mbcia_url>", "<72mbcia_url>", "<80mbcia_url>",
	"<96mbcia_url>", "<124mbcia_url>", "<178mbcia_url>", };
	std::string parse_end[18] = { "</newest>", "</3dsx_available>", "</cia_32mb_ram_available>",
	"</cia_64mb_ram_available>", "</cia_72mb_ram_available>", "</cia_80mb_ram_available>",
	"</cia_96mb_ram_available>", "</cia_124mb_ram_available>", "</cia_178mb_ram_available>",
	"</patch_note>", "</3dsx_url>", "</32mbcia_url>", "</64mbcia_url>", "</72mbcia_url>", "</80mbcia_url>",
	"</96mbcia_url>", "</124mbcia_url>", "</178mbcia_url>", };
	std::string editions[8] = { ".3dsx", "_32mb.cia", "_64mb.cia", "_72mb.cia",
	"_80mb.cia", "_96mb.cia", "_124mb.cia", "_178mb.cia", };
	Handle am_handle = 0;
	Result_with_string result;

	while (sem_check_update_thread_run)
	{
		if (sem_check_update_request || sem_dl_file_request)
		{
			if (sem_check_update_request)
			{
				url = sem_update_url;
				sem_check_update_progress = 0;
				new_version_available = false;
				for(int i = 0; i < 8; i++)
					sem_available_ver[i] = false;
				for (int i = 0; i < 10; i++)
					sem_newest_ver_data[i] = "";
			}
			else if (sem_dl_file_request)
			{
				url = sem_newest_ver_data[sem_selected_edition_num + 10];
				sem_update_progress = 0;
			}

			newest_ver = -1;
			sem_dled_size = 0;
			offset = 0;
			sem_installed_size = 0;
			sem_total_cia_size = 0;
			buffer = (u8*)malloc(0x20000);
			if (buffer == NULL)
			{
				Err_set_error_message(Err_query_template_summary(OUT_OF_MEMORY), Err_query_template_detail(OUT_OF_MEMORY), sem_check_update_string, OUT_OF_MEMORY);
				Err_set_error_show_flag(true);
				Log_log_save(sem_check_update_string, Err_query_template_summary(OUT_OF_MEMORY), OUT_OF_MEMORY, false);
			}
			else
			{
				log_num = Log_log_save(sem_check_update_string, "Httpc_dl_data()...", 1234567890, false);
				if(sem_dl_file_request)
				{
					if(sem_selected_edition_num == 0)
						dir_path = "/3ds/" + sem_update_dir + "_ver_" + sem_newest_ver_data[0] + "/";
					else
						dir_path = "/" + sem_update_dir + "/ver_" + sem_newest_ver_data[0] + "/";

					file_name = sem_update_file + editions[sem_selected_edition_num];
					File_delete_file(file_name, dir_path);//delete old file if exist
				}

				if(sem_dl_file_request)
					result = Httpc_dl_data(url, buffer, 0x20000, &sem_dled_size, &status_code, true, &last_url, false, 100, SEM_HTTP_PORT0, dir_path, file_name);
				else
					result = Httpc_dl_data(url, buffer, 0x20000, &sem_dled_size, &status_code, true, &last_url, false, 100, SEM_HTTP_PORT0);

				Log_log_add(log_num, result.string + std::to_string(sem_dled_size / 1024) + "KB (" + std::to_string(sem_dled_size) + "B)", result.code, false);

				if (result.code != 0)
				{
					Err_set_error_message(result.string, result.error_description, sem_check_update_string, result.code);
					Err_set_error_show_flag(true);
					if (sem_check_update_request)
						sem_check_update_progress = 2;
					else if (sem_dl_file_request)
						sem_update_progress = 3;
				}
				else
				{
					if (sem_check_update_request)
					{
						parse_cache = (char*)buffer;

						for (int i = 0; i < 18; i++)
						{
							parse_start_pos = std::string::npos;
							parse_end_pos = std::string::npos;
							parse_start_pos = parse_cache.find(parse_start[i]);
							parse_end_pos = parse_cache.find(parse_end[i]);

							parse_start_pos += parse_start[i].length();
							parse_end_pos -= parse_start_pos;
							if (parse_start_pos != std::string::npos && parse_end_pos != std::string::npos)
								sem_newest_ver_data[i] = parse_cache.substr(parse_start_pos, parse_end_pos);
							else
							{
								sem_check_update_progress = 2;
								break;
							}

							if (i == 0)
								newest_ver = stoi(sem_newest_ver_data[i]);
							else if (i > 0 && i < 9)
								sem_available_ver[i - 1] = stoi(sem_newest_ver_data[i]);
						}

						if (sem_current_app_ver < newest_ver)
							new_version_available = true;
						else
							new_version_available = false;

						sem_check_update_progress = 1;
					}
					else if (sem_dl_file_request)
					{
						sem_update_progress = 1;
						if (sem_selected_edition_num == 0)
							sem_update_progress = 2;

						if (sem_selected_edition_num > 0 && sem_selected_edition_num < 8)
						{
							sem_total_cia_size = sem_dled_size;
							log_num = Log_log_save(sem_check_update_string, "AM_StartCiaInstall()...", 1234567890, false);
							result.code = AM_StartCiaInstall(MEDIATYPE_SD, &am_handle);
							Log_log_add(log_num, "", result.code, false);

							while (true)
							{
								log_num = Log_log_save(sem_check_update_string, "File_load_from_file_with_range()...", 1234567890, false);
								result = File_load_from_file_with_range(file_name, (u8*)buffer, 0x20000, offset, &read_size, dir_path);
								Log_log_add(log_num, result.string, result.code, false);
								if(result.code != 0 || read_size <= 0)
									break;

								log_num = Log_log_save(sem_check_update_string, "FSFILE_Write()...", 1234567890, false);
								result.code = FSFILE_Write(am_handle, &write_size, offset, (u8*)buffer, read_size, FS_WRITE_FLUSH);
								Log_log_add(log_num, "", result.code, false);
								if(result.code != 0)
									break;

								offset += write_size;
								sem_installed_size += write_size;
								sem_need_reflesh = true;
							}

							log_num = Log_log_save(sem_check_update_string, "AM_FinishCiaInstall()...", 1234567890, false);
							result.code = AM_FinishCiaInstall(am_handle);
							Log_log_add(log_num, "", result.code, false);
							if (result.code == 0)
								sem_update_progress = 2;
							else
								sem_update_progress = 3;
						}
					}
				}
			}

			free(buffer);
			buffer = NULL;
			if(sem_check_update_request)
				sem_check_update_request = false;
			else if(sem_dl_file_request)
				sem_dl_file_request = false;
		}
		else
			usleep(ACTIVE_THREAD_SLEEP_TIME);

		while (sem_thread_suspend)
			usleep(INACTIVE_THREAD_SLEEP_TIME);
	}
	Log_log_save(sem_check_update_string, "Thread exit.", 1234567890, false);
	threadExit(0);
}
