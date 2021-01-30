#include <3ds.h>
#include <string>
#include <unistd.h>

#include "headers.hpp"

#include "menu.hpp"

#include "setting_menu.hpp"
#include "battery_mark.hpp"
#include "battery_mark_ranking.hpp"

/*For draw*/
bool menu_need_reflesh = true;
/*---------------------------------------------*/

bool menu_check_connectivity_thread_run = false;
bool menu_update_thread_run = false;
bool menu_change_brightness_request = false;
bool menu_connect_test_succes = false;
bool menu_mcu_success = false;
bool menu_main_run = true;
bool menu_must_exit = false;
u8* menu_wifi_state;
u8* menu_wifi_state_internet_sample;
u8 menu_wifi_signal = -1;
u8 menu_battery_charge = -1;
int menu_cam_fps = 0;
int menu_hours = -1;
int menu_minutes = -1;
int menu_seconds = -1;
int menu_days = -1;
int menu_months = -1;
int menu_years = -1;
int menu_fps = 0;
int menu_free_ram = 0;
int menu_free_linear_ram = 0;
int menu_afk_time;
int menu_battery_level_raw = 0;
double menu_battery_voltage = 0;
char menu_status[128];
char menu_status_short[64];
std::string menu_clipboard = "";
std::string menu_battery_level = "?";
std::string menu_update_thread_string = "Menu/Update thread";
std::string menu_send_app_info_thread_string = "Menu/Send app info thread";
std::string menu_check_connection_thread_string = "Menu/Check connection thread";
std::string menu_init_string = "Menu/Init";
std::string menu_exit_string = "Menu/Exit";
std::string menu_app_ver = "v2.0.0";
Thread menu_update_thread, menu_send_app_info_thread, menu_check_connectivity_thread;


bool Menu_query_running_flag(void)
{
	return menu_main_run;
}

int Menu_query_afk_time(void)
{
	return menu_afk_time;
}

bool Menu_query_battery_charge(void)
{
	return (menu_battery_charge == 1);
}

std::string Menu_query_battery_level(void)
{
	return menu_battery_level;
}

int Menu_query_battery_level_raw(void)
{
	return menu_battery_level_raw;
}

double Menu_query_battery_voltage(void)
{
	return menu_battery_voltage;
}

std::string Menu_query_clipboard(void)
{
	return menu_clipboard;
}

int Menu_query_free_ram(void)
{
	return menu_free_ram;
}

int Menu_query_free_linear_ram(void)
{
	return menu_free_linear_ram;
}

bool Menu_query_must_exit_flag(void)
{
	return menu_must_exit;
}

void Menu_set_must_exit_flag(bool flag)
{
	menu_must_exit = flag;
}

std::string Menu_query_status(bool only_system_state)
{
	if(only_system_state)
		return menu_status_short;
	else
		return menu_status;
}

std::string Menu_query_time(int mode)
{
	char time[128];
	std::string return_time;

	if(mode == 0)
		sprintf(time, "%04d_%02d_%02d_%02d_%02d_%02d", menu_years, menu_months, menu_days, menu_hours, menu_minutes, menu_seconds);
	else if(mode == 1)
		sprintf(time, "%04d_%02d_%02d", menu_years, menu_months, menu_days);
	else if(mode == 2)
		sprintf(time, "%02d_%02d_%02d", menu_hours, menu_minutes, menu_seconds);
	else if(mode == 3)
		sprintf(time, "%02d:%02d:%02d", menu_hours, menu_minutes, menu_seconds);

	return_time = time;
	return return_time;
}

std::string Menu_query_ver(void)
{
	return menu_app_ver;
}

int Menu_query_wifi_state(void)
{
	return (int)menu_wifi_signal;
}

void Menu_reset_afk_time(void)
{
	menu_afk_time = 0;
}

void Menu_set_clipboard(std::string data)
{
	menu_clipboard = data;
}

void Menu_set_operation_flag(int operation_num, bool flag)
{
	if (operation_num == MENU_CHANGE_BRIGHTNESS_REQUEST)
		menu_change_brightness_request = flag;
}

void Menu_resume(void)
{
	menu_main_run = true;
	menu_need_reflesh = true;
}

