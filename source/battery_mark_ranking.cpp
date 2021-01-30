#include <3ds.h>
#include <string>
#include <unistd.h>

#include "headers.hpp"

#include "battery_mark_ranking.hpp"

#include "menu.hpp"
#include "setting_menu.hpp"

bool bmr_need_reflesh = false;
bool bmr_main_run = false;
bool bmr_thread_run = false;
bool bmr_already_init = false;
bool bmr_thread_suspend = true;
bool bmr_dl_log_request = false;
int bmr_pre_dl_progress = 0;
int bmr_ranking[BMR_NUM_OF_LOGS];
int bmr_model_mode = 0;
double bmr_y_offset = 0;
double bmr_x_offset = 0;
double bmr_max_time[BMR_NUM_OF_LOGS];
double bmr_avg_time[BMR_NUM_OF_LOGS];
double bmr_min_time[BMR_NUM_OF_LOGS];
double bmr_total_time[BMR_NUM_OF_LOGS];
std::string bmr_date[BMR_NUM_OF_LOGS];
std::string bmr_app_ver[BMR_NUM_OF_LOGS];
std::string bmr_system_ver[BMR_NUM_OF_LOGS];
std::string bmr_model[BMR_NUM_OF_LOGS];
std::string bmr_user_name[BMR_NUM_OF_LOGS];
std::string bmr_msg[BMR_NUM_OF_MSG];
std::string bmr_ver = "v1.0.0";
std::string bmr_thread_string = "Bmr/Dl log thread";
std::string bmr_init_string = "Bmr/Init";
std::string bmr_exit_string = "Bmr/Exit";
Thread bmr_thread;

bool Bmr_query_init_flag(void)
{
	return bmr_already_init;
}

bool Bmr_query_running_flag(void)
{
	return bmr_main_run;
}

void Bmr_set_msg(int msg_num, std::string msg)
{
	if (msg_num >= 0 && msg_num <= BMR_NUM_OF_MSG)
		bmr_msg[msg_num] = msg;
}

Result_with_string Bmr_load_msg(std::string lang)
{
	u8* fs_buffer = NULL;
	u32 read_size;
	std::string setting_data[128];
	Result_with_string result;
	fs_buffer = (u8*)malloc(0x2000);

	result = File_load_from_rom("bmr_" + lang + ".txt", fs_buffer, 0x2000, &read_size, "romfs:/gfx/msg/");
	if (result.code != 0)
	{
		free(fs_buffer);
		return result;
	}

	result = Sem_parse_file((char*)fs_buffer, BMR_NUM_OF_MSG, setting_data);
	if (result.code != 0)
	{
		free(fs_buffer);
		return result;
	}

	for (int k = 0; k < BMR_NUM_OF_MSG; k++)
		Bmr_set_msg(k, setting_data[k]);

	free(fs_buffer);
	return result;
}

