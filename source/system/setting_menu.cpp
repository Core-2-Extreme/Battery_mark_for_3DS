﻿#include "definitions.hpp"
#include "system/types.hpp"

#include "system/menu.hpp"
#include "system/variables.hpp"

#include "system/draw/draw.hpp"
#include "system/draw/external_font.hpp"

#include "system/util/change_setting.hpp"
#include "system/util/converter.hpp"
#include "system/util/cpu_usage.hpp"
#include "system/util/curl.hpp"
#include "system/util/encoder.hpp"
#include "system/util/error.hpp"
#include "system/util/file.hpp"
#include "system/util/hid.hpp"
#include "system/util/httpc.hpp"
#include "system/util/log.hpp"
#include "system/util/util.hpp"

//Include myself.
#include "system/setting_menu.hpp"

#ifdef DEF_ENABLE_BMARK
#include "battery_mark.hpp"
#endif

#ifdef DEF_ENABLE_BMR
#include "battery_mark_ranking.hpp"
#endif

#ifdef DEF_ENABLE_SUB_APP2
#include "sub_app2.hpp"
#endif

#ifdef DEF_ENABLE_SUB_APP3
#include "sub_app3.hpp"
#endif

#ifdef DEF_ENABLE_SUB_APP4
#include "sub_app4.hpp"
#endif

#ifdef DEF_ENABLE_SUB_APP5
#include "sub_app5.hpp"
#endif

#ifdef DEF_ENABLE_SUB_APP6
#include "sub_app6.hpp"
#endif

#ifdef DEF_ENABLE_SUB_APP7
#include "sub_app7.hpp"
#endif

bool sem_main_run = false;
bool sem_already_init = false;
bool sem_thread_run = false;
bool sem_thread_suspend = false;
bool sem_reload_msg_request = false;
bool sem_change_brightness_request = false;
bool sem_scroll_mode = false;
bool sem_dump_log_request = false;
u8 sem_fake_model_num = 255;
int sem_selected_menu_mode = DEF_SEM_MENU_TOP;
double sem_y_offset = 0.0;
double sem_y_max = 0.0;
double sem_touch_x_move_left = 0.0;
double sem_touch_y_move_left = 0.0;
std::string sem_msg[DEF_SEM_NUM_OF_MSG];
std::string sem_newest_ver_data[6];//0 newest version number, 1 3dsx available, 2 cia available, 3 3dsx dl url, 4 cia dl url, 5 patch note
Image_data sem_back_button, sem_scroll_bar, sem_menu_button[9], sem_english_button, sem_japanese_button,
sem_hungarian_button, sem_chinese_button, sem_italian_button, sem_spanish_button, sem_romanian_button, sem_polish_button, sem_ryukyuan_button,
sem_night_mode_on_button, sem_night_mode_off_button, sem_flash_mode_button, sem_screen_brightness_slider, sem_screen_brightness_bar,
sem_screen_off_time_slider, sem_screen_off_time_bar, sem_sleep_time_slider, sem_sleep_time_bar, sem_800px_mode_button, sem_3d_mode_button,
sem_400px_mode_button, sem_auto_mode_button, sem_scroll_speed_slider, sem_scroll_speed_bar, sem_load_all_ex_font_button, sem_unload_all_ex_font_button,
sem_ex_font_button[DEF_EXFONT_NUM_OF_FONT_NAME], sem_wifi_on_button, sem_wifi_off_button, sem_allow_send_info_button, sem_deny_send_info_button,
sem_debug_mode_on_button, sem_debug_mode_off_button, sem_eco_mode_on_button, sem_eco_mode_off_button,sem_record_both_lcd_button,
sem_record_top_lcd_button, sem_record_bottom_lcd_button, sem_use_fake_model_button, sem_dump_log_button;

#if DEF_ENABLE_CPU_MONITOR_API
	bool sem_is_cpu_usage_monitor_running = false;
#endif

#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)

bool sem_check_update_request = false;
bool sem_new_version_available = false;
bool sem_show_patch_note_request = false;
bool sem_select_ver_request = false;
bool sem_dl_file_request = false;
int sem_update_progress = -1;
int sem_check_update_progress = 0;
int sem_selected_edition_num = DEF_SEM_EDTION_NONE;
int sem_installed_size = 0;
int sem_total_cia_size = 0;
Thread sem_update_thread;
Image_data sem_check_update_button, sem_select_edtion_button, sem_close_updater_button, sem_3dsx_button,
sem_cia_button, sem_dl_install_button, sem_back_to_patch_note_button, sem_close_app_button;
#if DEF_ENABLE_CURL_API
int sem_dled_size = 0;
#else
u32 sem_dled_size = 0;
#endif

#endif


#if DEF_ENABLE_CPU_MONITOR_API

Image_data sem_monitor_cpu_usage_on_button, sem_monitor_cpu_usage_off_button;

#endif

#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)

bool sem_record_request = false;
bool sem_encode_request = false;
bool sem_wait_request = false;
bool sem_stop_record_request = false;
int sem_selected_recording_mode = 0;
u8* sem_yuv420p = NULL;
int sem_rec_width = 400;
int sem_rec_height = 480;
Thread sem_record_thread, sem_encode_thread;

void Sem_encode_thread(void* arg);
void Sem_record_thread(void* arg);

#endif

void Sem_worker_callback(void);

#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)

void Sem_update_thread(void* arg);

#endif

bool Sem_query_init_flag(void)
{
	return sem_already_init;
}

bool Sem_query_running_flag(void)
{
	return sem_main_run;
}

void Sem_resume(void)
{
	sem_thread_suspend = false;
	sem_main_run = true;
	var_need_reflesh = true;
	Menu_suspend();
}

void Sem_suspend(void)
{
	Menu_resume();
	sem_thread_suspend = true;
	sem_main_run = false;
}

Result_with_string Sem_load_msg(std::string lang)
{
	return Util_load_msg("sem_" + lang + ".txt", sem_msg, DEF_SEM_NUM_OF_MSG);
}