void Menu_suspend(void)
{
	menu_main_run = false;
}

void Menu_init(void)
{
	Log_log_save(menu_init_string, "Initializing...", 1234567890, FORCE_DEBUG);

	Draw_progress("[Menu] Starting threads...");
	menu_update_thread_run = true;
	menu_check_connectivity_thread_run = true;
	menu_update_thread = threadCreate(Menu_update_thread, (void*)(""), STACKSIZE, PRIORITY_REALTIME, 1, false);
	menu_check_connectivity_thread = threadCreate(Menu_check_connectivity_thread, (void*)(""), STACKSIZE, PRIORITY_NORMAL, 1, false);

	if (Sem_query_settings(SEM_ALLOW_SEND_APP_INFO))
	{
		for (int i = 1; i <= 1000; i++)
		{
			if (Sem_query_settings_i(SEM_NUM_OF_APP_START) == i * 10)
			{
				menu_send_app_info_thread = threadCreate(Menu_send_app_info_thread, (void*)(""), STACKSIZE, PRIORITY_LOW, 1, true);
				break;
			}
		}
	}

	menu_wifi_state = (u8*)malloc(0x1);
	menu_wifi_state_internet_sample = (u8*)malloc(0x1);
	memset(menu_wifi_state, 0xff, 0x1);
	memset(menu_wifi_state_internet_sample, 0x2, 0x1);
	Menu_get_system_info();

	Menu_resume();
	Log_log_save(menu_init_string, "Initialized", 1234567890, FORCE_DEBUG);
}


void Menu_exit(void)
{
	Log_log_save(menu_exit_string, "Exiting...", 1234567890, FORCE_DEBUG);
	u64 time_out = 10000000000;
	int log_num;
	Result_with_string result;

	menu_update_thread_run = false;
	menu_check_connectivity_thread_run = false;

	if (Bmark_query_init_flag())
		Bmark_exit();
	if (Bmr_query_init_flag())
		Bmr_exit();
	if (Sem_query_init_flag())
		Sem_exit();

	Draw_progress("[Menu] Exiting...");

	for (int i = 0; i < 2; i++)
	{
		log_num = Log_log_save(menu_exit_string, "threadJoin()" + std::to_string(i) + "/1...", 1234567890, FORCE_DEBUG);

		if(i == 0)
			result.code = threadJoin(menu_update_thread, time_out);
		else if(i == 1)
			result.code = threadJoin(menu_check_connectivity_thread, time_out);

		if (result.code == 0)
			Log_log_add(log_num, Err_query_template_summary(0), result.code, FORCE_DEBUG);
		else
			Log_log_add(log_num, Err_query_template_summary(-1024), result.code, FORCE_DEBUG);
	}

	threadFree(menu_update_thread);
	threadFree(menu_check_connectivity_thread);

	Log_log_save(menu_exit_string, "Exited.", 1234567890, FORCE_DEBUG);
}