void Bmr_thread(void* arg)
{
	Log_log_save(bmr_thread_string, "Thread started.", 1234567890, false);
	int size = 0x100000;
	int log_num = 0;
	size_t cut_pos = 0;
	u8* data = (u8*)malloc(size);
	u32 dled_size = 0;
	u32 status_code = 0;
	std::string last_url = "";
	std::string string_data = "";
	std::string cache = "";
	Result_with_string result;
	if(data == NULL)
	{
		Err_set_error_message(Err_query_template_summary(OUT_OF_MEMORY), Err_query_template_detail(OUT_OF_MEMORY), bmr_thread_string, OUT_OF_MEMORY);
		Err_set_error_show_flag(true);
		bmr_thread_run = false;
	}

	while (bmr_thread_run)
	{
		if(bmr_dl_log_request)
		{
			//download ranking data from the ranking server
			memset(data, 0x0, size);
			cache = "";
			log_num = Log_log_save(bmr_thread_string, "Httpc_dl_data()...");
			result = Httpc_dl_data("https://script.google.com/macros/s/AKfycbxo4iwCbxtI2ZuYQP7bKveRdHx6kPTH4rZ8Pg8nUguIq_7zcoooEUszJQ/exec?mode=" + std::to_string(bmr_model_mode) + "&start_pos=1&logs=1000", data, size, &dled_size, &status_code, true, &last_url, false, 10, BMR_HTTP_PORT0);
			Log_log_add(log_num, result.string, result.code);
			string_data = (char*)data;

			cut_pos = string_data.find(",");
			if(!(cut_pos == std::string::npos))
			{
				cache = string_data.substr(0, cut_pos);
				string_data = string_data.substr(cut_pos + 1);
			}

			if(cache == "Success")
			{
				for(int i = 0; i < BMR_NUM_OF_LOGS; i++)
				{
					bmr_ranking[i] = 0;
					bmr_max_time[i] = 0;
					bmr_avg_time[i] = 0;
					bmr_min_time[i] = 0;
					bmr_total_time[i] = 0;
					bmr_date[i] = "";
					bmr_app_ver[i] = "";
					bmr_system_ver[i] = "";
					bmr_model[i] = "";
					bmr_user_name[i] = "";
				}
				//parse data
				for(int i = 0; i < BMR_NUM_OF_LOGS; i++)
				{
					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;
					
					if(string_data.substr(0, cut_pos) != "")
						bmr_ranking[i] = stoi(string_data.substr(0, cut_pos));
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;
					
					bmr_date[i] = string_data.substr(0, cut_pos);
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;
					
					bmr_app_ver[i] = string_data.substr(0, cut_pos);
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;
					
					bmr_system_ver[i] = string_data.substr(0, cut_pos);
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;
					
					bmr_model[i] = string_data.substr(0, cut_pos);
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;
					
					bmr_user_name[i] = string_data.substr(0, cut_pos);
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;
					
					if(string_data.substr(0, cut_pos) != "")
						bmr_max_time[i] = stod(string_data.substr(0, cut_pos));
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;
					
					if(string_data.substr(0, cut_pos) != "")
						bmr_avg_time[i] = stod(string_data.substr(0, cut_pos));
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;

					if(string_data.substr(0, cut_pos) != "")
						bmr_min_time[i] = stod(string_data.substr(0, cut_pos));
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find("\n");
					if(cut_pos == std::string::npos)
						break;
					
					if(string_data.substr(0, cut_pos) != "")
						bmr_total_time[i] = stod(string_data.substr(0, cut_pos));
					string_data = string_data.substr(cut_pos + 1);
				}
			}
			else if(result.code != 0)
			{
				Err_set_error_message(result.string, result.error_description, bmr_thread_string, result.code);
				Err_set_error_show_flag(true);
			}
			else
			{
				Err_set_error_message(Err_query_template_summary(GAS_RETURNED_NOT_SUCCESS), string_data, bmr_thread_string, GAS_RETURNED_NOT_SUCCESS);
				Err_set_error_show_flag(true);
			}
			bmr_need_reflesh = true;
			bmr_dl_log_request = false;
		}
		else
			usleep(ACTIVE_THREAD_SLEEP_TIME);

		while (bmr_thread_suspend)
			usleep(INACTIVE_THREAD_SLEEP_TIME);
	}

	free(data);
	data = NULL;
	Log_log_save(bmr_thread_string, "Thread exit.", 1234567890, false);
	threadExit(0);
}

void Bmr_resume(void)
{
	bmr_thread_suspend = false;
	bmr_main_run = true;
	bmr_need_reflesh = true;
	Menu_suspend();
}

void Bmr_suspend(void)
{
	bmr_thread_suspend = true;
	bmr_main_run = false;
	Menu_resume();
}

void Bmr_init(void)
{
	Log_log_save(bmr_init_string, "Initializing...", 1234567890, FORCE_DEBUG);

	Draw_progress("[Bmr] Starting threads...");
	bmr_thread_run = true;
	bmr_thread = threadCreate(Bmr_thread, (void*)(""), STACKSIZE, PRIORITY_NORMAL, 0, false);

	for(int i = 0; i < BMR_NUM_OF_LOGS; i++)
	{
		bmr_ranking[i] = 0;
		bmr_max_time[i] = 0;
		bmr_avg_time[i] = 0;
		bmr_min_time[i] = 0;
		bmr_total_time[i] = 0;
		bmr_date[i] = "";
		bmr_app_ver[i] = "";
		bmr_system_ver[i] = "";
		bmr_model[i] = "";
		bmr_user_name[i] = "";
	}
	Bmr_resume();
	bmr_already_init = true;
	Log_log_save(bmr_init_string, "Initialized", 1234567890, FORCE_DEBUG);
}

void Bmr_exit(void)
{
	Log_log_save(bmr_exit_string, "Exiting...", 1234567890, FORCE_DEBUG);
	u64 time_out = 10000000000;
	int log_num;
	Result_with_string result;

	bmr_already_init = false;
	bmr_thread_suspend = false;
	bmr_thread_run = false;
	
	Draw_progress("[Bmr] Exiting...");
	log_num = Log_log_save(bmr_exit_string, "threadJoin()0/0...", 1234567890, FORCE_DEBUG);
	result.code = threadJoin(bmr_thread, time_out);
	if (result.code == 0)
		Log_log_add(log_num, Err_query_template_summary(0), result.code, FORCE_DEBUG);
	else
		Log_log_add(log_num, Err_query_template_summary(-1024), result.code, FORCE_DEBUG);

	threadFree(bmr_thread);
}