void Sem_init(void)
{
	Util_log_save(DEF_SEM_INIT_STR, "Initializing...");
	bool wifi_state = true;
	u8* read_cache = NULL;
	u32 read_size = 0;
	std::string data[13];
	Result_with_string result;

	if(var_fake_model)
		sem_fake_model_num = var_model;

	result = Util_file_load_from_file("settings.txt", DEF_MAIN_DIR, &read_cache, 0x1000, &read_size);
	Util_log_save(DEF_SEM_INIT_STR , "Util_file_load_from_file()..." + result.string + result.error_description, result.code);
	if (result.code == 0)
	{
		result = Util_parse_file((char*)read_cache, 13, data);
		Util_log_save(DEF_SEM_INIT_STR , "Util_parse_file()..." + result.string + result.error_description, result.code);

		//If this fails, the settings file may come from older version.
		//To keep backward compatibility, try to parse with less elements.
		if(result.code != 0)
		{
			result = Util_parse_file((char*)read_cache, 12, data);
			Util_log_save(DEF_SEM_INIT_STR , "Util_parse_file()..." + result.string + result.error_description, result.code);
			data[12] = "175";//Time to turn off LCDs default value.
		}
		if(result.code != 0)
		{
			result = Util_parse_file((char*)read_cache, 11, data);
			Util_log_save(DEF_SEM_INIT_STR , "Util_parse_file()..." + result.string + result.error_description, result.code);
			data[11] = "0";//Screen mode default value.
			data[12] = "175";//Time to turn off LCDs default value.
		}

		if(result.code == 0)
		{
			var_lang = data[0];
			var_lcd_brightness = atoi(data[1].c_str());
			var_time_to_turn_off_lcd = atoi(data[2].c_str());
			var_scroll_speed = strtod(data[3].c_str(), NULL);
			var_allow_send_app_info = (data[4] == "1");
			var_num_of_app_start = atoi(data[5].c_str());
			var_night_mode = (data[6] == "1");
			var_eco_mode = (data[7] == "1");
			wifi_state = (data[8] == "1");
			//9 and 10 is no longer used.
			var_screen_mode = atoi(data[11].c_str());
			var_time_to_enter_sleep = atoi(data[12].c_str());

			if(var_lang != "jp" && var_lang != "en" && var_lang != "hu" && var_lang != "zh-cn" && var_lang != "it"
			&& var_lang != "es" && var_lang != "ro" && var_lang != "pl" && var_lang != "ryu")
				var_lang = "en";

			if(var_lcd_brightness < 0 || var_lcd_brightness > 180)
				var_lcd_brightness = 100;

			if(var_time_to_turn_off_lcd < 0)
				var_time_to_turn_off_lcd = -1;
			else if(var_time_to_turn_off_lcd < 20 || var_time_to_turn_off_lcd > 600)
				var_time_to_turn_off_lcd = 150;

			if(var_scroll_speed < 0.05 || var_scroll_speed > 2)
				var_scroll_speed = 0.5;

			if(var_num_of_app_start < 0)
				var_num_of_app_start = 0;

			if(var_screen_mode > DEF_SEM_SCREEN_3D)
				var_screen_mode = DEF_SEM_SCREEN_AUTO;

			if(var_time_to_enter_sleep < 0)
				var_time_to_enter_sleep = -1;
			else if(var_time_to_enter_sleep < 20 || var_time_to_enter_sleep > 600)
				var_time_to_enter_sleep = 175;

			if(var_time_to_turn_off_lcd > 0 && var_time_to_enter_sleep > 0 && (var_time_to_turn_off_lcd > var_time_to_enter_sleep))
				var_time_to_enter_sleep = var_time_to_turn_off_lcd;

			var_top_lcd_brightness = var_lcd_brightness;
			var_bottom_lcd_brightness = var_lcd_brightness;
		}
		Util_safe_linear_free(read_cache);
		read_cache = NULL;
	}

	if(var_model == CFG_MODEL_2DS && var_screen_mode == DEF_SEM_SCREEN_800PX)//OLD 2DS doesn't support high resolution mode.
		var_screen_mode = DEF_SEM_SCREEN_AUTO;

	if((var_model == CFG_MODEL_2DS || var_model == CFG_MODEL_N2DSXL) && var_screen_mode == DEF_SEM_SCREEN_3D)//2DSs don't support 3d mode.
		var_screen_mode = DEF_SEM_SCREEN_AUTO;

	sem_thread_run = true;
#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)
	sem_update_thread = threadCreate(Sem_update_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 0, false);
#endif

#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)
	sem_record_thread = threadCreate(Sem_record_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_HIGH, 0, false);
	if(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS)
		sem_encode_thread = threadCreate(Sem_encode_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_HIGH, 2, false);
	else
		sem_encode_thread = threadCreate(Sem_encode_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_HIGH, 1, false);
#endif

	sem_reload_msg_request = true;

	result = Util_cset_set_wifi_state(wifi_state);
	if(result.code == 0 || result.code == 0xC8A06C0D)
		var_wifi_enabled = wifi_state;

	//global
	Util_add_watch(&sem_y_offset);
	Util_add_watch(&sem_y_max);
	Util_add_watch(&sem_selected_menu_mode);
	Util_add_watch(&sem_scroll_mode);
	Util_add_watch(&sem_scroll_bar.selected);

	Util_add_watch(&sem_back_button.selected);
	for(int i = 0; i < 9; i++)
		Util_add_watch(&sem_menu_button[i].selected);


#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)
	//Updater
	Util_add_watch(&sem_show_patch_note_request);
	Util_add_watch(&sem_select_ver_request);
	Util_add_watch(&sem_dl_file_request);
	Util_add_watch(&sem_check_update_request);
	Util_add_watch(&sem_selected_edition_num);
	Util_add_watch(&sem_installed_size);
	Util_add_watch((int*)&sem_dled_size);

	Util_add_watch(&sem_close_updater_button.selected);
	Util_add_watch(&sem_select_edtion_button.selected);
	Util_add_watch(&sem_3dsx_button.selected);
	Util_add_watch(&sem_cia_button.selected);
	Util_add_watch(&sem_back_to_patch_note_button.selected);
	Util_add_watch(&sem_dl_install_button.selected);
	Util_add_watch(&sem_close_app_button.selected);
	Util_add_watch(&sem_check_update_button.selected);
#endif

	//Languages
	Util_add_watch(&sem_reload_msg_request);

	Util_add_watch(&sem_english_button.selected);
	Util_add_watch(&sem_japanese_button.selected);
	Util_add_watch(&sem_hungarian_button.selected);
	Util_add_watch(&sem_chinese_button.selected);
	Util_add_watch(&sem_italian_button.selected);
	Util_add_watch(&sem_spanish_button.selected);
	Util_add_watch(&sem_romanian_button.selected);
	Util_add_watch(&sem_polish_button.selected);
	Util_add_watch(&sem_ryukyuan_button.selected);

	//LCD
	Util_add_watch(&var_night_mode);
	Util_add_watch(&var_flash_mode);
	Util_add_watch(&var_lcd_brightness);
	Util_add_watch(&var_top_lcd_brightness);
	Util_add_watch(&var_bottom_lcd_brightness);
	Util_add_watch(&sem_change_brightness_request);
	Util_add_watch(&var_time_to_turn_off_lcd);
	Util_add_watch(&var_time_to_enter_sleep);
	Util_add_watch((bool*)&var_screen_mode);

	Util_add_watch(&sem_night_mode_on_button.selected);
	Util_add_watch(&sem_night_mode_off_button.selected);
	Util_add_watch(&sem_flash_mode_button.selected);
	Util_add_watch(&sem_screen_brightness_bar.selected);
	Util_add_watch(&sem_screen_off_time_bar.selected);
	Util_add_watch(&sem_sleep_time_bar.selected);
	Util_add_watch(&sem_800px_mode_button.selected);
	Util_add_watch(&sem_3d_mode_button.selected);
	Util_add_watch(&sem_400px_mode_button.selected);
	Util_add_watch(&sem_auto_mode_button.selected);

	//Scroll speed
	Util_add_watch(&var_scroll_speed);

	Util_add_watch(&sem_scroll_speed_bar.selected);

	//Font
	Util_add_watch(&sem_load_all_ex_font_button.selected);
	Util_add_watch(&sem_unload_all_ex_font_button.selected);
	for(int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
		Util_add_watch(&sem_ex_font_button[i].selected);
	
	//Wireless
	Util_add_watch(&var_wifi_enabled);

	Util_add_watch(&sem_wifi_on_button.selected);
	Util_add_watch(&sem_wifi_off_button.selected);

	//Advanced settings
	Util_add_watch(&var_allow_send_app_info);
	Util_add_watch(&var_debug_mode);

	Util_add_watch(&sem_allow_send_info_button.selected);
	Util_add_watch(&sem_deny_send_info_button.selected);
	Util_add_watch(&sem_debug_mode_on_button.selected);
	Util_add_watch(&sem_debug_mode_off_button.selected);
	Util_add_watch(&sem_use_fake_model_button.selected);
	Util_add_watch(&sem_dump_log_button.selected);

#if DEF_ENABLE_CPU_MONITOR_API
	Util_add_watch(&sem_monitor_cpu_usage_on_button.selected);
	Util_add_watch(&sem_monitor_cpu_usage_off_button.selected);
#endif

	//Battery
	Util_add_watch(&var_eco_mode);
	Util_add_watch(&sem_eco_mode_on_button.selected);
	Util_add_watch(&sem_eco_mode_off_button.selected);

#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)
	//Screen recording
	Util_add_watch(&sem_record_request);
	Util_add_watch(&sem_stop_record_request);
#endif

	Util_add_watch(&sem_record_both_lcd_button.selected);
	Util_add_watch(&sem_record_top_lcd_button.selected);
	Util_add_watch(&sem_record_bottom_lcd_button.selected);

	result.code = Menu_add_worker_thread_callback(Sem_worker_callback);
	Util_log_save(DEF_SEM_INIT_STR, "Menu_add_worker_thread_callback()...", result.code);

	Sem_resume();
	sem_already_init = true;
	Util_log_save(DEF_SEM_INIT_STR, "Initialized.");
}

void Sem_draw_init(void)
{
	Util_add_watch(&Draw_get_bot_ui_button()->selected);
	sem_back_button.c2d = var_square_image[0];
	sem_scroll_bar.c2d = var_square_image[0];
	sem_english_button.c2d = var_square_image[0];
	sem_japanese_button.c2d = var_square_image[0];
	sem_hungarian_button.c2d = var_square_image[0];
	sem_chinese_button.c2d = var_square_image[0];
	sem_italian_button.c2d = var_square_image[0];
	sem_spanish_button.c2d = var_square_image[0];
	sem_romanian_button.c2d = var_square_image[0];
	sem_polish_button.c2d = var_square_image[0];
	sem_ryukyuan_button.c2d = var_square_image[0];
	sem_night_mode_on_button.c2d = var_square_image[0];
	sem_night_mode_off_button.c2d = var_square_image[0];
	sem_flash_mode_button.c2d = var_square_image[0];
	sem_screen_brightness_slider.c2d = var_square_image[0];
	sem_screen_brightness_bar.c2d = var_square_image[0];
	sem_screen_off_time_slider.c2d = var_square_image[0];
	sem_screen_off_time_bar.c2d = var_square_image[0];
	sem_sleep_time_slider.c2d = var_square_image[0];
	sem_sleep_time_bar.c2d = var_square_image[0];
	sem_800px_mode_button.c2d = var_square_image[0];
	sem_3d_mode_button.c2d = var_square_image[0];
	sem_400px_mode_button.c2d = var_square_image[0];
	sem_auto_mode_button.c2d = var_square_image[0];
	sem_scroll_speed_slider.c2d = var_square_image[0];
	sem_scroll_speed_bar.c2d = var_square_image[0];
	sem_load_all_ex_font_button.c2d = var_square_image[0];
	sem_unload_all_ex_font_button.c2d = var_square_image[0];
	sem_wifi_on_button.c2d = var_square_image[0];
	sem_wifi_off_button.c2d = var_square_image[0];
	sem_allow_send_info_button.c2d = var_square_image[0];
	sem_deny_send_info_button.c2d = var_square_image[0];
	sem_debug_mode_on_button.c2d = var_square_image[0];
	sem_debug_mode_off_button.c2d = var_square_image[0];
	sem_eco_mode_on_button.c2d = var_square_image[0];
	sem_eco_mode_off_button.c2d = var_square_image[0];
	sem_record_both_lcd_button.c2d = var_square_image[0];
	sem_record_top_lcd_button.c2d = var_square_image[0];
	sem_record_bottom_lcd_button.c2d = var_square_image[0];
	sem_use_fake_model_button.c2d = var_square_image[0];
	sem_dump_log_button.c2d = var_square_image[0];

#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)
	sem_check_update_button.c2d = var_square_image[0];
	sem_select_edtion_button.c2d = var_square_image[0];
	sem_close_updater_button.c2d = var_square_image[0];
	sem_3dsx_button.c2d = var_square_image[0];
	sem_cia_button.c2d = var_square_image[0];
	sem_dl_install_button.c2d = var_square_image[0];
	sem_back_to_patch_note_button.c2d = var_square_image[0];
	sem_close_app_button.c2d = var_square_image[0];
#endif

#if DEF_ENABLE_CPU_MONITOR_API
	sem_monitor_cpu_usage_on_button.c2d = var_square_image[0];
	sem_monitor_cpu_usage_off_button.c2d = var_square_image[0];
#endif

	for(int i = 0; i < 9; i++)
		sem_menu_button[i].c2d = var_square_image[0];
	for(int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
		sem_ex_font_button[i].c2d = var_square_image[0];
}

void Sem_exit(void)
{
	Util_log_save(DEF_SEM_EXIT_STR, "Exiting...");
	int log_num;
	var_num_of_app_start++;
	std::string data = "<0>" + var_lang + "</0><1>" + std::to_string(var_lcd_brightness) + "</1><2>" + std::to_string(var_time_to_turn_off_lcd)
	+ "</2><3>" + std::to_string(var_scroll_speed) + "</3><4>" + std::to_string(var_allow_send_app_info) + "</4><5>" + std::to_string(var_num_of_app_start)
	+ "</5><6>" + std::to_string(var_night_mode) + "</6><7>" + std::to_string(var_eco_mode) + "</7><8>" + std::to_string(var_wifi_enabled) + "</8>"
	+ "<9>0</9><10>0</10><11>" + std::to_string(var_screen_mode) + "</11><12>" + std::to_string(var_time_to_enter_sleep) + "</12>";
	//9 and 10 is no longer used.
	Result_with_string result;

#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)
	sem_stop_record_request = true;
#endif

	sem_already_init = false;
	sem_thread_suspend = false;
	sem_thread_run = false;
	Menu_remove_worker_thread_callback(Sem_worker_callback);

	log_num = Util_log_save(DEF_SEM_EXIT_STR, "Util_file_save_to_file()...");
	result = Util_file_save_to_file("settings.txt", DEF_MAIN_DIR, (u8*)data.c_str(), data.length(), true);
	Util_log_add(log_num, result.string, result.code);

	log_num = Util_log_save(DEF_SEM_EXIT_STR, "Util_file_save_to_file()...");
	result = Util_file_save_to_file("fake_model.txt", DEF_MAIN_DIR, &sem_fake_model_num, 1, true);
	Util_log_add(log_num, result.string, result.code);

#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)
	Util_log_save(DEF_SEM_EXIT_STR, "threadJoin()...", threadJoin(sem_update_thread, DEF_THREAD_WAIT_TIME));
	threadFree(sem_update_thread);
#endif

#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)
	Util_log_save(DEF_SEM_EXIT_STR, "threadJoin()...", threadJoin(sem_encode_thread, DEF_THREAD_WAIT_TIME));
	Util_log_save(DEF_SEM_EXIT_STR, "threadJoin()...", threadJoin(sem_record_thread, DEF_THREAD_WAIT_TIME));

	threadFree(sem_encode_thread);
	threadFree(sem_record_thread);
#endif

	//global
	Util_remove_watch(&Draw_get_bot_ui_button()->selected);
	Util_remove_watch(&sem_y_offset);
	Util_remove_watch(&sem_y_max);
	Util_remove_watch(&sem_selected_menu_mode);
	Util_remove_watch(&sem_scroll_mode);
	Util_remove_watch(&sem_scroll_bar.selected);

	Util_remove_watch(&sem_back_button.selected);
	for(int i = 0; i < 9; i++)
		Util_remove_watch(&sem_menu_button[i].selected);

#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)
	//Updater
	Util_remove_watch(&sem_show_patch_note_request);
	Util_remove_watch(&sem_select_ver_request);
	Util_remove_watch(&sem_dl_file_request);
	Util_remove_watch(&sem_check_update_request);
	Util_remove_watch(&sem_selected_edition_num);
	Util_remove_watch(&sem_installed_size);
	Util_remove_watch((int*)&sem_dled_size);

	Util_remove_watch(&sem_close_updater_button.selected);
	Util_remove_watch(&sem_select_edtion_button.selected);
	Util_remove_watch(&sem_3dsx_button.selected);
	Util_remove_watch(&sem_cia_button.selected);
	Util_remove_watch(&sem_back_to_patch_note_button.selected);
	Util_remove_watch(&sem_dl_install_button.selected);
	Util_remove_watch(&sem_close_app_button.selected);
	Util_remove_watch(&sem_check_update_button.selected);
#endif

	//Languages
	Util_remove_watch(&sem_reload_msg_request);

	Util_remove_watch(&sem_english_button.selected);
	Util_remove_watch(&sem_japanese_button.selected);
	Util_remove_watch(&sem_hungarian_button.selected);
	Util_remove_watch(&sem_chinese_button.selected);
	Util_remove_watch(&sem_italian_button.selected);
	Util_remove_watch(&sem_spanish_button.selected);
	Util_remove_watch(&sem_romanian_button.selected);
	Util_remove_watch(&sem_polish_button.selected);
	Util_remove_watch(&sem_ryukyuan_button.selected);

	//LCD
	Util_remove_watch(&var_night_mode);
	Util_remove_watch(&var_flash_mode);
	Util_remove_watch(&var_lcd_brightness);
	Util_remove_watch(&var_top_lcd_brightness);
	Util_remove_watch(&var_bottom_lcd_brightness);
	Util_remove_watch(&sem_change_brightness_request);
	Util_remove_watch(&var_time_to_turn_off_lcd);
	Util_remove_watch((bool*)&var_screen_mode);

	Util_remove_watch(&sem_night_mode_on_button.selected);
	Util_remove_watch(&sem_night_mode_off_button.selected);
	Util_remove_watch(&sem_flash_mode_button.selected);
	Util_remove_watch(&sem_screen_brightness_bar.selected);
	Util_remove_watch(&sem_screen_off_time_bar.selected);
	Util_remove_watch(&sem_sleep_time_bar.selected);
	Util_remove_watch(&sem_800px_mode_button.selected);
	Util_remove_watch(&sem_3d_mode_button.selected);
	Util_remove_watch(&sem_400px_mode_button.selected);
	Util_remove_watch(&sem_auto_mode_button.selected);

	//Scroll speed
	Util_remove_watch(&var_scroll_speed);

	Util_remove_watch(&sem_scroll_speed_bar.selected);

	//Font
	Util_remove_watch(&sem_load_all_ex_font_button.selected);
	Util_remove_watch(&sem_unload_all_ex_font_button.selected);
	for(int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
		Util_remove_watch(&sem_ex_font_button[i].selected);

	//Wireless
	Util_remove_watch(&var_wifi_enabled);

	Util_remove_watch(&sem_wifi_on_button.selected);
	Util_remove_watch(&sem_wifi_off_button.selected);

	//Advanced settings
	Util_remove_watch(&var_allow_send_app_info);
	Util_remove_watch(&var_debug_mode);

	Util_remove_watch(&sem_allow_send_info_button.selected);
	Util_remove_watch(&sem_deny_send_info_button.selected);
	Util_remove_watch(&sem_debug_mode_on_button.selected);
	Util_remove_watch(&sem_debug_mode_off_button.selected);
	Util_remove_watch(&sem_use_fake_model_button.selected);
	Util_remove_watch(&sem_dump_log_button.selected);

#if DEF_ENABLE_CPU_MONITOR_API
	Util_remove_watch(&sem_monitor_cpu_usage_on_button.selected);
	Util_remove_watch(&sem_monitor_cpu_usage_off_button.selected);
#endif

	//Battery
	Util_remove_watch(&var_eco_mode);
	Util_remove_watch(&sem_eco_mode_on_button.selected);
	Util_remove_watch(&sem_eco_mode_off_button.selected);

#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)
	//Screen recording
	Util_remove_watch(&sem_record_request);
	Util_remove_watch(&sem_stop_record_request);
#endif

	Util_remove_watch(&sem_record_both_lcd_button.selected);
	Util_remove_watch(&sem_record_top_lcd_button.selected);
	Util_remove_watch(&sem_record_bottom_lcd_button.selected);

	Util_log_save(DEF_SEM_EXIT_STR, "Exited.");
}

void Sem_main(void)
{
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;
	int cache_color[DEF_EXFONT_NUM_OF_FONT_NAME];
	double draw_x;
	double draw_y;

	if (var_night_mode)
	{
		color = DEF_DRAW_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	for(int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
		cache_color[i] = color;

	if(Util_is_watch_changed() || var_need_reflesh || !var_eco_mode)
	{
		var_need_reflesh = false;
		Draw_frame_ready();
		Draw_screen_ready(SCREEN_TOP_LEFT, back_color);

		if(Util_log_query_log_show_flag())
			Util_log_draw();
	
		Draw_top_ui();

		if(var_monitor_cpu_usage)
			Draw_cpu_usage_info();

		Draw_screen_ready(SCREEN_BOTTOM, back_color);

		if (sem_selected_menu_mode >= DEF_SEM_MENU_UPDATE && sem_selected_menu_mode <= DEF_SEM_MENU_RECORDING)
		{
			draw_y = 0.0;
			if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
			{
				//Back
				Draw(sem_msg[DEF_SEM_BACK_MSG], 0.0, draw_y + sem_y_offset, 0.55, 0.55, color, X_ALIGN_CENTER, Y_ALIGN_CENTER,
				55, 25, BACKGROUND_ENTIRE_BOX, &sem_back_button, sem_back_button.selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED);
			}
		}

		//Scroll bar.
		if (sem_selected_menu_mode == DEF_SEM_MENU_LANGAGES || sem_selected_menu_mode == DEF_SEM_MENU_LCD
		|| sem_selected_menu_mode == DEF_SEM_MENU_FONT)
		{
			Draw_texture(var_square_image[0], color, 312.5, 0.0, 7.5, 15.0);
			Draw_texture(var_square_image[0], color, 312.5, 215.0, 7.5, 10.0);
			Draw_texture(&sem_scroll_bar, sem_scroll_bar.selected ? DEF_DRAW_BLUE : DEF_DRAW_WEAK_BLUE, 312.5, 15.0 + (195 * (sem_y_offset / sem_y_max)), 7.5, 5.0);
		}

		if (sem_selected_menu_mode == DEF_SEM_MENU_TOP)
		{
			//Update
			Draw(sem_msg[DEF_SEM_UPDATE_MSG], 0, 0, 0.75, 0.75, color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_UPDATE], sem_menu_button[DEF_SEM_MENU_UPDATE].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Lang
			Draw(sem_msg[DEF_SEM_LANGAGES_MSG], 0, 25, 0.75, 0.75, color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_LANGAGES], sem_menu_button[DEF_SEM_MENU_LANGAGES].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//LCD
			Draw(sem_msg[DEF_SEM_LCD_MSG], 0, 50, 0.75, 0.75, color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_LCD], sem_menu_button[DEF_SEM_MENU_LCD].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Control
			Draw(sem_msg[DEF_SEM_CONTROL_MSG], 0, 75, 0.75, 0.75, color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_CONTROL], sem_menu_button[DEF_SEM_MENU_CONTROL].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Font
			Draw(sem_msg[DEF_SEM_FONT_MSG], 0, 100, 0.75, 0.75, color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_FONT], sem_menu_button[DEF_SEM_MENU_FONT].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Wireless
			Draw(sem_msg[DEF_SEM_WIFI_MSG], 0, 125, 0.75, 0.75, color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_WIFI], sem_menu_button[DEF_SEM_MENU_WIFI].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Advanced
			Draw(sem_msg[DEF_SEM_ADVANCED_MSG], 0, 150, 0.75, 0.75, color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_ADVANCED], sem_menu_button[DEF_SEM_MENU_ADVANCED].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Battery
			Draw(sem_msg[DEF_SEM_BATTERY_MSG], 0, 175, 0.75, 0.75, color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_BATTERY], sem_menu_button[DEF_SEM_MENU_BATTERY].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Screen recording
			Draw(sem_msg[DEF_SEM_RECORDING_MSG], 0, 200, 0.75, 0.75, color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_menu_button[DEF_SEM_MENU_RECORDING], sem_menu_button[DEF_SEM_MENU_RECORDING].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_UPDATE)
		{
#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)
			//Check for updates
			Draw(sem_msg[DEF_SEM_CHECK_UPDATE_MSG], 10, 25, 0.75, 0.75, color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_check_update_button, sem_check_update_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			if (sem_show_patch_note_request)
			{
				Draw_texture(var_square_image[0], DEF_DRAW_AQUA, 15, 15, 290, 200);
				Draw_texture(&sem_select_edtion_button, sem_select_edtion_button.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, 15, 200, 145, 15);
				Draw_texture(&sem_close_updater_button, sem_close_updater_button.selected ? DEF_DRAW_WHITE : DEF_DRAW_WEAK_WHITE, 160, 200, 145, 15);

				if(sem_update_progress == 0)//checking...
					Draw(sem_msg[DEF_SEM_CHECKING_UPDATE_MSG], 17.5, 15, 0.5, 0.5, DEF_DRAW_BLACK);
				else if(sem_update_progress == -1)//failed
					Draw(sem_msg[DEF_SEM_CHECKING_UPDATE_FAILED_MSG], 17.5, 15, 0.5, 0.5, DEF_DRAW_BLACK);
				else if (sem_update_progress == 1)//success
				{
					Draw(sem_msg[sem_new_version_available ? DEF_SEM_NEW_VERSION_AVAILABLE_MSG : DEF_SEM_UP_TO_DATE_MSG], 17.5, 15, 0.5, 0.5, DEF_DRAW_BLACK);
					Draw(sem_newest_ver_data[5], 17.5, 35, 0.425, 0.425, DEF_DRAW_BLACK);
				}
				if(var_lang == "ro")
					Draw(sem_msg[DEF_SEM_SELECT_EDITION_MSG], 17.5, 200, 0.35, 0.35, DEF_DRAW_BLACK);
				else
					Draw(sem_msg[DEF_SEM_SELECT_EDITION_MSG], 17.5, 200, 0.425, 0.425, DEF_DRAW_BLACK);

				Draw(sem_msg[DEF_SEM_CLOSE_UPDATER_MSG], 162.5, 200, 0.425, 0.425, DEF_DRAW_BLACK);
			}
			else if (sem_select_ver_request)
			{
				Draw_texture(var_square_image[0], DEF_DRAW_AQUA, 15, 15, 290, 200);
				Draw_texture(&sem_back_to_patch_note_button, sem_back_to_patch_note_button.selected ? DEF_DRAW_WHITE : DEF_DRAW_WEAK_WHITE, 15, 200, 145, 15);
				Draw_texture(&sem_dl_install_button, sem_dl_install_button.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, 160, 200, 145, 15);
				Draw_texture(&sem_3dsx_button, sem_3dsx_button.selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 15, 15, 100, 25);
				Draw_texture(&sem_cia_button, sem_cia_button.selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED, 15, 45, 100, 25);

				//3dsx
				if(sem_selected_edition_num == DEF_SEM_EDTION_3DSX)
					Draw(sem_msg[DEF_SEM_3DSX_MSG], 17.5, 15, 0.8, 0.8, DEF_DRAW_RED);
				else if(sem_newest_ver_data[1] == "1")
					Draw(sem_msg[DEF_SEM_3DSX_MSG], 17.5, 15, 0.8, 0.8, DEF_DRAW_BLACK);
				else
					Draw(sem_msg[DEF_SEM_3DSX_MSG], 17.5, 15, 0.8, 0.8, DEF_DRAW_WEAK_BLACK);

				//cia
				if(sem_selected_edition_num == DEF_SEM_EDTION_CIA)
					Draw(sem_msg[DEF_SEM_CIA_MSG], 17.5, 45, 0.8, 0.8, DEF_DRAW_RED);
				else if(sem_newest_ver_data[2] == "1")
					Draw(sem_msg[DEF_SEM_CIA_MSG], 17.5, 45, 0.8, 0.8, DEF_DRAW_BLACK);
				else
					Draw(sem_msg[DEF_SEM_CIA_MSG], 17.5, 45, 0.8, 0.8, DEF_DRAW_WEAK_BLACK);

				if (sem_selected_edition_num == DEF_SEM_EDTION_3DSX)
				{
					Draw(sem_msg[DEF_SEM_FILE_PATH_MSG], 17.5, 140, 0.5, 0.5, DEF_DRAW_BLACK);
					Draw("sdmc:" + DEF_UPDATE_DIR_PREFIX + sem_newest_ver_data[0] + "/" + DEF_UPDATE_FILE_PREFIX + ".3dsx", 17.5, 150, 0.425, 0.425, DEF_DRAW_RED);
				}

				if(sem_update_progress == 2)
				{
					//downloading...
					Draw(std::to_string(sem_dled_size / 1024.0 / 1024.0).substr(0, 4) + "MB(" + std::to_string(sem_dled_size / 1024) + "KB)", 17.5, 180, 0.425, 0.425, DEF_DRAW_BLACK);
					Draw(sem_msg[DEF_SEM_DOWNLOADING_MSG], 17.5, 160, 0.75, 0.75, DEF_DRAW_BLACK);
				}
				else if(sem_update_progress == 3)
				{
					//installing...
					Draw(std::to_string(sem_installed_size / 1024.0 / 1024.0).substr(0, 4) + "MB/" + std::to_string(sem_total_cia_size / 1024.0 / 1024.0).substr(0, 4) + "MB", 17.5, 180, 0.425, 0.425, DEF_DRAW_BLACK);
					Draw(sem_msg[DEF_SEM_INSTALLING_MSG], 17.5, 160, 0.75, 0.75, DEF_DRAW_BLACK);
				}
				else if (sem_update_progress == 4)
				{
					//success
					Draw(sem_msg[DEF_SEM_SUCCESS_MSG], 17.5, 160, 0.75, 0.75, DEF_DRAW_BLACK);
					Draw(sem_msg[DEF_SEM_RESTART_MSG], 17.5, 180, 0.45, 0.45, DEF_DRAW_BLACK);
					Draw_texture(&sem_close_app_button, sem_close_app_button.selected ? DEF_DRAW_YELLOW : DEF_DRAW_WEAK_YELLOW, 250, 180, 55.0, 20.0);
					Draw(sem_msg[DEF_SEM_CLOSE_APP_MSG], 250, 180, 0.375, 0.375, DEF_DRAW_BLACK);
				}
				else if (sem_update_progress == -2)
					Draw(sem_msg[DEF_SEM_FAILURE_MSG], 17.5, 160, 0.75, 0.75, DEF_DRAW_BLACK);

				Draw(sem_msg[DEF_SEM_DL_INSTALL_MSG], 162.5, 200, 0.425, 0.425, (sem_selected_edition_num != DEF_SEM_EDTION_NONE && sem_newest_ver_data[1 + sem_selected_edition_num] == "1") ? DEF_DRAW_BLACK : DEF_DRAW_WEAK_BLACK);
				Draw(sem_msg[DEF_SEM_BACK_TO_PATCH_NOTE_MSG], 17.5, 200, 0.45, 0.45, DEF_DRAW_BLACK);
			}
#else
			Draw("☢Updater is disabled\non this app.☢", 10, 25, 0.75, 0.75, DEF_DRAW_RED);
#endif
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_LANGAGES)
		{
			//Languages

			//English
			Draw(sem_msg[DEF_SEM_ENGLISH_MSG], 10, 25 + sem_y_offset, 0.75, 0.75, (var_lang == "en") ? DEF_DRAW_RED : color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_english_button, sem_english_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Japanese
			Draw(sem_msg[DEF_SEM_JAPANESE_MSG], 10, 50 + sem_y_offset, 0.75, 0.75, (var_lang == "jp") ? DEF_DRAW_RED : color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_japanese_button, sem_japanese_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Hungarian
			Draw(sem_msg[DEF_SEM_HUNGARIAN_MSG], 10, 75 + sem_y_offset, 0.75, 0.75, (var_lang == "hu") ? DEF_DRAW_RED : color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_hungarian_button, sem_hungarian_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Chinese
			Draw(sem_msg[DEF_SEM_CHINESE_MSG], 10, 100 + sem_y_offset, 0.75, 0.75, (var_lang == "zh-cn") ? DEF_DRAW_RED : color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_chinese_button, sem_chinese_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Italian
			Draw(sem_msg[DEF_SEM_ITALIAN_MSG], 10, 125 + sem_y_offset, 0.75, 0.75, (var_lang == "it") ? DEF_DRAW_RED : color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_italian_button, sem_italian_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Spanish
			Draw(sem_msg[DEF_SEM_SPANISH_MSG], 10, 150 + sem_y_offset, 0.75, 0.75, (var_lang == "es") ? DEF_DRAW_RED : color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_spanish_button, sem_spanish_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Romanian
			Draw(sem_msg[DEF_SEM_ROMANIAN_MSG], 10, 175 + sem_y_offset, 0.75, 0.75, (var_lang == "ro") ? DEF_DRAW_RED : color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_romanian_button, sem_romanian_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Polish
			Draw(sem_msg[DEF_SEM_POLISH_MSG], 10, 200 + sem_y_offset, 0.75, 0.75, (var_lang == "pl") ? DEF_DRAW_RED : color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_polish_button, sem_polish_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Ryukyuan
			Draw(sem_msg[DEF_SEM_RYUKYUAN_MSG], 10, 225 + sem_y_offset, 0.75, 0.75, (var_lang == "ryu") ? DEF_DRAW_RED : color, X_ALIGN_LEFT, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_ryukyuan_button, sem_ryukyuan_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_LCD)
		{
			double bar_pos = 0;

#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)
			if(sem_record_request && var_night_mode)
			{
				cache_color[0] = DEF_DRAW_WEAK_WHITE;
				cache_color[1] = DEF_DRAW_WEAK_WHITE;
				cache_color[2] = DEF_DRAW_WEAK_WHITE;
			}
			else if(sem_record_request)
			{
				cache_color[0] = DEF_DRAW_WEAK_BLACK;
				cache_color[1] = DEF_DRAW_WEAK_BLACK;
				cache_color[2] = DEF_DRAW_WEAK_BLACK;
			}
#endif

			if(var_model == CFG_MODEL_2DS && var_night_mode)
			{
				cache_color[0] = DEF_DRAW_WEAK_WHITE;
				cache_color[1] = DEF_DRAW_WEAK_WHITE;
			}
			else if(var_model == CFG_MODEL_2DS)
			{
				cache_color[0] = DEF_DRAW_WEAK_BLACK;
				cache_color[1] = DEF_DRAW_WEAK_BLACK;
			}

			if(var_model == CFG_MODEL_N2DSXL && var_night_mode)
				cache_color[1] = DEF_DRAW_WEAK_WHITE;
			else if(var_model == CFG_MODEL_N2DSXL)
				cache_color[1] = DEF_DRAW_WEAK_BLACK;

			//Night mode
			draw_y = 25;
			Draw(sem_msg[DEF_SEM_NIGHT_MODE_MSG], 0, draw_y + sem_y_offset, 0.5, 0.5, color);

			//ON
			draw_y += 20;
			Draw(sem_msg[DEF_SEM_ON_MSG], 10, draw_y + sem_y_offset, 0.55, 0.55, var_night_mode ? DEF_DRAW_RED : color, X_ALIGN_CENTER, Y_ALIGN_CENTER, 140, 20,
			BACKGROUND_ENTIRE_BOX, &sem_night_mode_on_button, sem_night_mode_on_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//OFF
			Draw(sem_msg[DEF_SEM_OFF_MSG], 170, draw_y + sem_y_offset, 0.55, 0.55, var_night_mode ? color : DEF_DRAW_RED, X_ALIGN_CENTER, Y_ALIGN_CENTER, 140, 20,
			BACKGROUND_ENTIRE_BOX, &sem_night_mode_off_button, sem_night_mode_off_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Flash
			draw_y += 25;
			Draw(sem_msg[DEF_SEM_FLASH_MSG], 10, draw_y + sem_y_offset, 0.8, 0.8, var_flash_mode ? DEF_DRAW_RED : color, X_ALIGN_CENTER, Y_ALIGN_CENTER, 300, 20,
			BACKGROUND_ENTIRE_BOX, &sem_flash_mode_button, sem_flash_mode_button.selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED);

			//Screen brightness.
			draw_y += 30;
			bar_pos = 10 + (290 * (var_lcd_brightness / 180.0));
			Draw(sem_msg[DEF_SEM_BRIGHTNESS_MSG] + std::to_string(var_lcd_brightness), 0, draw_y + sem_y_offset, 0.5, 0.5, color);
			//Bar
			draw_y += 15;
			Draw_texture(&sem_screen_brightness_slider, DEF_DRAW_WEAK_RED, 10, draw_y + sem_y_offset + 6.5, 300, 7);
			Draw_texture(&sem_screen_brightness_bar, sem_screen_brightness_bar.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, bar_pos, draw_y + sem_y_offset, 10, 20);

			//Time to turn off LCDs.
			draw_y += 25;
			if(var_time_to_turn_off_lcd > 0)
			{
				bar_pos = 10 + (290 * ((var_time_to_turn_off_lcd - 20) / 580.0));
				Draw(sem_msg[DEF_SEM_LCD_OFF_TIME_0_MSG] + std::to_string(var_time_to_turn_off_lcd) + sem_msg[DEF_SEM_LCD_OFF_TIME_1_MSG], 0, draw_y + sem_y_offset, 0.5, 0.5, color);
			}
			else
			{
				//Never turn off LCD automatically.
				bar_pos = 300;
				Draw(sem_msg[DEF_SEM_LCD_OFF_TIME_0_MSG] + sem_msg[DEF_SEM_OFF_MSG], 0, draw_y + sem_y_offset, 0.5, 0.5, color);
			}

			//Bar
			draw_y += 15;
			Draw_texture(&sem_screen_off_time_slider, DEF_DRAW_WEAK_RED, 10, draw_y + sem_y_offset + 6.5, 300, 7);
			Draw_texture(&sem_screen_off_time_bar, sem_screen_off_time_bar.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, bar_pos, draw_y + sem_y_offset, 10, 20);

			//Time to enter sleep.
			draw_y += 25;
			if(var_time_to_enter_sleep > 0)
			{
				bar_pos = 10 + (290 * ((var_time_to_enter_sleep - 20) / 580.0));
				Draw(sem_msg[DEF_SEM_SLEEP_TIME_MSG] + std::to_string(var_time_to_enter_sleep) + sem_msg[DEF_SEM_LCD_OFF_TIME_1_MSG], 0, draw_y + sem_y_offset, 0.5, 0.5, color);
			}
			else
			{
				//Never enter sleep automatically.
				bar_pos = 300;
				Draw(sem_msg[DEF_SEM_SLEEP_TIME_MSG] + sem_msg[DEF_SEM_OFF_MSG], 0, draw_y + sem_y_offset, 0.5, 0.5, color);
			}

			//Bar
			draw_y += 15;
			Draw_texture(&sem_sleep_time_slider, DEF_DRAW_WEAK_RED, 10, draw_y + sem_y_offset + 6.5, 300, 7);
			Draw_texture(&sem_sleep_time_bar, sem_sleep_time_bar.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, bar_pos, draw_y + sem_y_offset, 10, 20);

			//Screen mode
			draw_y += 25;
			Draw(sem_msg[DEF_SEM_LCD_MODE_MSG], 0, draw_y + sem_y_offset, 0.5, 0.5, color);

			//800px
			draw_y += 15;
			Draw(sem_msg[DEF_SEM_800PX_MSG], 10, draw_y + sem_y_offset, 0.65, 0.65, (var_screen_mode == DEF_SEM_SCREEN_800PX) ? DEF_DRAW_RED : cache_color[0], X_ALIGN_CENTER, Y_ALIGN_CENTER, 65, 20,
			BACKGROUND_ENTIRE_BOX, &sem_800px_mode_button, sem_800px_mode_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//3D
			Draw(sem_msg[DEF_SEM_3D_MSG], 85, draw_y + sem_y_offset, 0.65, 0.65, (var_screen_mode == DEF_SEM_SCREEN_3D) ? DEF_DRAW_RED : cache_color[1], X_ALIGN_CENTER, Y_ALIGN_CENTER, 65, 20,
			BACKGROUND_ENTIRE_BOX, &sem_3d_mode_button, sem_3d_mode_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//Nothing
			Draw(sem_msg[DEF_SEM_400PX_MSG], 160, draw_y + sem_y_offset, 0.65, 0.65, (var_screen_mode == DEF_SEM_SCREEN_400PX) ? DEF_DRAW_RED : cache_color[2], X_ALIGN_CENTER, Y_ALIGN_CENTER, 65, 20,
			BACKGROUND_ENTIRE_BOX, &sem_400px_mode_button, sem_400px_mode_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//Auto
			Draw(sem_msg[DEF_SEM_AUTO_MSG], 235, draw_y + sem_y_offset, 0.65, 0.65, (var_screen_mode == DEF_SEM_SCREEN_AUTO) ? DEF_DRAW_RED : cache_color[2], X_ALIGN_CENTER, Y_ALIGN_CENTER, 65, 20,
			BACKGROUND_ENTIRE_BOX, &sem_auto_mode_button, sem_auto_mode_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_CONTROL)
		{
			//Scroll speed
			double bar_pos = 10 + (290 * ((var_scroll_speed - 0.05) / 1.95));
			Draw(sem_msg[DEF_SEM_SCROLL_SPEED_MSG] + std::to_string(var_scroll_speed), 0, 25, 0.5, 0.5, color);
			//Bar
			Draw_texture(&sem_scroll_speed_slider, DEF_DRAW_WEAK_RED, 10, 46.5, 300, 7);
			Draw_texture(&sem_scroll_speed_bar, sem_scroll_speed_bar.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN, bar_pos, 40, 10, 20);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_FONT)
		{
			//Font
			if (30 + sem_y_offset >= -30 && 30 + sem_y_offset <= 240)
			{
				cache_color[0] = color;
				if ((Exfont_is_unloading_external_font() || Exfont_is_loading_external_font()) && var_night_mode)
					cache_color[0] = DEF_DRAW_WEAK_WHITE;
				else if (Exfont_is_unloading_external_font() || Exfont_is_loading_external_font())
					cache_color[0] = DEF_DRAW_WEAK_BLACK;

				//Load all
				Draw(sem_msg[DEF_SEM_LOAD_ALL_FONT_MSG], 10, 30 + sem_y_offset, 0.65, 0.65, cache_color[0], X_ALIGN_CENTER, Y_ALIGN_CENTER, 150, 20,
				BACKGROUND_ENTIRE_BOX, &sem_load_all_ex_font_button, sem_load_all_ex_font_button.selected ? DEF_DRAW_RED : DEF_DRAW_WEAK_RED);

				//Unload all
				Draw(sem_msg[DEF_SEM_UNLOAD_ALL_FONT_MSG], 160, 30 + sem_y_offset, 0.65, 0.65, cache_color[0], X_ALIGN_CENTER, Y_ALIGN_CENTER, 150, 20,
				BACKGROUND_ENTIRE_BOX, &sem_unload_all_ex_font_button, sem_unload_all_ex_font_button.selected ? DEF_DRAW_YELLOW : DEF_DRAW_WEAK_YELLOW);
			}

			draw_x = 10.0;
			draw_y = 50.0;
			for(int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
				cache_color[i] = color;

			for (int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
			{
				if (Exfont_is_loaded_external_font(i))
				{
					if(Exfont_is_unloading_external_font() || Exfont_is_loading_external_font())
						cache_color[i] = DEF_DRAW_WEAK_RED;
					else
						cache_color[i] = DEF_DRAW_RED;
				}
				else if ((Exfont_is_unloading_external_font() || Exfont_is_loading_external_font()) && var_night_mode)
					cache_color[i] = DEF_DRAW_WEAK_WHITE;
				else if (Exfont_is_unloading_external_font() || Exfont_is_loading_external_font())
					cache_color[i] = DEF_DRAW_WEAK_BLACK;

				if (draw_y + sem_y_offset >= -30 && draw_y + sem_y_offset <= 240)
				{
					Draw(Exfont_query_external_font_name(i), draw_x, draw_y + sem_y_offset, 0.45, 0.45, cache_color[i], X_ALIGN_LEFT, Y_ALIGN_CENTER, 300, 20,
					BACKGROUND_ENTIRE_BOX, &sem_ex_font_button[i], sem_ex_font_button[i].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
				}
				draw_y += 20.0;
			}
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_WIFI)
		{
			//Wifi
			Draw(sem_msg[DEF_SEM_WIFI_MODE_MSG], 0, 25, 0.5, 0.5, color);
			//ON
			Draw(sem_msg[DEF_SEM_ON_MSG], 10, 40, 0.55, 0.55, var_wifi_enabled ? DEF_DRAW_RED : color, X_ALIGN_CENTER, Y_ALIGN_CENTER, 90, 20,
			BACKGROUND_ENTIRE_BOX, &sem_wifi_on_button, sem_wifi_on_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//OFF
			Draw(sem_msg[DEF_SEM_OFF_MSG], 110, 40, 0.55, 0.55, var_wifi_enabled ? color : DEF_DRAW_RED, X_ALIGN_CENTER, Y_ALIGN_CENTER, 90, 20,
			BACKGROUND_ENTIRE_BOX, &sem_wifi_off_button, sem_wifi_off_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Connected SSID
			Draw(sem_msg[DEF_SEM_CONNECTED_SSID_MSG] + var_connected_ssid, 0, 65, 0.425, 0.425, color);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_ADVANCED)
		{
			//Allow send app info
			Draw(sem_msg[DEF_SEM_SEND_INFO_MODE_MSG], 0, 25, 0.5, 0.5, color);
			//Allow
			Draw(sem_msg[DEF_SEM_ALLOW_MSG], 10, 40, 0.65, 0.65, var_allow_send_app_info ? DEF_DRAW_RED : color, X_ALIGN_CENTER, Y_ALIGN_CENTER, 90, 20,
			BACKGROUND_ENTIRE_BOX, &sem_allow_send_info_button, sem_allow_send_info_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			//Deny
			Draw(sem_msg[DEF_SEM_DENY_MSG], 110, 40, 0.65, 0.65, var_allow_send_app_info ? color : DEF_DRAW_RED, X_ALIGN_CENTER, Y_ALIGN_CENTER, 90, 20,
			BACKGROUND_ENTIRE_BOX, &sem_deny_send_info_button, sem_deny_send_info_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Debug mode
			Draw(sem_msg[DEF_SEM_DEBUG_MODE_MSG], 0, 65, 0.5, 0.5, color);
			//ON
			Draw(sem_msg[DEF_SEM_ON_MSG], 10, 80, 0.55, 0.55, var_debug_mode ? DEF_DRAW_RED : color, X_ALIGN_CENTER, Y_ALIGN_CENTER, 90, 20,
			BACKGROUND_ENTIRE_BOX, &sem_debug_mode_on_button, sem_debug_mode_on_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//OFF
			Draw(sem_msg[DEF_SEM_OFF_MSG], 110, 80, 0.55, 0.55, var_debug_mode ? color : DEF_DRAW_RED, X_ALIGN_CENTER, Y_ALIGN_CENTER, 90, 20,
			BACKGROUND_ENTIRE_BOX, &sem_debug_mode_off_button, sem_debug_mode_off_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Fake model
			Draw(sem_msg[DEF_SEM_FAKE_MODEL_MSG], 0, 105, 0.5, 0.5, color);
			if(sem_fake_model_num <= 5)
			{
				Draw(sem_msg[DEF_SEM_ON_MSG] + " (" + var_model_name[sem_fake_model_num] + ")", 10, 135, 0.65, 0.65, color, X_ALIGN_CENTER, Y_ALIGN_CENTER, 190, 20,
				BACKGROUND_ENTIRE_BOX, &sem_use_fake_model_button, sem_use_fake_model_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			}
			else
			{
				Draw(sem_msg[DEF_SEM_OFF_MSG], 10, 135, 0.65, 0.65, color, X_ALIGN_CENTER, Y_ALIGN_CENTER, 190, 20,
				BACKGROUND_ENTIRE_BOX, &sem_use_fake_model_button, sem_use_fake_model_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			}

			Draw(sem_msg[DEF_SEM_DUMP_LOGS_MSG], 10, 165, 0.5, 0.5, color, X_ALIGN_CENTER, Y_ALIGN_CENTER, 190, 20,
			BACKGROUND_ENTIRE_BOX, &sem_dump_log_button, sem_dump_log_button.selected  ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

#if DEF_ENABLE_CPU_MONITOR_API
			//CPU usage monitor
			Draw(sem_msg[DEF_SEM_CPU_USAGE_MONITOR_MSG], 0, 185, 0.5, 0.5, color);
			//ON
			Draw(sem_msg[DEF_SEM_ON_MSG], 10, 200, 0.55, 0.55, var_monitor_cpu_usage ? DEF_DRAW_RED : color, X_ALIGN_CENTER, Y_ALIGN_CENTER, 90, 20,
			BACKGROUND_ENTIRE_BOX, &sem_monitor_cpu_usage_on_button, sem_monitor_cpu_usage_on_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//OFF
			Draw(sem_msg[DEF_SEM_OFF_MSG], 110, 200, 0.55, 0.55, var_monitor_cpu_usage ? color : DEF_DRAW_RED, X_ALIGN_CENTER, Y_ALIGN_CENTER, 90, 20,
			BACKGROUND_ENTIRE_BOX, &sem_monitor_cpu_usage_off_button, sem_monitor_cpu_usage_off_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
#endif
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_BATTERY)
		{
			//Eco mode
			Draw(sem_msg[DEF_SEM_ECO_MODE_MSG], 0, 25, 0.5, 0.5, color);
			//ON
			Draw(sem_msg[DEF_SEM_ON_MSG], 10, 40, 0.55, 0.55, var_eco_mode ? DEF_DRAW_RED : color, X_ALIGN_CENTER, Y_ALIGN_CENTER, 90, 20,
			BACKGROUND_ENTIRE_BOX, &sem_eco_mode_on_button, sem_eco_mode_on_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//OFF
			Draw(sem_msg[DEF_SEM_OFF_MSG], 110, 40, 0.55, 0.55, var_eco_mode ? color : DEF_DRAW_RED, X_ALIGN_CENTER, Y_ALIGN_CENTER, 90, 20,
			BACKGROUND_ENTIRE_BOX, &sem_eco_mode_off_button, sem_eco_mode_off_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
		}
		else if (sem_selected_menu_mode == DEF_SEM_MENU_RECORDING)
		{
#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)
			if(var_screen_mode == DEF_SEM_SCREEN_800PX && var_night_mode)
				cache_color[0] = DEF_DRAW_WEAK_WHITE;
			else if(var_screen_mode == DEF_SEM_SCREEN_800PX)
				cache_color[0] = DEF_DRAW_WEAK_BLACK;

			//Record both screen
			Draw(sem_msg[sem_record_request ? DEF_SEM_STOP_RECORDING_MSG : DEF_SEM_RECORD_BOTH_LCD_MSG], 10, 25, 0.475, 0.475, cache_color[0], X_ALIGN_CENTER, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_record_both_lcd_button, sem_record_both_lcd_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Record top screen
			Draw(sem_msg[sem_record_request ? DEF_SEM_STOP_RECORDING_MSG : DEF_SEM_RECORD_TOP_LCD_MSG], 10, 60, 0.475, 0.475, cache_color[0], X_ALIGN_CENTER, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_record_top_lcd_button, sem_record_top_lcd_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			//Record bottom screen
			Draw(sem_msg[sem_record_request ? DEF_SEM_STOP_RECORDING_MSG : DEF_SEM_RECORD_BOTTOM_LCD_MSG], 10, 95, 0.475, 0.475, cache_color[0], X_ALIGN_CENTER, Y_ALIGN_CENTER, 240, 20,
			BACKGROUND_ENTIRE_BOX, &sem_record_bottom_lcd_button, sem_record_bottom_lcd_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			if(var_screen_mode == DEF_SEM_SCREEN_800PX)
				Draw(sem_msg[DEF_SEM_CANNOT_RECORD_MSG], 10, 120, 0.5, 0.5, DEF_DRAW_RED);
#else
			Draw("☢Screen recorder is disabled\non this app.☢", 10, 25, 0.75, 0.75, DEF_DRAW_RED);
#endif
		}

		if(Util_err_query_error_show_flag())
			Util_err_draw();

		Draw_bot_ui();

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}

void Sem_hid(Hid_info key)
{
	int menu_button_list[9] = { DEF_SEM_MENU_UPDATE, DEF_SEM_MENU_LANGAGES, DEF_SEM_MENU_LCD, DEF_SEM_MENU_CONTROL,
	DEF_SEM_MENU_FONT, DEF_SEM_MENU_WIFI, DEF_SEM_MENU_ADVANCED, DEF_SEM_MENU_BATTERY, DEF_SEM_MENU_RECORDING };
	Result_with_string result;

	if(aptShouldJumpToHome())
		return;

	if (Util_err_query_error_show_flag())
		Util_err_main(key);
	else
	{
		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Sem_suspend();
		else if (sem_selected_menu_mode == DEF_SEM_MENU_TOP)
		{
			for(int i = 0; i < 9; i++)
			{
				if(Util_hid_is_pressed(key, sem_menu_button[menu_button_list[i]]))
					sem_menu_button[menu_button_list[i]].selected = true;
				else if(Util_hid_is_released(key, sem_menu_button[menu_button_list[i]]) && sem_menu_button[menu_button_list[i]].selected)
				{
					sem_y_offset = 0.0;
					sem_selected_menu_mode = menu_button_list[i];
					if (sem_selected_menu_mode == DEF_SEM_MENU_LANGAGES)
						sem_y_max = -50.0;
					else if (sem_selected_menu_mode == DEF_SEM_MENU_LCD)
						sem_y_max = -60.0;
					else if (sem_selected_menu_mode == DEF_SEM_MENU_FONT)
						sem_y_max = -950.0;
				}
			}
		}
		else if(sem_selected_menu_mode >= DEF_SEM_MENU_UPDATE && sem_selected_menu_mode <= DEF_SEM_MENU_RECORDING)
		{
			bool enable_back_button = true;

#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)
			enable_back_button = (!sem_show_patch_note_request && !sem_select_ver_request);
#endif

			if (Util_hid_is_pressed(key, sem_back_button) && enable_back_button)
				sem_back_button.selected = true;
			else if (Util_hid_is_released(key, sem_back_button) && sem_back_button.selected && enable_back_button)
			{
				sem_y_offset = 0.0;
				sem_y_max = 0.0;
				sem_selected_menu_mode = DEF_SEM_MENU_TOP;
			}
			else if (sem_back_button.selected && !Util_hid_is_held(key, sem_back_button))
				sem_back_button.selected = false;
			else if (sem_selected_menu_mode == DEF_SEM_MENU_UPDATE)//Check for updates
			{
#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)
				if (sem_show_patch_note_request)
				{
					if (Util_hid_is_pressed(key, sem_close_updater_button))
						sem_close_updater_button.selected = true;
					else if (key.p_b || (Util_hid_is_released(key, sem_close_updater_button) && sem_close_updater_button.selected))
						sem_show_patch_note_request = false;
					else if (Util_hid_is_pressed(key, sem_select_edtion_button))
						sem_select_edtion_button.selected = true;
					else if (key.p_a || (Util_hid_is_released(key, sem_select_edtion_button) && sem_select_edtion_button.selected))
					{
						sem_show_patch_note_request = false;
						sem_select_ver_request = true;
					}
				}
				else if (sem_select_ver_request && !sem_dl_file_request)
				{
					if (Util_hid_is_pressed(key, sem_3dsx_button) && sem_newest_ver_data[1] == "1")
						sem_3dsx_button.selected = true;
					else if (Util_hid_is_released(key, sem_3dsx_button) && sem_newest_ver_data[1] == "1" && sem_3dsx_button.selected)
						sem_selected_edition_num = DEF_SEM_EDTION_3DSX;
					else if (Util_hid_is_pressed(key, sem_cia_button) && sem_newest_ver_data[2] == "1")
						sem_cia_button.selected = true;
					else if (Util_hid_is_released(key, sem_cia_button) && sem_newest_ver_data[2] == "1" && sem_cia_button.selected)
						sem_selected_edition_num = DEF_SEM_EDTION_CIA;
					else if (Util_hid_is_pressed(key, sem_back_to_patch_note_button))
						sem_back_to_patch_note_button.selected = true;
					else if (key.p_b || (Util_hid_is_released(key, sem_back_to_patch_note_button) && sem_back_to_patch_note_button.selected))
					{
						sem_show_patch_note_request = true;
						sem_select_ver_request = false;
					}
					else if (Util_hid_is_pressed(key, sem_dl_install_button) && sem_selected_edition_num != DEF_SEM_EDTION_NONE && sem_newest_ver_data[1 + sem_selected_edition_num] == "1")
						sem_dl_install_button.selected = true;
					else if ((key.p_x || (Util_hid_is_released(key, sem_dl_install_button) && sem_dl_install_button.selected)) && sem_selected_edition_num != DEF_SEM_EDTION_NONE && sem_newest_ver_data[1 + sem_selected_edition_num] == "1")
						sem_dl_file_request = true;
					else if(Util_hid_is_pressed(key, sem_close_app_button) && sem_update_progress == 4)
						sem_close_app_button.selected = true;
					else if(Util_hid_is_released(key, sem_close_app_button) && sem_update_progress == 4 && sem_close_app_button.selected)
						Menu_set_must_exit_flag(true);
				}
				else
				{
					if(Util_hid_is_pressed(key, sem_check_update_button))
						sem_check_update_button.selected = true;
					if(Util_hid_is_released(key, sem_check_update_button) && sem_check_update_button.selected)
					{
						sem_check_update_request = true;
						sem_show_patch_note_request = true;
					}
				}
#endif
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_LANGAGES)//Language
			{
				if(key.p_touch || key.h_touch || key.r_touch)
				{
					if(!sem_reload_msg_request)
					{
						if(Util_hid_is_pressed(key, sem_english_button))
							sem_english_button.selected = true;
						else if(Util_hid_is_released(key, sem_english_button) && sem_english_button.selected)
						{
							var_lang = "en";
							sem_reload_msg_request = true;
						}
						else if(sem_english_button.selected && !Util_hid_is_held(key, sem_english_button))
							sem_english_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_japanese_button))
							sem_japanese_button.selected = true;
						else if(Util_hid_is_released(key, sem_japanese_button) && sem_japanese_button.selected)
						{
							var_lang = "jp";
							sem_reload_msg_request = true;
						}
						else if(sem_japanese_button.selected && !Util_hid_is_held(key, sem_japanese_button))
							sem_japanese_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_hungarian_button))
							sem_hungarian_button.selected = true;
						else if(Util_hid_is_released(key, sem_hungarian_button) && sem_hungarian_button.selected)
						{
							var_lang = "hu";
							sem_reload_msg_request = true;
						}
						else if(sem_hungarian_button.selected && !Util_hid_is_held(key, sem_hungarian_button))
							sem_hungarian_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_chinese_button))
							sem_chinese_button.selected = true;
						else if(Util_hid_is_released(key, sem_chinese_button) && sem_chinese_button.selected)
						{
							var_lang = "zh-cn";
							sem_reload_msg_request = true;
						}
						else if(sem_chinese_button.selected && !Util_hid_is_held(key, sem_chinese_button))
							sem_chinese_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_italian_button))
							sem_italian_button.selected = true;
						else if(Util_hid_is_released(key, sem_italian_button) && sem_italian_button.selected)
						{
							var_lang = "it";
							sem_reload_msg_request = true;
						}
						else if(sem_italian_button.selected && !Util_hid_is_held(key, sem_italian_button))
							sem_italian_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_spanish_button))
							sem_spanish_button.selected = true;
						else if(Util_hid_is_released(key, sem_spanish_button) && sem_spanish_button.selected)
						{
							var_lang = "es";
							sem_reload_msg_request = true;
						}
						else if(sem_spanish_button.selected && !Util_hid_is_held(key, sem_spanish_button))
							sem_spanish_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_romanian_button))
							sem_romanian_button.selected = true;
						else if(Util_hid_is_released(key, sem_romanian_button) && sem_romanian_button.selected)
						{
							var_lang = "ro";
							sem_reload_msg_request = true;
						}
						else if(sem_romanian_button.selected && !Util_hid_is_held(key, sem_romanian_button))
							sem_romanian_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_polish_button))
							sem_polish_button.selected = true;
						else if(Util_hid_is_released(key, sem_polish_button) && sem_polish_button.selected)
						{
							var_lang = "pl";
							sem_reload_msg_request = true;
						}
						else if(sem_polish_button.selected && !Util_hid_is_held(key, sem_polish_button))
							sem_polish_button.selected = false;
						else if(Util_hid_is_pressed(key, sem_ryukyuan_button))
							sem_ryukyuan_button.selected = true;
						else if(Util_hid_is_released(key, sem_ryukyuan_button) && sem_ryukyuan_button.selected)
						{
							var_lang = "ryu";
							sem_reload_msg_request = true;
						}
						else if(sem_ryukyuan_button.selected && !Util_hid_is_held(key, sem_ryukyuan_button))
							sem_ryukyuan_button.selected = false;
					}

					sem_scroll_mode = true;
					if(sem_english_button.selected || sem_japanese_button.selected || sem_hungarian_button.selected || sem_chinese_button.selected
					|| sem_italian_button.selected || sem_spanish_button.selected || sem_romanian_button.selected || sem_polish_button.selected
					|| sem_ryukyuan_button.selected || Draw_get_bot_ui_button()->selected)
						sem_scroll_mode = false;
				}
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_LCD)//LCD
			{
				bool record_request = false;
#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)
				record_request = sem_record_request;
#endif

				if(Util_hid_is_pressed(key, sem_night_mode_on_button))
					sem_night_mode_on_button.selected = true;
				else if(Util_hid_is_released(key, sem_night_mode_on_button) && sem_night_mode_on_button.selected)
					var_night_mode = true;
				else if(sem_night_mode_on_button.selected && !Util_hid_is_held(key, sem_night_mode_on_button))
					sem_night_mode_on_button.selected = false;
				else if(Util_hid_is_pressed(key, sem_night_mode_off_button))
					sem_night_mode_off_button.selected = true;
				else if(Util_hid_is_released(key, sem_night_mode_off_button) && sem_night_mode_off_button.selected)
					var_night_mode = false;
				else if(sem_night_mode_off_button.selected && !Util_hid_is_held(key, sem_night_mode_off_button))
					sem_night_mode_off_button.selected = false;
				else if(Util_hid_is_pressed(key, sem_flash_mode_button))
					sem_flash_mode_button.selected = true;
				else if(Util_hid_is_released(key, sem_flash_mode_button) && sem_flash_mode_button.selected)
					var_flash_mode = !var_flash_mode;
				else if(sem_flash_mode_button.selected && !Util_hid_is_held(key, sem_flash_mode_button))
					sem_flash_mode_button.selected = false;
				else if(Util_hid_is_pressed(key, sem_screen_brightness_bar) || Util_hid_is_pressed(key, sem_screen_brightness_slider)
				|| (key.h_touch && sem_screen_brightness_bar.selected))
				{
					//Update screen brightness.
					int new_brightness = 180 * ((key.touch_x - 15) / 290.0);

					if(new_brightness < 0)
						new_brightness = 0;
					else if(new_brightness > 180)
						new_brightness = 180;

					var_lcd_brightness = new_brightness;
					var_top_lcd_brightness = var_lcd_brightness;
					var_bottom_lcd_brightness = var_lcd_brightness;
					sem_change_brightness_request = true;

					sem_screen_brightness_bar.selected = true;
				}
				else if(Util_hid_is_pressed(key, sem_screen_off_time_bar) || Util_hid_is_pressed(key, sem_screen_off_time_slider)
				|| (key.h_touch && sem_screen_off_time_bar.selected))
				{
					//Update time to turn off LCD.
					int new_time = (580 * ((key.touch_x - 15) / 290.0)) + 20;

					if(new_time < 20)
						new_time = 20;
					else if(new_time > 600)
						new_time = -1;//Never turn off.

					var_time_to_turn_off_lcd = new_time;
					if(var_time_to_turn_off_lcd > 0 && var_time_to_enter_sleep > 0 && (var_time_to_turn_off_lcd > var_time_to_enter_sleep))
						var_time_to_enter_sleep = var_time_to_turn_off_lcd;
					else if(var_time_to_turn_off_lcd < 0)
						var_time_to_enter_sleep = -1;

					sem_screen_off_time_bar.selected = true;
				}
				else if(Util_hid_is_pressed(key, sem_sleep_time_bar) || Util_hid_is_pressed(key, sem_sleep_time_slider)
				|| (key.h_touch && sem_sleep_time_bar.selected))
				{
					//Update time enter sleep.
					int new_time = (580 * ((key.touch_x - 15) / 290.0)) + 20;

					if(new_time < 20)
						new_time = 20;
					else if(new_time > 600)
						new_time = -1;//Never enter sleep.

					var_time_to_enter_sleep = new_time;
					if(var_time_to_enter_sleep > 0 && var_time_to_turn_off_lcd > 0 && (var_time_to_enter_sleep < var_time_to_turn_off_lcd))
						var_time_to_turn_off_lcd = var_time_to_enter_sleep;

					sem_sleep_time_bar.selected = true;
				}
				else if (Util_hid_is_pressed(key, sem_800px_mode_button) && !record_request && var_model != CFG_MODEL_2DS)
					sem_800px_mode_button.selected = true;
				else if (Util_hid_is_released(key, sem_800px_mode_button) && !record_request && var_model != CFG_MODEL_2DS && sem_800px_mode_button.selected)
					var_screen_mode = DEF_SEM_SCREEN_800PX;
				else if(sem_800px_mode_button.selected && !Util_hid_is_held(key, sem_800px_mode_button))
					sem_800px_mode_button.selected = false;
				else if (Util_hid_is_pressed(key, sem_3d_mode_button) && !record_request && var_model != CFG_MODEL_2DS && var_model != CFG_MODEL_N2DSXL)
					sem_3d_mode_button.selected = true;
				else if (Util_hid_is_released(key, sem_3d_mode_button) && !record_request && var_model != CFG_MODEL_2DS && var_model != CFG_MODEL_N2DSXL && sem_3d_mode_button.selected)
					var_screen_mode = DEF_SEM_SCREEN_3D;
				else if(sem_3d_mode_button.selected && !Util_hid_is_held(key, sem_3d_mode_button))
					sem_3d_mode_button.selected = false;
				else if (Util_hid_is_pressed(key, sem_400px_mode_button) && !record_request)
					sem_400px_mode_button.selected = true;
				else if (Util_hid_is_released(key, sem_400px_mode_button) && !record_request && sem_400px_mode_button.selected)
					var_screen_mode = DEF_SEM_SCREEN_400PX;
				else if(sem_400px_mode_button.selected && !Util_hid_is_held(key, sem_400px_mode_button))
					sem_400px_mode_button.selected = false;
				else if (Util_hid_is_pressed(key, sem_auto_mode_button) && !record_request)
					sem_auto_mode_button.selected = true;
				else if (Util_hid_is_released(key, sem_auto_mode_button) && !record_request && sem_auto_mode_button.selected)
					var_screen_mode = DEF_SEM_SCREEN_AUTO;
				else if(sem_auto_mode_button.selected && !Util_hid_is_held(key, sem_auto_mode_button))
					sem_auto_mode_button.selected = false;

				sem_scroll_mode = true;
				if(sem_night_mode_on_button.selected || sem_night_mode_off_button.selected || sem_flash_mode_button.selected
				|| sem_screen_brightness_bar.selected || sem_screen_brightness_slider.selected || sem_screen_off_time_bar.selected
				|| sem_screen_off_time_slider.selected || sem_sleep_time_bar.selected || sem_sleep_time_slider.selected 
				|| sem_800px_mode_button.selected || sem_3d_mode_button.selected || sem_400px_mode_button.selected
				|| sem_auto_mode_button.selected || Draw_get_bot_ui_button()->selected)
					sem_scroll_mode = false;
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_CONTROL)//Scroll speed
			{
				if (Util_hid_is_pressed(key, sem_scroll_speed_slider) || Util_hid_is_pressed(key, sem_scroll_speed_bar)
				|| (key.h_touch && sem_scroll_speed_bar.selected))
				{
					//Update time to turn off LCD.
					double new_speed = (1.95 * ((key.touch_x - 15) / 290.0)) + 0.05;

					if(new_speed < 0.05)
						new_speed = 0.05;
					else if(new_speed > 2)
						new_speed = 2;

					var_scroll_speed = new_speed;
					sem_scroll_speed_bar.selected = true;
				}
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_FONT)//Font
			{
				if (Util_hid_is_pressed(key, sem_load_all_ex_font_button) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font())
					sem_load_all_ex_font_button.selected = true;
				else if (Util_hid_is_released(key, sem_load_all_ex_font_button) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font() && sem_load_all_ex_font_button.selected)
				{
					for (int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
						Exfont_set_external_font_request_state(i ,true);
					
					Exfont_request_load_external_font();
				}
				else if (Util_hid_is_pressed(key, sem_unload_all_ex_font_button) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font())
					sem_unload_all_ex_font_button.selected = true;
				else if (Util_hid_is_released(key, sem_unload_all_ex_font_button) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font() && sem_unload_all_ex_font_button.selected)
				{
					for (int i = 1; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
						Exfont_set_external_font_request_state(i ,false);

					Exfont_request_unload_external_font();
				}
				else
				{
					for (int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
					{
						if (Util_hid_is_pressed(key, sem_ex_font_button[i]) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font())
						{
							sem_ex_font_button[i].selected = true;
							break;
						}
						else if (Util_hid_is_released(key, sem_ex_font_button[i]) && !Exfont_is_loading_external_font() && !Exfont_is_unloading_external_font() && sem_ex_font_button[i].selected)
						{
							if (Exfont_is_loaded_external_font(i))
							{
								if(i != 0)
								{
									Exfont_set_external_font_request_state(i ,false);
									Exfont_request_unload_external_font();
								}
							}
							else
							{
								Exfont_set_external_font_request_state(i ,true);
								Exfont_request_load_external_font();
							}
							break;
						}
						else if(sem_ex_font_button[i].selected && !Util_hid_is_held(key, sem_ex_font_button[i]))
							sem_ex_font_button[i].selected = false;
					}
				}

				sem_scroll_mode = true;
				if(sem_load_all_ex_font_button.selected || sem_unload_all_ex_font_button.selected || Draw_get_bot_ui_button()->selected)
					sem_scroll_mode = false;

				for (int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
				{
					if(sem_ex_font_button[i].selected)
						sem_scroll_mode = false;
				}
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_WIFI)//Wireless
			{
				if (Util_hid_is_pressed(key, sem_wifi_on_button))
					sem_wifi_on_button.selected = true;
				else if (Util_hid_is_released(key, sem_wifi_on_button) && sem_wifi_on_button.selected)
				{
					result = Util_cset_set_wifi_state(true);
					if(result.code == 0 || result.code == 0xC8A06C0D)
						var_wifi_enabled = true;
				}
				else if (Util_hid_is_pressed(key, sem_wifi_off_button))
					sem_wifi_off_button.selected = true;
				else if (Util_hid_is_released(key, sem_wifi_off_button) && sem_wifi_off_button.selected)
				{
					result = Util_cset_set_wifi_state(false);
					if(result.code == 0 || result.code == 0xC8A06C0D)
						var_wifi_enabled = false;
				}
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_ADVANCED)//Advanced settings
			{
				if (Util_hid_is_pressed(key, sem_allow_send_info_button))
					sem_allow_send_info_button.selected = true;
				else if (Util_hid_is_released(key, sem_allow_send_info_button) && sem_allow_send_info_button.selected)
					var_allow_send_app_info = true;
				else if (Util_hid_is_pressed(key, sem_deny_send_info_button))
					sem_deny_send_info_button.selected = true;
				else if (Util_hid_is_released(key, sem_deny_send_info_button) && sem_deny_send_info_button.selected)
					var_allow_send_app_info = false;
				else if (Util_hid_is_pressed(key, sem_debug_mode_on_button))
					sem_debug_mode_on_button.selected = true;
				else if (Util_hid_is_released(key, sem_debug_mode_on_button) && sem_debug_mode_on_button.selected)
					var_debug_mode = true;
#if DEF_ENABLE_CPU_MONITOR_API
				else if (Util_hid_is_pressed(key, sem_monitor_cpu_usage_on_button))
					sem_monitor_cpu_usage_on_button.selected = true;
				else if (Util_hid_is_released(key, sem_monitor_cpu_usage_on_button) && sem_monitor_cpu_usage_on_button.selected)
					var_monitor_cpu_usage = true;
				else if (Util_hid_is_pressed(key, sem_monitor_cpu_usage_off_button))
					sem_monitor_cpu_usage_off_button.selected = true;
				else if (Util_hid_is_released(key, sem_monitor_cpu_usage_off_button) && sem_monitor_cpu_usage_off_button.selected)
					var_monitor_cpu_usage = false;
#endif
				else if (Util_hid_is_pressed(key, sem_debug_mode_off_button))
					sem_debug_mode_off_button.selected = true;
				else if (Util_hid_is_released(key, sem_debug_mode_off_button) && sem_debug_mode_off_button.selected)
					var_debug_mode = false;
				else if (Util_hid_is_pressed(key, sem_use_fake_model_button))
					sem_use_fake_model_button.selected = true;
				else if (Util_hid_is_released(key, sem_use_fake_model_button) && sem_use_fake_model_button.selected)
				{
					if((u8)(sem_fake_model_num + 1) > 5)
						sem_fake_model_num = 255;
					else
						sem_fake_model_num++;
					
					var_need_reflesh = true;
				}
				else if(Util_hid_is_pressed(key, sem_dump_log_button) && !sem_dump_log_request)
					sem_dump_log_button.selected = true;
				else if(Util_hid_is_released(key, sem_dump_log_button) && sem_dump_log_button.selected && !sem_dump_log_request)
					sem_dump_log_request = true;
			}
			else if (sem_selected_menu_mode == DEF_SEM_MENU_BATTERY)//Battery
			{
				if (Util_hid_is_pressed(key, sem_eco_mode_on_button))
					sem_eco_mode_on_button.selected = true;
				else if (Util_hid_is_released(key, sem_eco_mode_on_button) && sem_eco_mode_on_button.selected)
					var_eco_mode = true;
				else if (Util_hid_is_pressed(key, sem_eco_mode_off_button))
					sem_eco_mode_off_button.selected = true;
				else if (Util_hid_is_released(key, sem_eco_mode_off_button) && sem_eco_mode_off_button.selected)
					var_eco_mode = false;
			}
#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)
			else if (sem_selected_menu_mode == DEF_SEM_MENU_RECORDING)//Screen recording
			{
				if (Util_hid_is_pressed(key, sem_record_both_lcd_button) && var_screen_mode != DEF_SEM_SCREEN_800PX)
					sem_record_both_lcd_button.selected = true;
				else if (Util_hid_is_released(key, sem_record_both_lcd_button) && var_screen_mode != DEF_SEM_SCREEN_800PX && sem_record_both_lcd_button.selected)
				{
					if(sem_record_request)
						sem_stop_record_request = true;
					else
					{
						sem_selected_recording_mode = DEF_SEM_RECORD_BOTH;
						sem_record_request = true;
					}
				}
				else if (Util_hid_is_pressed(key, sem_record_top_lcd_button) && var_screen_mode != DEF_SEM_SCREEN_800PX)
					sem_record_top_lcd_button.selected = true;
				else if (Util_hid_is_released(key, sem_record_top_lcd_button) && var_screen_mode != DEF_SEM_SCREEN_800PX && sem_record_top_lcd_button.selected)
				{
					if(sem_record_request)
						sem_stop_record_request = true;
					else
					{
						sem_selected_recording_mode = DEF_SEM_RECORD_TOP;
						sem_record_request = true;
					}
				}
				else if (Util_hid_is_pressed(key, sem_record_bottom_lcd_button) && var_screen_mode != DEF_SEM_SCREEN_800PX)
					sem_record_bottom_lcd_button.selected = true;
				else if (Util_hid_is_released(key, sem_record_bottom_lcd_button) && var_screen_mode != DEF_SEM_SCREEN_800PX && sem_record_bottom_lcd_button.selected)
				{
					if(sem_record_request)
						sem_stop_record_request = true;
					else
					{
						sem_selected_recording_mode = DEF_SEM_RECORD_BOTTOM;
						sem_record_request = true;
					}
				}
			}
#endif
		}

		//Scroll bar
		if(sem_scroll_mode)
		{
			if (key.h_c_down || key.h_c_up)
				sem_y_offset += (double)key.cpad_y * var_scroll_speed * 0.0625;

			if (key.h_touch && sem_scroll_bar.selected)
				sem_y_offset = ((key.touch_y - 15.0) / 195.0) * sem_y_max;

			if (Util_hid_is_pressed(key, sem_scroll_bar))
				sem_scroll_bar.selected = true;

			if(sem_touch_y_move_left * var_scroll_speed != 0)
				sem_y_offset -= sem_touch_y_move_left * var_scroll_speed;
		}

		if (sem_y_offset >= 0)
			sem_y_offset = 0.0;
		else if (sem_y_offset <= sem_y_max)
			sem_y_offset = sem_y_max;

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

			sem_back_button.selected = sem_scroll_bar.selected = sem_english_button.selected = sem_japanese_button.selected
			= sem_hungarian_button.selected = sem_chinese_button.selected = sem_italian_button.selected = sem_spanish_button.selected
			= sem_romanian_button.selected = sem_polish_button.selected = sem_ryukyuan_button.selected = sem_night_mode_on_button.selected
			= sem_night_mode_off_button.selected = sem_flash_mode_button.selected = sem_screen_brightness_bar.selected = sem_screen_off_time_bar.selected
			= sem_sleep_time_bar.selected = sem_800px_mode_button.selected = sem_3d_mode_button.selected = sem_400px_mode_button.selected
			= sem_auto_mode_button.selected = sem_scroll_speed_bar.selected = sem_wifi_on_button.selected = sem_wifi_off_button.selected
			= sem_allow_send_info_button.selected = sem_deny_send_info_button.selected = sem_debug_mode_on_button.selected = sem_debug_mode_off_button.selected
			= sem_eco_mode_on_button.selected = sem_eco_mode_off_button.selected = sem_record_both_lcd_button.selected = sem_record_top_lcd_button.selected
			= sem_record_bottom_lcd_button.selected = sem_load_all_ex_font_button.selected = sem_unload_all_ex_font_button.selected 
			= sem_use_fake_model_button.selected = sem_dump_log_button.selected = false;

#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)
			sem_check_update_button.selected = sem_close_updater_button.selected = sem_select_edtion_button.selected
			= sem_3dsx_button.selected = sem_cia_button.selected = sem_back_to_patch_note_button.selected
			= sem_dl_install_button.selected = sem_close_app_button.selected = false;
#endif

#if DEF_ENABLE_CPU_MONITOR_API
			sem_monitor_cpu_usage_on_button.selected = sem_monitor_cpu_usage_off_button.selected = false;
#endif

			Draw_get_bot_ui_button()->selected = false;

			for (int i = 0; i < 9; i++)
				sem_menu_button[i].selected = false;

			for (int i = 0; i < DEF_EXFONT_NUM_OF_FONT_NAME; i++)
				sem_ex_font_button[i].selected = false;

			sem_touch_x_move_left -= (sem_touch_x_move_left * 0.025);
			sem_touch_y_move_left -= (sem_touch_y_move_left * 0.025);
			if (sem_touch_x_move_left < 0.5 && sem_touch_x_move_left > -0.5)
				sem_touch_x_move_left = 0;
			if (sem_touch_y_move_left < 0.5 && sem_touch_y_move_left > -0.5)
				sem_touch_y_move_left = 0;
		}
	}

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

#if (DEF_ENABLE_VIDEO_AUDIO_ENCODER_API && DEF_ENABLE_SW_CONVERTER_API && DEF_SEM_ENABLE_SCREEN_RECORDER)

void Sem_encode_thread(void* arg)
{
	Util_log_save(DEF_SEM_ENCODE_THREAD_STR, "Thread started.");
	u8* yuv420p = NULL;
	Result_with_string result;

	while (sem_thread_run)
	{
		while(sem_record_request)
		{
			if(sem_encode_request)
			{
				sem_wait_request = true;
				sem_encode_request = false;
				yuv420p = (u8*)Util_safe_linear_alloc(sem_rec_width * sem_rec_height * 1.5);
				if(yuv420p == NULL)
					sem_stop_record_request = true;
				else
				{
					memcpy(yuv420p, sem_yuv420p, sem_rec_width * sem_rec_height * 1.5);

					//int log_num = Util_log_save("", "");
					result = Util_video_encoder_encode(yuv420p, 0);
					//Util_log_add(log_num, "");
					if(result.code != 0)
					{
						Util_log_save(DEF_SEM_ENCODE_THREAD_STR, "Util_video_encoder_encode()..." + result.string + result.error_description, result.code);
						break;
					}
				}

				Util_safe_linear_free(yuv420p);
				yuv420p = NULL;
				sem_wait_request = false;
			}
			else
				Util_sleep(1000);
		}

		Util_sleep(DEF_ACTIVE_THREAD_SLEEP_TIME);
	}

	Util_log_save(DEF_SEM_ENCODE_THREAD_STR, "Thread exit.");
	threadExit(0);
}

void Sem_record_thread(void* arg)
{
	Util_log_save(DEF_SEM_RECORD_THREAD_STR, "Thread started.");
	bool new_3ds = false;
	int new_width = 0;
	int new_height = 0;
	int bot_bgr_offset = 0;
	int offset = 0;
	int mode = 0;
	int rec_width = 400;
	int rec_height = 480;
	int rec_framerate = 10;
	int log_num = 0;
	u8* top_framebuffer = NULL;
	u8* bot_framebuffer = NULL;
	u8* top_bgr = NULL;
	u8* bot_bgr = NULL;
	u8* both_bgr = NULL;
	u16 width = 0;
	u16 height = 0;
	double time = 0;
	Result_with_string result;
	TickCounter counter;
	APT_CheckNew3DS(&new_3ds);
	osTickCounterStart(&counter);

	while (sem_thread_run)
	{
		if (sem_record_request)
		{
			if(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DS || var_model == CFG_MODEL_N3DSXL)
				APT_SetAppCpuTimeLimit(80);
			else
				APT_SetAppCpuTimeLimit(70);

			mode = sem_selected_recording_mode;
			if(mode == DEF_SEM_RECORD_BOTH)
			{
				rec_width = 400;
				rec_height = 480;
				if(new_3ds)
					rec_framerate = 15;
				else
					rec_framerate = 5;
			}
			else if(mode == DEF_SEM_RECORD_TOP)
			{
				rec_width = 400;
				rec_height = 240;
				if(new_3ds)
					rec_framerate = 30;
				else
					rec_framerate = 10;
			}
			else if(mode == DEF_SEM_RECORD_BOTTOM)
			{
				rec_width = 320;
				rec_height = 240;
				if(new_3ds)
					rec_framerate = 30;
				else
					rec_framerate = 10;
			}
			sem_rec_width = rec_width;
			sem_rec_height = rec_height;

			log_num = Util_log_save(DEF_SEM_RECORD_THREAD_STR, "Util_encoder_create_output_file()...");
			result = Util_encoder_create_output_file(DEF_MAIN_DIR + "screen_recording/" + std::to_string(var_years) + "_" + std::to_string(var_months) 
			+ "_" + std::to_string(var_days) + "_" + std::to_string(var_hours) + "_" + std::to_string(var_minutes) + "_" + std::to_string(var_seconds) + ".mp4", 0);
			Util_log_add(log_num, result.string + result.error_description, result.code);
			if(result.code != 0)
				sem_record_request = false;

			log_num = Util_log_save(DEF_SEM_RECORD_THREAD_STR, "Util_video_encoder_init()...");
			result = Util_video_encoder_init(VIDEO_CODEC_MJPEG, rec_width, rec_height, 1500000, rec_framerate, 0);
			Util_log_add(log_num, result.string + result.error_description, result.code);
			if(result.code != 0)
				sem_record_request = false;

			log_num = Util_log_save(DEF_SEM_RECORD_THREAD_STR, "Util_encoder_write_header()...");
			result = Util_encoder_write_header(0);
			Util_log_add(log_num, result.string + result.error_description, result.code);
			if(result.code != 0)
				sem_record_request = false;

			sem_yuv420p = (u8*)Util_safe_linear_alloc(rec_width * rec_height * 1.5);
			if(sem_yuv420p == NULL)
				sem_stop_record_request = true;

			while(sem_record_request)
			{
				Color_converter_parameters parameters;

				if(sem_stop_record_request)
					break;

				osTickCounterUpdate(&counter);
				
				if(mode == DEF_SEM_RECORD_BOTH)
				{
					top_framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &width, &height);
					result = Util_converter_rgb888_rotate_90_degree(top_framebuffer, &top_bgr, width, height, &new_width, &new_height);
					if(result.code != 0)
					{
						Util_log_save(DEF_SEM_RECORD_THREAD_STR, "Util_converter_rgb888_rotate_90_degree()..." + result.string + result.error_description, result.code);
						break;
					}

					bot_framebuffer = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &width, &height);
					result = Util_converter_rgb888_rotate_90_degree(bot_framebuffer, &bot_bgr, width, height, &new_width, &new_height);
					if(result.code != 0)
					{
						Util_log_save(DEF_SEM_RECORD_THREAD_STR, "Util_converter_rgb888_rotate_90_degree()..." + result.string + result.error_description, result.code);
						break;
					}

					both_bgr = (u8*)Util_safe_linear_alloc(rec_width * rec_height * 3);
					if(both_bgr == NULL)
						break;

					memcpy(both_bgr, top_bgr, 400 * 240 * 3);
					Util_safe_linear_free(top_bgr);
					top_bgr = NULL;

					offset = 400 * 240 * 3;
					bot_bgr_offset = 0;

					for(int i = 0; i < 240; i++)
					{
						memset(both_bgr + offset, 0x0, 40 * 3);
						offset += 40 * 3;
						memcpy(both_bgr + offset, bot_bgr + bot_bgr_offset, 320 * 3);
						offset += 320 * 3;
						bot_bgr_offset += 320 * 3;
						memset(both_bgr + offset, 0x0, 40 * 3);
						offset += 40 * 3;
					}
					Util_safe_linear_free(bot_bgr);
					bot_bgr = NULL;
				}
				else if(mode == DEF_SEM_RECORD_TOP)
				{
					top_framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &width, &height);
					result = Util_converter_rgb888_rotate_90_degree(top_framebuffer, &both_bgr, width, height, &new_width, &new_height);
					if(result.code != 0)
					{
						Util_log_save(DEF_SEM_RECORD_THREAD_STR, "Util_converter_rgb888_rotate_90_degree()..." + result.string + result.error_description, result.code);
						break;
					}
				}
				else if(mode == DEF_SEM_RECORD_BOTTOM)
				{
					bot_framebuffer = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &width, &height);
					result = Util_converter_rgb888_rotate_90_degree(bot_framebuffer, &both_bgr, width, height, &new_width, &new_height);
					if(result.code != 0)
					{
						Util_log_save(DEF_SEM_RECORD_THREAD_STR, "Util_converter_rgb888_rotate_90_degree()..." + result.string + result.error_description, result.code);
						break;
					}
				}
				
				parameters.converted = NULL;
				parameters.in_color_format = PIXEL_FORMAT_BGR888;
				parameters.in_height = rec_height;
				parameters.in_width = rec_width;
				parameters.out_color_format = PIXEL_FORMAT_YUV420P;
				parameters.out_height = rec_height;
				parameters.out_width = rec_width;
				parameters.source = both_bgr;

				Util_converter_convert_color(&parameters);
				Util_safe_linear_free(both_bgr);
				both_bgr = NULL;
				if(result.code != 0)
				{
					Util_log_save(DEF_SEM_RECORD_THREAD_STR, "Util_converter_rgb888_to_yuv420p()..." + result.string + result.error_description, result.code);
					break;
				}
				memcpy(sem_yuv420p, parameters.converted, rec_width * rec_height * 1.5);
				Util_safe_linear_free(parameters.converted);
				parameters.converted = NULL;

				sem_encode_request = true;
				osTickCounterUpdate(&counter);
				time = osTickCounterRead(&counter);
				if(1000.0 / rec_framerate > time)
					Util_sleep(((1000.0 / rec_framerate) - time) * 1000);
			}

			while(sem_wait_request)
				Util_sleep(100000);

			Util_encoder_close_output_file(0);
			Util_safe_linear_free(both_bgr);
			Util_safe_linear_free(bot_bgr);
			Util_safe_linear_free(top_bgr);
			Util_safe_linear_free(sem_yuv420p);
			both_bgr = NULL;
			bot_bgr = NULL;
			top_bgr = NULL;
			sem_yuv420p = NULL;
			sem_record_request = false;
			sem_stop_record_request = false;
			var_need_reflesh = true;
			APT_SetAppCpuTimeLimit(30);
		}
		else
			Util_sleep(DEF_ACTIVE_THREAD_SLEEP_TIME);
	}

	Util_log_save(DEF_SEM_RECORD_THREAD_STR, "Thread exit.");
	threadExit(0);
}

#endif

void Sem_worker_callback(void)
{
	Result_with_string result;

	if (sem_already_init)
	{
		if (sem_reload_msg_request)
		{
			result = Sem_load_msg(var_lang);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sem_load_msg()..." + result.string + result.error_description, result.code);
			if (result.code != 0)
			{
				result = Sem_load_msg("en");
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sem_load_msg()..." + result.string + result.error_description, result.code);
			}

			result = Menu_load_msg(var_lang);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Menu_load_msg()..." + result.string + result.error_description, result.code);
			if (result.code != 0)
			{
				result = Menu_load_msg("en");
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Menu_load_msg()..." + result.string + result.error_description, result.code);
			}
			
			#ifdef DEF_ENABLE_BMARK
			result = Bmark_load_msg(var_lang);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Bmark_load_msg()..." + result.string + result.error_description, result.code);
			if (result.code != 0)
			{
				result = Bmark_load_msg("en");
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Bmark_load_msg()..." + result.string + result.error_description, result.code);
			}
			#endif

			#ifdef DEF_ENABLE_BMR
			result = Bmr_load_msg(var_lang);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Bmr_load_msg()..." + result.string + result.error_description, result.code);
			if (result.code != 0)
			{
				result = Bmr_load_msg("en");
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Bmr_load_msg()..." + result.string + result.error_description, result.code);
			}
			#endif

			#ifdef DEF_ENABLE_SUB_APP2
			result = Sapp2_load_msg(var_lang);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp2_load_msg()..." + result.string + result.error_description, result.code);
			if (result.code != 0)
			{
				result = Sapp2_load_msg("en");
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp2_load_msg()..." + result.string + result.error_description, result.code);
			}
			#endif

			#ifdef DEF_ENABLE_SUB_APP3
			result = Sapp3_load_msg(var_lang);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp3_load_msg()..." + result.string + result.error_description, result.code);
			if (result.code != 0)
			{
				result = Sapp3_load_msg("en");
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp4_load_msg()..." + result.string + result.error_description, result.code);
			}
			#endif

			#ifdef DEF_ENABLE_SUB_APP4
			result = Sapp4_load_msg(var_lang);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp4_load_msg()..." + result.string + result.error_description, result.code);
			if (result.code != 0)
			{
				result = Sapp4_load_msg("en");
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp4_load_msg()..." + result.string + result.error_description, result.code);
			}
			#endif

			#ifdef DEF_ENABLE_SUB_APP5
			result = Sapp5_load_msg(var_lang);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp5_load_msg()..." + result.string + result.error_description, result.code);
			if (result.code != 0)
			{
				result = Sapp5_load_msg("en");
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp5_load_msg()..." + result.string + result.error_description, result.code);
			}
			#endif

			#ifdef DEF_ENABLE_SUB_APP6
			result = Sapp6_load_msg(var_lang);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp6_load_msg()..." + result.string + result.error_description, result.code);
			if (result.code != 0)
			{
				result = Sapp6_load_msg("en");
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp6_load_msg()..." + result.string + result.error_description, result.code);
			}
			#endif

			#ifdef DEF_ENABLE_SUB_APP7
			result = Sapp7_load_msg(var_lang);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp7_load_msg()..." + result.string + result.error_description, result.code);
			if (result.code != 0)
			{
				result = Sapp7_load_msg("en");
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Sapp7_load_msg()..." + result.string + result.error_description, result.code);
			}
			#endif

			sem_reload_msg_request = false;
			var_need_reflesh = true;
		}
		else if(sem_change_brightness_request)
		{
			result = Util_cset_set_screen_brightness(true, true, var_lcd_brightness);
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Util_cset_set_screen_brightness()..." + result.string + result.error_description, result.code);
			sem_change_brightness_request = false;
		}
#if DEF_ENABLE_CPU_MONITOR_API
		else if(sem_is_cpu_usage_monitor_running != var_monitor_cpu_usage)
		{
			if(var_monitor_cpu_usage)
			{
				result = Util_cpu_usage_monitor_init();
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Util_cpu_usage_monitor_init()..." + result.string + result.error_description, result.code);
				if(result.code == 0)
					sem_is_cpu_usage_monitor_running = true;
				else
				{
					Util_err_set_error_message(result.string, result.error_description, DEF_SEM_WORKER_CALLBACK_STR, result.code);
					Util_err_set_error_show_flag(true);
					var_monitor_cpu_usage = false;
				}
			}
			else
			{
				Util_cpu_usage_monitor_exit();
				sem_is_cpu_usage_monitor_running = false;
			}
		}
#endif
		else if(sem_dump_log_request)
		{
			char file_name[64];
			sprintf(file_name, "%04d_%02d_%02d_%02d_%02d_%02d.txt", var_years, var_months, var_days, var_hours, var_minutes, var_seconds);

			result = Util_log_dump(file_name, DEF_MAIN_DIR + "logs/");
			Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Util_log_dump()..." + result.string + result.error_description, result.code);
			if(result.code == 0)
				Util_log_save(DEF_SEM_WORKER_CALLBACK_STR, "Log file was dumped at : " + DEF_MAIN_DIR + "logs/" + file_name);

			sem_dump_log_request = false;
		}
	}
}

#if ((DEF_ENABLE_CURL_API || DEF_ENABLE_HTTPC_API) && DEF_SEM_ENABLE_UPDATER)
void Sem_update_thread(void* arg)
{
	Util_log_save(DEF_SEM_UPDATE_THREAD_STR, "Thread started.");

	u8* buffer = NULL;
	u32 write_size = 0;
	u32 read_size = 0;
	u64 offset = 0;
	int log_num = 0;
	size_t parse_start_pos = std::string::npos;
	size_t parse_end_pos = std::string::npos;
	std::string dir_path = "";
	std::string file_name = "";
	std::string url = "";
	std::string last_url = "";
	std::string parse_cache = "";
	std::string parse_start[6] = {"<newest>", "<3dsx_available>", "<cia_available>", "<3dsx_url>", "<cia_url>", "<patch_note>", };
	std::string parse_end[6] = { "</newest>", "</3dsx_available>", "</cia_available>", "</3dsx_url>", "</cia_url>", "</patch_note>", };
	Handle am_handle = 0;
	Result_with_string result;

	while (sem_thread_run)
	{
		if (sem_check_update_request || sem_dl_file_request)
		{
			if (sem_check_update_request)
			{
				sem_update_progress = 0;
				sem_selected_edition_num = DEF_SEM_EDTION_NONE;
				url = DEF_CHECK_UPDATE_URL;
				sem_new_version_available = false;
				for (int i = 0; i < 6; i++)
					sem_newest_ver_data[i] = "";
			}
			else if (sem_dl_file_request)
			{
				sem_update_progress = 2;
				url = sem_newest_ver_data[3 + sem_selected_edition_num];
			}
			var_need_reflesh = true;

			sem_dled_size = 0;
			offset = 0;
			sem_installed_size = 0;
			sem_total_cia_size = 0;

			if(sem_dl_file_request)
			{
				dir_path = DEF_UPDATE_DIR_PREFIX + sem_newest_ver_data[0] + "/";
				file_name = DEF_UPDATE_FILE_PREFIX;
				if(sem_selected_edition_num == DEF_SEM_EDTION_3DSX)
					file_name += ".3dsx";
				else if(sem_selected_edition_num == DEF_SEM_EDTION_CIA)
					file_name += ".cia";

				Util_file_delete_file(file_name, dir_path);//delete old file if exist
			}

#if DEF_ENABLE_CURL_API
			if(sem_dl_file_request)
			{
				log_num = Util_log_save(DEF_SEM_UPDATE_THREAD_STR, "Util_curl_save_data()...");
				result = Util_curl_save_data(url, 0x20000, &sem_dled_size, true, 5, dir_path, file_name);
			}
			else
			{
				log_num = Util_log_save(DEF_SEM_UPDATE_THREAD_STR, "Util_curl_dl_data()...");
				result = Util_curl_dl_data(url, &buffer, 0x20000, &sem_dled_size, true, 5);
			}
#else
			if(sem_dl_file_request)
			{
				log_num = Util_log_save(DEF_SEM_UPDATE_THREAD_STR, "Util_httpc_save_data()...");
				result = Util_httpc_save_data(url, 0x20000, &sem_dled_size, true, 5, dir_path, file_name);
			}
			else
			{
				log_num = Util_log_save(DEF_SEM_UPDATE_THREAD_STR, "Util_httpc_dl_data()...");
				result = Util_httpc_dl_data(url, &buffer, 0x20000, &sem_dled_size, true, 5);
			}
#endif

			Util_log_add(log_num, result.string, result.code);

			if (result.code != 0)
			{
				Util_err_set_error_message(result.string, result.error_description, DEF_SEM_UPDATE_THREAD_STR, result.code);
				Util_err_set_error_show_flag(true);
				if (sem_check_update_request)
					sem_update_progress = -1;
				else if (sem_dl_file_request)
					sem_update_progress = -2;
				var_need_reflesh = true;
			}
			else
			{
				if (sem_check_update_request)
				{
					parse_cache = (char*)buffer;

					for (int i = 0; i < 6; i++)
					{
						parse_start_pos = parse_cache.find(parse_start[i]);
						parse_end_pos = parse_cache.find(parse_end[i]);

						parse_start_pos += parse_start[i].length();
						parse_end_pos -= parse_start_pos;
						if (parse_start_pos != std::string::npos && parse_end_pos != std::string::npos)
							sem_newest_ver_data[i] = parse_cache.substr(parse_start_pos, parse_end_pos);
						else
						{
							sem_update_progress = -1;
							break;
						}
					}

					if(sem_update_progress != -1)
					{
						if (DEF_CURRENT_APP_VER_INT < atoi(sem_newest_ver_data[0].c_str()))
							sem_new_version_available = true;
						else
							sem_new_version_available = false;
						
						if(envIsHomebrew() && sem_newest_ver_data[1] == "1")
							sem_selected_edition_num = DEF_SEM_EDTION_3DSX;
						else if(sem_newest_ver_data[2] == "1")
							sem_selected_edition_num = DEF_SEM_EDTION_CIA;
					}

					sem_update_progress = 1;
					var_need_reflesh = true;
				}
				else if (sem_dl_file_request)
				{
					sem_update_progress = 3;
					if (sem_selected_edition_num == DEF_SEM_EDTION_3DSX)
						sem_update_progress = 4;

					var_need_reflesh = true;
					if (sem_selected_edition_num == DEF_SEM_EDTION_CIA)
					{
						sem_total_cia_size = sem_dled_size;
						log_num = Util_log_save(DEF_SEM_UPDATE_THREAD_STR, "AM_StartCiaInstall()...");
						result.code = AM_StartCiaInstall(MEDIATYPE_SD, &am_handle);
						Util_log_add(log_num, "", result.code);

						while (true)
						{
							Util_safe_linear_free(buffer);
							buffer = NULL;
							log_num = Util_log_save(DEF_SEM_UPDATE_THREAD_STR, "Util_file_load_from_file_with_range()...");
							result = Util_file_load_from_file_with_range(file_name, dir_path, &buffer, 0x20000, offset, &read_size);
							Util_log_add(log_num, result.string, result.code);
							if(result.code != 0 || read_size <= 0)
								break;

							log_num = Util_log_save(DEF_SEM_UPDATE_THREAD_STR, "FSFILE_Write()...");
							result.code = FSFILE_Write(am_handle, &write_size, offset, buffer, read_size, FS_WRITE_FLUSH);
							Util_log_add(log_num, "", result.code);
							if(result.code != 0)
								break;

							offset += write_size;
							sem_installed_size += write_size;
						}

						log_num = Util_log_save(DEF_SEM_UPDATE_THREAD_STR, "AM_FinishCiaInstall()...");
						result.code = AM_FinishCiaInstall(am_handle);
						Util_log_add(log_num, "", result.code);
						if (result.code == 0)
							sem_update_progress = 4;
						else
							sem_update_progress = -2;
						var_need_reflesh = true;
					}
				}
			}

			Util_safe_linear_free(buffer);
			buffer = NULL;
			if(sem_check_update_request)
				sem_check_update_request = false;
			else if(sem_dl_file_request)
				sem_dl_file_request = false;
		}
		else
			Util_sleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (sem_thread_suspend)
			Util_sleep(DEF_INACTIVE_THREAD_SLEEP_TIME);
	}
	Util_log_save(DEF_SEM_UPDATE_THREAD_STR, "Thread exit.");
	threadExit(0);
}
#endif