void Menu_main(void)
{
	Hid_set_disable_flag(false);
	double x = 0, y = 0;
	float r, g, b, a;

	sprintf(menu_status_short, "%04d/%02d/%02d %02d:%02d:%02d ", menu_years, menu_months, menu_days, menu_hours, menu_minutes, menu_seconds);
	sprintf(menu_status, "%02dfps %06.1fms %s", menu_fps,  Draw_query_frametime(), menu_status_short);

	if (menu_main_run)
	{
		Hid_info key;
		Hid_query_key_state(&key);
		Log_main();
		if(Draw_query_need_reflesh() || !Sem_query_settings(SEM_ECO_MODE))
			menu_need_reflesh = true;

		Hid_key_flag_reset();

		if(menu_need_reflesh)
		{
			if (Sem_query_settings(SEM_NIGHT_MODE))
			{
				r = 1.0;
				g = 1.0;
				b = 1.0;
				a = 0.75;
			}
			else
			{
				r = 0.0;
				g = 0.0;
				b = 0.0;
				a = 1.0;
			}

			Draw_frame_ready();
			if (Sem_query_settings(SEM_NIGHT_MODE))
				Draw_screen_ready_to_draw(0, true, 2, 0.0, 0.0, 0.0);
			else
				Draw_screen_ready_to_draw(0, true, 2, 1.0, 1.0, 1.0);

			Draw_top_ui();

			if (Sem_query_settings(SEM_NIGHT_MODE))
				Draw_screen_ready_to_draw(1, true, 2, 0.0, 0.0, 0.0);
			else
				Draw_screen_ready_to_draw(1, true, 2, 1.0, 1.0, 1.0);

			Draw_texture(Square_image, weak_aqua_tint, 0, 0.0, 0.0, 60.0, 60.0);
			Draw("Battery\n mark", 10.0, 20.0, 0.4, 0.4, r, g, b, a);
			if(Bmark_query_init_flag())
			{
				Draw_texture(Square_image, weak_red_tint, 0, 45.0, 0.0, 15.0, 15.0);
				Draw("X", 47.5, 0.0, 0.5, 0.5, 1.0, 0.0, 0.0, 0.5);
			}
			
			Draw_texture(Square_image, weak_aqua_tint, 0, 80.0, 0.0, 60.0, 60.0);
			Draw("Battery\n mark\nranking", 90.0, 15.0, 0.4, 0.4, r, g, b, a);
			if(Bmr_query_init_flag())
			{
				Draw_texture(Square_image, weak_red_tint, 0, 125.0, 0.0, 15.0, 15.0);
				Draw("X", 127.5, 0.0, 0.5, 0.5, 1.0, 0.0, 0.0, 0.5);
			}

			Draw_texture(Square_image, weak_aqua_tint, 0, 260.0, 180.0, 60.0, 60.0);
			Draw("Settings", 270.0, 205.0, 0.4, 0.4, r, g, b, a);

			Draw_bot_ui();
			Draw_touch_pos();

			Draw_apply_draw();
			menu_need_reflesh = false;
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
			if (key.p_start || (key.p_touch && key.touch_x >= 0 && key.touch_x <= 320 && key.touch_y >= 220 && key.touch_y <= 240))
					menu_must_exit = Menu_check_exit();
			else if (key.p_select)
				Log_set_log_show_flag(!Log_query_log_show_flag());
			else if (key.p_touch && key.touch_x >= 45 && key.touch_x <= 59 && key.touch_y >= 0 && key.touch_y <= 14 && Bmark_query_init_flag())
			{
				Hid_set_disable_flag(true);
				Bmark_exit();
			}
			else if (key.p_touch && key.touch_x >= 0 && key.touch_x <= 59 && key.touch_y >= 0 && key.touch_y <= 59)
			{
				Hid_set_disable_flag(true);
				if (!Bmark_query_init_flag())
					Bmark_init();
				else
					Bmark_resume();
			}
			else if (key.p_touch && key.touch_x >= 125 && key.touch_x <= 139 && key.touch_y >= 0 && key.touch_y <= 14 && Bmr_query_init_flag())
			{
				Hid_set_disable_flag(true);
				Bmr_exit();
			}
			else if (key.p_touch && key.touch_x >= 80 && key.touch_x <= 139 && key.touch_y >= 0 && key.touch_y <= 59)
			{
				Hid_set_disable_flag(true);
				if (!Bmr_query_init_flag())
					Bmr_init();
				else
					Bmr_resume();
			}			
			else if (key.p_touch && key.touch_x >= 260 && key.touch_x <= 319 && key.touch_y >= 180 && key.touch_y <= 239)
			{
				Hid_set_disable_flag(true);
				if (!Sem_query_init_flag())
					Sem_init();
				else
					Sem_resume();
			}
		}
	}
	else if (Bmark_query_running_flag())
		Bmark_main();
	else if (Bmr_query_running_flag())
		Bmr_main();
	else if (Sem_query_running_flag())
		Sem_main();
	else
		menu_main_run = true;

	Hid_set_disable_flag(true);
}