void Bmr_main(void)
{
	float r, g, b, a;
	double x, y;
	Hid_info key;
	C2D_ImageTint color_tint;

	if (Sem_query_settings(SEM_NIGHT_MODE))
	{
		r = 1.0;
		g = 1.0;
		b = 1.0;
		a = 0.75;
		color_tint = weak_white_tint;
	}
	else
	{
		r = 0.0;
		g = 0.0;
		b = 0.0;
		a = 1.0;
		color_tint = weak_black_tint;
	}
	if(Httpc_query_dl_progress(BMR_HTTP_PORT0) != bmr_pre_dl_progress)
	{
		bmr_pre_dl_progress = Httpc_query_dl_progress(BMR_HTTP_PORT0);
		bmr_need_reflesh = true;
	}

	Hid_query_key_state(&key);
	Log_main();
	if(Draw_query_need_reflesh() || !Sem_query_settings(SEM_ECO_MODE))
		bmr_need_reflesh = true;

	Hid_key_flag_reset();

	if(bmr_need_reflesh)
	{
		Draw_frame_ready();
		if (Sem_query_settings(SEM_NIGHT_MODE))
			Draw_screen_ready_to_draw(0, true, 2, 0.0, 0.0, 0.0);
		else
			Draw_screen_ready_to_draw(0, true, 2, 1.0, 1.0, 1.0);

		if(bmr_dl_log_request)
			Draw_texture(Square_image, weak_red_tint, 0, 0, 15, 50 * Httpc_query_dl_progress(BMR_HTTP_PORT0), 5);

		Draw_texture(Square_image, color_tint, 0, 23 + bmr_x_offset, 20, 1, 240);
		Draw_texture(Square_image, color_tint, 0, 93 + bmr_x_offset, 20, 1, 240);
		Draw_texture(Square_image, color_tint, 0, 138 + bmr_x_offset, 20, 1, 240);
		Draw_texture(Square_image, color_tint, 0, 178 + bmr_x_offset, 20, 1, 240);
		Draw_texture(Square_image, color_tint, 0, 218 + bmr_x_offset, 20, 1, 240);
		Draw_texture(Square_image, color_tint, 0, 258 + bmr_x_offset, 20, 1, 240);
		Draw_texture(Square_image, color_tint, 0, 373 + bmr_x_offset, 20, 1, 240);
		Draw_texture(Square_image, color_tint, 0, 423 + bmr_x_offset, 20, 1, 240);
		Draw_texture(Square_image, color_tint, 0, 493 + bmr_x_offset, 20, 1, 240);
		Draw(bmr_msg[0], 25 + bmr_x_offset, 20, 0.4, 0.4, r, g, b, a);
		Draw(bmr_msg[1], 95 + bmr_x_offset, 20, 0.4, 0.4, r, g, b, a);
		Draw(bmr_msg[2], 140 + bmr_x_offset, 20, 0.4, 0.4, r, g, b, a);
		Draw(bmr_msg[3], 180 + bmr_x_offset, 20, 0.4, 0.4, r, g, b, a);
		Draw(bmr_msg[4], 220 + bmr_x_offset, 20, 0.4, 0.4, r, g, b, a);
		Draw(bmr_msg[5], 260 + bmr_x_offset, 20, 0.4, 0.4, r, g, b, a);
		Draw(bmr_msg[6], 375 + bmr_x_offset, 20, 0.4, 0.4, r, g, b, a);
		Draw(bmr_msg[7], 425 + bmr_x_offset, 20, 0.4, 0.4, r, g, b, a);
		Draw(bmr_msg[8], 495 + bmr_x_offset, 20, 0.4, 0.4, r, g, b, a);
		for(int i = 0; i < 22; i++)
		{
			Draw_texture(Square_image, color_tint, 0, 0, 31 + (i * 10), 600, 1);
			Draw(std::to_string(bmr_ranking[i + (int)bmr_y_offset]), 0 + bmr_x_offset, 30 + (i * 10), 0.4, 0.4, r, g, b, a);
			Draw(bmr_model[i + (int)bmr_y_offset], 25 + bmr_x_offset, 30 + (i * 10), 0.4, 0.4, r, g, b, a);
			Draw(Sem_convert_seconds_to_time(bmr_total_time[i + (int)bmr_y_offset]).substr(0, Sem_convert_seconds_to_time(bmr_total_time[i + (int)bmr_y_offset]).length() - 2), 95 + bmr_x_offset, 30 + (i * 10), 0.4, 0.4, r, g, b, a);
			Draw(Sem_convert_seconds_to_time(bmr_avg_time[i + (int)bmr_y_offset]).substr(0, Sem_convert_seconds_to_time(bmr_avg_time[i + (int)bmr_y_offset]).length() - 2), 140 + bmr_x_offset, 30 + (i * 10), 0.4, 0.4, r, g, b, a);
			Draw(Sem_convert_seconds_to_time(bmr_max_time[i + (int)bmr_y_offset]).substr(0, Sem_convert_seconds_to_time(bmr_max_time[i + (int)bmr_y_offset]).length() - 2), 180 + bmr_x_offset, 30 + (i * 10), 0.4, 0.4, r, g, b, a);
			Draw(Sem_convert_seconds_to_time(bmr_min_time[i + (int)bmr_y_offset]).substr(0, Sem_convert_seconds_to_time(bmr_min_time[i + (int)bmr_y_offset]).length() - 2), 220 + bmr_x_offset, 30 + (i * 10), 0.4, 0.4, r, g, b, a);
			Draw(bmr_user_name[i + (int)bmr_y_offset], 260 + bmr_x_offset, 30 + (i * 10), 0.4, 0.4, r, g, b, a);
			Draw(bmr_app_ver[i + (int)bmr_y_offset], 375 + bmr_x_offset, 30 + (i * 10), 0.4, 0.4, r, g, b, a);
			Draw(bmr_system_ver[i + (int)bmr_y_offset], 425 + bmr_x_offset, 30 + (i * 10), 0.4, 0.4, r, g, b, a);
			Draw(bmr_date[i + (int)bmr_y_offset], 495 + bmr_x_offset, 30 + (i * 10), 0.4, 0.4, r, g, b, a);
		}

		Draw_top_ui();

		if (Sem_query_settings(SEM_NIGHT_MODE))
			Draw_screen_ready_to_draw(1, true, 2, 0.0, 0.0, 0.0);
		else
			Draw_screen_ready_to_draw(1, true, 2, 1.0, 1.0, 1.0);

		Draw(bmr_ver, 0.0, 0.0, 0.4, 0.4, 0.0, 1.0, 0.0, 1.0);

		for(int i = 0; i < 7; i++)
		{
			Draw_texture(Square_image, weak_aqua_tint, 0, 40, 40 + (i * 20), 240, 20);
			if(bmr_model_mode == i)
				Draw(bmr_msg[9 + i], 42.5, 40 + (i * 20), 0.5, 0.5, 1, 0, 0, 1);
			else if(bmr_dl_log_request)
				Draw(bmr_msg[9 + i], 42.5, 40 + (i * 20), 0.5, 0.5, r, g, b, 0.25);
			else
				Draw(bmr_msg[9 + i], 42.5, 40 + (i * 20), 0.5, 0.5, r, g, b, a);
		}
		Draw_texture(Square_image, weak_aqua_tint, 0, 100, 190, 120, 20);
		Draw(bmr_msg[16], 102.5, 190, 0.5, 0.5, r, g, b, a);

		Draw_bot_ui();
		Draw_touch_pos();

		Draw_apply_draw();
		bmr_need_reflesh = false;
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
		if (key.p_start || (key.p_touch && key.touch_x >= 110 && key.touch_x <= 230 && key.touch_y >= 220 && key.touch_y <= 240))
			Bmr_suspend();
		else if(key.p_touch && key.touch_x >= 40 && key.touch_x <= 279 && key.touch_y >= 40 && key.touch_y <= 179 && !bmr_dl_log_request)
		{
			for(int i = 0; i < 7; i++)
			{
				if(key.touch_y >= 40 + (i * 20) && key.touch_y <= 59 + (i * 20))
					bmr_model_mode = i;
			}
			bmr_need_reflesh = true;
		}
		else if(key.p_b || (key.p_touch && key.touch_x >= 100 && key.touch_x <= 219 && key.touch_y >= 190 && key.touch_y <= 219))
		{
			bmr_dl_log_request = true;
			bmr_need_reflesh = true;
		}
		else if(key.h_c_left)
		{
			if(bmr_x_offset + (2.5 * key.count) > 0)
				bmr_x_offset = 0;
			else
				bmr_x_offset += (2.5 * key.count);
		
			bmr_need_reflesh = true;
		}
		else if(key.h_c_right)
		{
			if(bmr_x_offset - (2.5 * key.count) < -200)
				bmr_x_offset = -200;
			else
				bmr_x_offset -= (2.5 * key.count);

			bmr_need_reflesh = true;
		}
		else if(key.h_c_down)
		{
			if(bmr_y_offset + (0.5 * key.count) > 978)
				bmr_y_offset = 978;
			else
				bmr_y_offset += (0.5 * key.count);

			bmr_need_reflesh = true;
		}
		else if(key.h_c_up)
		{
			if(bmr_y_offset - (0.5 * key.count) < 0)
				bmr_y_offset = 0;
			else
				bmr_y_offset -= (0.5 * key.count);
		
			bmr_need_reflesh = true;
		}
	}
}