bool Menu_check_exit(void)
{
	Hid_info key;
	while (true)
	{
		Draw_frame_ready();

		if (Sem_query_settings(SEM_NIGHT_MODE))
		{
			Draw_screen_ready_to_draw(0, true, 2, 0.0, 0.0, 0.0);
			Draw("Do you want to exit this software?", 90.0, 105.0, 0.5, 0.5, 1.0, 1.0, 1.0, 0.75);
		}
		else
		{
			Draw_screen_ready_to_draw(0, true, 2, 1.0, 1.0, 1.0);
			Draw("Do you want to exit this software?", 90.0, 105.0, 0.5, 0.5, 0.0, 0.0, 0.0, 1.0);
		}

		Draw("A to close", 130.0, 140.0, 0.5, 0.5, 0.0, 1.0, 0.0, 1.0);
		Draw("B to back", 210.0, 140.0, 0.5, 0.5, 1.0, 0.0, 0.0, 1.0);

		Draw_apply_draw();
		Hid_query_key_state(&key);
		Hid_key_flag_reset();
		if (key.p_a)
			return true;
		else if (key.p_b)
			return false;
	}
}

void Menu_get_system_info(void)
{
	u8 battery_level = -1;
	u8 battery_voltage = -1;
	Result_with_string result;

	PTMU_GetBatteryChargeState(&menu_battery_charge);//battery charge
	result.code = MCUHWC_GetBatteryLevel(&battery_level);//battery level(%)
	if(result.code == 0)
	{
		MCUHWC_GetBatteryVoltage(&battery_voltage);
		menu_battery_voltage = 5.0 * ((double)battery_voltage / 256); 
		menu_battery_level_raw = battery_level;
		menu_battery_level = std::to_string(battery_level);
	}
	else
	{
		PTMU_GetBatteryLevel(&battery_level);
		if ((int)battery_level == 0)
			menu_battery_level_raw = 0;
		else if ((int)battery_level == 1)
			menu_battery_level_raw = 5;
		else if ((int)battery_level == 2)
			menu_battery_level_raw = 10;
		else if ((int)battery_level == 3)
			menu_battery_level_raw = 30;
		else if ((int)battery_level == 4)
			menu_battery_level_raw = 60;
		else if ((int)battery_level == 5)
			menu_battery_level_raw = 100;

		menu_battery_level = "?";
	}

	menu_wifi_signal = osGetWifiStrength();
	//Get wifi state from shared memory #0x1FF81067
	memcpy((void*)menu_wifi_state, (void*)0x1FF81067, 0x1);
	if (memcmp(menu_wifi_state, menu_wifi_state_internet_sample, 0x1) == 0)
	{
		if (!menu_connect_test_succes)
			menu_wifi_signal = menu_wifi_signal + 4;
	}
	else
	{
		menu_wifi_signal = 8;
		menu_connect_test_succes = false;
	}

	//Get time
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t*)&unixTime);
	menu_years = timeStruct->tm_year + 1900;
	menu_months = timeStruct->tm_mon + 1;
	menu_days = timeStruct->tm_mday;
	menu_hours = timeStruct->tm_hour;
	menu_minutes = timeStruct->tm_min;
	menu_seconds = timeStruct->tm_sec;

	if (Sem_query_settings(SEM_DEBUG_MODE))
	{
		//check free RAM
		menu_free_ram = Menu_check_free_ram();
		menu_free_linear_ram = linearSpaceFree();
	}
}

int Menu_check_free_ram(void)
{
	u8* malloc_check[2000];
	int count;

	for (int i = 0; i < 2000; i++)
		malloc_check[i] = NULL;

	for (count = 0; count < 2000; count++)
	{
		malloc_check[count] = (u8*)malloc(0x186A0);// 100KB
		if (malloc_check[count] == NULL)
			break;
	}

	for (int i = 0; i <= count; i++)
		free(malloc_check[i]);

	return count;
}

void Menu_send_app_info_thread(void* arg)
{
	Log_log_save(menu_send_app_info_thread_string, "Thread started.", 1234567890, false);
	OS_VersionBin os_ver;
	bool is_new3ds = false;
	u8* dl_data;
	u32 status_code;
	u32 downloaded_size;
	char system_ver_char[0x50] = " ";
	std::string new3ds;
	dl_data = (u8*)malloc(0x10000);

	osGetSystemVersionDataString(&os_ver, &os_ver, system_ver_char, 0x50);
	std::string system_ver = system_ver_char;

	APT_CheckNew3DS(&is_new3ds);

	if (is_new3ds)
		new3ds = "yes";
	else
		new3ds = "no";

	std::string send_data = "{ \"app_ver\": \"" + menu_app_ver + "\",\"system_ver\" : \"" + system_ver + "\",\"start_num_of_app\" : \"" + std::to_string(Sem_query_settings_i(SEM_NUM_OF_APP_START)) + "\",\"language\" : \"" + Sem_query_lang() + "\",\"new3ds\" : \"" + new3ds + "\",\"time_to_enter_sleep\" : \"" + std::to_string(Sem_query_settings_i(SEM_TIME_TO_TURN_OFF_LCD)) + "\",\"scroll_speed\" : \"" + std::to_string(Sem_query_settings_d(SEM_SCROLL_SPEED)) + "\" }";
	Httpc_post_and_dl_data("https://script.google.com/macros/s/AKfycbyn_blFyKWXCgJr6NIF8x6ETs7CHRN5FXKYEAAIrzV6jPYcCkI/exec", (char*)send_data.c_str(), send_data.length(), dl_data, 0x10000, &downloaded_size, &status_code, true, MENU_HTTP_PORT0);
	free(dl_data);
	dl_data = NULL;

	Log_log_save(menu_send_app_info_thread_string, "Thread exit.", 1234567890, false);
	threadExit(0);
}

void Menu_check_connectivity_thread(void* arg)
{
	Log_log_save(menu_check_connection_thread_string, "Thread started.", 1234567890, false);
	u8* http_buffer;
	u32 response_code = 0;
	u32 dl_size = 0;
	int count = 100;
	std::string url = "https://connectivitycheck.gstatic.com/generate_204";
	std::string last_url;
	http_buffer = (u8*)malloc(0x1000);

	while (menu_check_connectivity_thread_run)
	{
		if (count >= 100 && !(Hid_query_disable_flag()))
		{
			count = 0;
			Httpc_dl_data(url, http_buffer, 0x1000, &dl_size, &response_code, false, &last_url, true, 0, MENU_HTTP_POST_PORT0);

			if (response_code == 204)
				menu_connect_test_succes = true;
			else
				menu_connect_test_succes = false;
		}
		else
			usleep(ACTIVE_THREAD_SLEEP_TIME);

		count++;
	}
	Log_log_save(menu_check_connection_thread_string, "Thread exit.", 1234567890, false);
	threadExit(0);
}

void Menu_update_thread(void* arg)
{
	Log_log_save(menu_update_thread_string, "Thread started.", 1234567890, false);
	int count = 0;
	Result_with_string result;

	while (menu_update_thread_run)
	{
		usleep(49000);
		count++;

		if (count >= 20)
		{
			menu_change_brightness_request = true;
			//fps

			menu_fps = Draw_query_fps();
			Menu_get_system_info();
			Draw_reset_fps();
			count = 0;
		}

		if (menu_change_brightness_request)
		{
			if (menu_afk_time > (Sem_query_settings_i(SEM_TIME_TO_TURN_OFF_LCD) + 100) * 2)
				Change_screen_state(true, true, false);
			else if (menu_afk_time >= Sem_query_settings_i(SEM_TIME_TO_TURN_OFF_LCD) * 2)
			{
				Change_screen_state(true, true, true);
				Change_brightness(true, true, Sem_query_settings_i(SEM_LCD_BRIGHTNESS_BEFORE_TURN_OFF));
			}
			else
			{
				Change_screen_state(true, true, true);
				Change_brightness(true, true, Sem_query_settings_i(SEM_LCD_BRIGHTNESS));
			}
			menu_change_brightness_request = false;
		}

		if (Sem_query_settings(SEM_FLASH_MODE))
		{
			if (Sem_query_settings(SEM_NIGHT_MODE))
				C2D_PlainImageTint(&texture_tint, C2D_Color32f(0.0, 0.0, 0.0, 1.0), true);
			else
				C2D_PlainImageTint(&texture_tint, C2D_Color32f(1.0, 1.0, 1.0, 0.75), true);

			Sem_set_settings(SEM_NIGHT_MODE, !Sem_query_settings(SEM_NIGHT_MODE));
		}
		menu_afk_time++;
	}
	Log_log_save(menu_update_thread_string, "Thread exit.", 1234567890, false);
	threadExit(0);
}
