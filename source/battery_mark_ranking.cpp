#include "system/headers.hpp"

bool bmr_main_run = false;
bool bmr_thread_run = false;
bool bmr_already_init = false;
bool bmr_thread_suspend = true;
bool bmr_dl_log_request = false;
int bmr_ranking[DEF_BMR_NUM_OF_LOGS];
int bmr_model_mode = 0;
double bmr_y_offset = 0;
double bmr_x_offset = 0;
double bmr_max_time[DEF_BMR_NUM_OF_LOGS];
double bmr_avg_time[DEF_BMR_NUM_OF_LOGS];
double bmr_min_time[DEF_BMR_NUM_OF_LOGS];
double bmr_total_time[DEF_BMR_NUM_OF_LOGS];
std::string bmr_date[DEF_BMR_NUM_OF_LOGS];
std::string bmr_app_ver[DEF_BMR_NUM_OF_LOGS];
std::string bmr_system_ver[DEF_BMR_NUM_OF_LOGS];
std::string bmr_model[DEF_BMR_NUM_OF_LOGS];
std::string bmr_user_name[DEF_BMR_NUM_OF_LOGS];
std::string bmr_msg[DEF_BMR_NUM_OF_MSG];
std::string bmr_status = "";
Thread bmr_init_thread, bmr_exit_thread, bmr_worker_thread;
Image_data bmr_model_selection_button[7], bmr_dl_ranking_button;

bool Bmr_query_init_flag(void)
{
	return bmr_already_init;
}

bool Bmr_query_running_flag(void)
{
	return bmr_main_run;
}

Result_with_string Bmr_load_msg(std::string lang)
{
	return Util_load_msg("bmr_" + lang + ".txt", bmr_msg, DEF_BMR_NUM_OF_MSG);
}

void Bmr_thread(void* arg)
{
	Util_log_save(DEF_BMR_WORKER_THREAD_STR, "Thread started.");
	int size = 0x100000;
	int log_num = 0;
	size_t cut_pos = 0;
	u8* data = NULL;
	u32 dled_size = 0;
	std::string last_url = "";
	std::string string_data = "";
	std::string cache = "";
	Result_with_string result;


	while (bmr_thread_run)
	{
		if(bmr_dl_log_request)
		{
			//download ranking data from the ranking server
			cache = "";
			log_num = Util_log_save(DEF_BMR_WORKER_THREAD_STR, "Util_httpc_dl_data()...");
			result = Util_httpc_dl_data("https://script.google.com/macros/s/AKfycbxo4iwCbxtI2ZuYQP7bKveRdHx6kPTH4rZ8Pg8nUguIq_7zcoooEUszJQ/exec?mode=" + std::to_string(bmr_model_mode) + "&start_pos=1&logs=1000", &data, size, &dled_size, true, 5);
			Util_log_add(log_num, result.string, result.code);
			string_data = (char*)data;
			Util_safe_linear_free(data);
			data = NULL;

			cut_pos = string_data.find(",");
			if(!(cut_pos == std::string::npos))
			{
				cache = string_data.substr(0, cut_pos);
				string_data = string_data.substr(cut_pos + 1);
			}

			if(cache == "Success")
			{
				for(int i = 0; i < DEF_BMR_NUM_OF_LOGS; i++)
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
				for(int i = 0; i < DEF_BMR_NUM_OF_LOGS; i++)
				{
					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;
					
					if(string_data.substr(0, cut_pos) != "")
						bmr_ranking[i] = atoi(string_data.substr(0, cut_pos).c_str());
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
						bmr_max_time[i] = atof(string_data.substr(0, cut_pos).c_str());
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;
					
					if(string_data.substr(0, cut_pos) != "")
						bmr_avg_time[i] = atof(string_data.substr(0, cut_pos).c_str());
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find(",");
					if(cut_pos == std::string::npos)
						break;

					if(string_data.substr(0, cut_pos) != "")
						bmr_min_time[i] = atof(string_data.substr(0, cut_pos).c_str());
					string_data = string_data.substr(cut_pos + 1);

					cut_pos = string_data.find("\n");
					if(cut_pos == std::string::npos)
						break;
					
					if(string_data.substr(0, cut_pos) != "")
						bmr_total_time[i] = atof(string_data.substr(0, cut_pos).c_str());
					string_data = string_data.substr(cut_pos + 1);
				}
			}
			else if(result.code != 0)
			{
				Util_err_set_error_message(result.string, result.error_description, DEF_BMR_WORKER_THREAD_STR, result.code);
				Util_err_set_error_show_flag(true);
			}
			else
			{
				Util_err_set_error_message(DEF_ERR_GAS_RETURNED_NOT_SUCCESS_STR, string_data, DEF_BMR_WORKER_THREAD_STR, DEF_ERR_GAS_RETURNED_NOT_SUCCESS);
				Util_err_set_error_show_flag(true);
			}

			bmr_dl_log_request = false;
		}
		else
			usleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (bmr_thread_suspend)
			usleep(DEF_INACTIVE_THREAD_SLEEP_TIME);
	}

	Util_log_save(DEF_BMR_WORKER_THREAD_STR, "Thread exit.");
	threadExit(0);
}

void Bmr_init_thread(void* arg)
{
	Util_log_save(DEF_BMR_INIT_STR, "Thread started.");
	Result_with_string result;
	
	bmr_status = "Starting threads...";
	bmr_thread_run = true;
	bmr_worker_thread = threadCreate(Bmr_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 0, false);

	bmr_status += "\nInitializing variables...";
	for(int i = 0; i < DEF_BMR_NUM_OF_LOGS; i++)
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

	bmr_dl_ranking_button.c2d = var_square_image[0];
	for(int i = 0; i < 7; i++)
		bmr_model_selection_button[i].c2d = var_square_image[0];

	Util_add_watch(&bmr_x_offset);
	Util_add_watch(&bmr_y_offset);
	Util_add_watch(&bmr_model_mode);
	Util_add_watch(&bmr_dl_log_request);
	Util_add_watch(&bmr_dl_ranking_button.selected);
	for(int i = 0; i < 7; i++)
		Util_add_watch(&bmr_model_selection_button[i].selected);

	bmr_already_init = true;

	Util_log_save(DEF_BMR_INIT_STR, "Thread exit.");
	threadExit(0);
}

void Bmr_exit_thread(void* arg)
{
	Util_log_save(DEF_BMR_EXIT_STR, "Thread started.");

	bmr_thread_suspend = false;
	bmr_thread_run = false;

	bmr_status = "Exiting threads...";
	Util_log_save(DEF_BMR_EXIT_STR, "threadJoin()...", threadJoin(bmr_init_thread, DEF_THREAD_WAIT_TIME));	

	bmr_status += ".";
	Util_log_save(DEF_BMR_EXIT_STR, "threadJoin()...", threadJoin(bmr_worker_thread, DEF_THREAD_WAIT_TIME));

	bmr_status += "\nCleaning up...";

	threadFree(bmr_init_thread);
	threadFree(bmr_worker_thread);

	Util_remove_watch(&bmr_x_offset);
	Util_remove_watch(&bmr_y_offset);
	Util_remove_watch(&bmr_model_mode);
	Util_remove_watch(&bmr_dl_log_request);
	Util_remove_watch(&bmr_dl_ranking_button.selected);
	for(int i = 0; i < 7; i++)
		Util_remove_watch(&bmr_model_selection_button[i].selected);

	bmr_already_init = false;

	Util_log_save(DEF_BMR_EXIT_STR, "Thread exit.");
	threadExit(0);
}

void Bmr_resume(void)
{
	bmr_thread_suspend = false;
	bmr_main_run = true;
	var_need_reflesh = true;
	Menu_suspend();
}

void Bmr_suspend(void)
{
	bmr_thread_suspend = true;
	bmr_main_run = false;
	Menu_resume();
}

void Bmr_hid(Hid_info key)
{
	if(aptShouldJumpToHome())
		return;

	if(Util_err_query_error_show_flag())
		Util_err_main(key);
	else
	{
		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if (key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected))
			Bmr_suspend();
		else if(Util_hid_is_pressed(key, bmr_dl_ranking_button) && !bmr_dl_log_request)
			bmr_dl_ranking_button.selected = true;
		else if((key.p_b || (Util_hid_is_released(key, bmr_dl_ranking_button) && bmr_dl_ranking_button.selected)) && !bmr_dl_log_request)
			bmr_dl_log_request = true;
		else if(key.h_c_left)
		{
			if(bmr_x_offset + 2.5 > 0)
				bmr_x_offset = 0;
			else
				bmr_x_offset += 2.5;
		}
		else if(key.h_c_right)
		{
			if(bmr_x_offset - 2.5 < -200)
				bmr_x_offset = -200;
			else
				bmr_x_offset -= 2.5;
		}
		else if(key.h_c_down)
		{
			if(bmr_y_offset + 0.5 > 978)
				bmr_y_offset = 978;
			else
				bmr_y_offset += 0.5;
		}
		else if(key.h_c_up)
		{
			if(bmr_y_offset - 0.5 < 0)
				bmr_y_offset = 0;
			else
				bmr_y_offset -= 0.5;
		}
		else
		{
			for(int i = 0; i < 7; i++)
			{
				if(Util_hid_is_pressed(key, bmr_model_selection_button[i]) && !bmr_dl_log_request)
					bmr_model_selection_button[i].selected = true;
				else if (Util_hid_is_released(key, bmr_model_selection_button[i]) && bmr_model_selection_button[i].selected && !bmr_dl_log_request)
					bmr_model_mode = i;
			}
		}
	}

	if(!key.p_touch && !key.h_touch)
	{
		Draw_get_bot_ui_button()->selected = false;
		bmr_dl_ranking_button.selected = false;
		for(int i = 0; i < 7; i++)
			bmr_model_selection_button[i].selected = false;
	}

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Bmr_init(void)
{
	Util_log_save(DEF_BMR_INIT_STR, "Initializing...");
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	Util_add_watch(&bmr_status);
	bmr_status = "";

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_3DSXL) && var_core_2_available)
		bmr_init_thread = threadCreate(Bmr_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		bmr_init_thread = threadCreate(Bmr_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!bmr_already_init)
	{
		if (var_night_mode)
		{
			color = DEF_DRAW_WHITE;
			back_color = DEF_DRAW_BLACK;
		}

		if(Util_is_watch_changed() || var_need_reflesh || !var_eco_mode)
		{
			var_need_reflesh = false;
			Draw_frame_ready();
			Draw_screen_ready(0, back_color);
			Draw_top_ui();
			Draw(bmr_status, 0, 20, 0.65, 0.65, color);

			Draw_apply_draw();
		}
		else
			gspWaitForVBlank();
	}

	if(!(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_3DSXL) || !var_core_2_available)
		APT_SetAppCpuTimeLimit(10);

	Bmr_resume();

	Util_log_save(DEF_BMR_INIT_STR, "Initialized.");
}

void Bmr_exit(void)
{
	Util_log_save(DEF_BMR_EXIT_STR, "Exiting...");

	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	bmr_status = "";

	bmr_exit_thread = threadCreate(Bmr_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(bmr_already_init)
	{
		if (var_night_mode)
		{
			color = DEF_DRAW_WHITE;
			back_color = DEF_DRAW_BLACK;
		}

		if(Util_is_watch_changed() || var_need_reflesh || !var_eco_mode)
		{
			var_need_reflesh = false;
			Draw_frame_ready();
			Draw_screen_ready(0, back_color);
			Draw_top_ui();
			Draw(bmr_status, 0, 20, 0.65, 0.65, color);

			Draw_apply_draw();
		}
		else
			gspWaitForVBlank();
	}

	Util_log_save(DEF_BMR_EXIT_STR, "threadJoin()...", threadJoin(bmr_exit_thread, DEF_THREAD_WAIT_TIME));	
	threadFree(bmr_exit_thread);
	Util_remove_watch(&bmr_status);
	var_need_reflesh = true;

	Util_log_save(DEF_BMR_EXIT_STR, "Exited.");
}

void Bmr_main(void)
{
	int color = DEF_DRAW_BLACK;
	int line_color = DEF_DRAW_WEAK_BLACK;
	int back_color = DEF_DRAW_WHITE;
	int text_color_cache = DEF_DRAW_BLACK;

	if (var_night_mode)
	{
		color = DEF_DRAW_WHITE;
		line_color = DEF_DRAW_WEAK_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	if(Util_is_watch_changed() || var_need_reflesh || !var_eco_mode)
	{
		var_need_reflesh = false;
		Draw_frame_ready();

		if(var_turn_on_top_lcd)
		{
			Draw_screen_ready(0, back_color);

			Draw_line(23 + bmr_x_offset, 15, line_color, 23 + bmr_x_offset, 240, line_color, 2);
			Draw_line(93 + bmr_x_offset, 15, line_color, 93 + bmr_x_offset, 240, line_color, 2);
			Draw_line(138 + bmr_x_offset, 15, line_color, 138 + bmr_x_offset, 240, line_color, 2);
			Draw_line(178 + bmr_x_offset, 15, line_color, 178 + bmr_x_offset, 240, line_color, 2);
			Draw_line(218 + bmr_x_offset, 15, line_color, 218 + bmr_x_offset, 240, line_color, 2);
			Draw_line(258 + bmr_x_offset, 15, line_color, 258 + bmr_x_offset, 240, line_color, 2);
			Draw_line(373 + bmr_x_offset, 15, line_color, 373 + bmr_x_offset, 240, line_color, 2);
			Draw_line(423 + bmr_x_offset, 15, line_color, 423 + bmr_x_offset, 240, line_color, 2);
			Draw_line(493 + bmr_x_offset, 15, line_color, 493 + bmr_x_offset, 240, line_color, 2);
			Draw(bmr_msg[DEF_BMR_MODEL_MSG], 25 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_TOTAL_MSG], 95 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_AVG_MSG], 140 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_MAX_MSG], 180 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_MIN_MSG], 220 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_USER_NAME_MSG], 260 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_APP_VER_MSG], 375 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_SYS_VER_MSG], 425 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_DATE_MSG], 495 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			for(int i = 0; i < 22; i++)
			{
				Draw_line(0, 30 + (i * 10), line_color, 400, 30 + (i * 10), line_color, 2);
				Draw(std::to_string(bmr_ranking[i + (int)bmr_y_offset]), 0 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(bmr_model[i + (int)bmr_y_offset], 25 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(Util_convert_seconds_to_time(bmr_total_time[i + (int)bmr_y_offset]).substr(0, Util_convert_seconds_to_time(bmr_total_time[i + (int)bmr_y_offset]).length() - 2), 95 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(Util_convert_seconds_to_time(bmr_avg_time[i + (int)bmr_y_offset]).substr(0, Util_convert_seconds_to_time(bmr_avg_time[i + (int)bmr_y_offset]).length() - 2), 140 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(Util_convert_seconds_to_time(bmr_max_time[i + (int)bmr_y_offset]).substr(0, Util_convert_seconds_to_time(bmr_max_time[i + (int)bmr_y_offset]).length() - 2), 180 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(Util_convert_seconds_to_time(bmr_min_time[i + (int)bmr_y_offset]).substr(0, Util_convert_seconds_to_time(bmr_min_time[i + (int)bmr_y_offset]).length() - 2), 220 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(bmr_user_name[i + (int)bmr_y_offset], 260 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(bmr_app_ver[i + (int)bmr_y_offset], 375 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(bmr_system_ver[i + (int)bmr_y_offset], 425 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(bmr_date[i + (int)bmr_y_offset], 495 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
			}

			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui();

			if(var_3d_mode)
			{
				Draw_screen_ready(2, back_color);

				if(Util_log_query_log_show_flag())
					Util_log_draw();

				Draw_top_ui();
			}
		}
		
		if(var_turn_on_bottom_lcd)
		{
			Draw_screen_ready(1, back_color);

			Draw(DEF_BMR_VER, 0.0, 0.0, 0.4, 0.4, DEF_DRAW_GREEN);

			if(Util_err_query_error_show_flag())
				Util_err_draw();

			//Model selection
			for(int i = 0; i < 7; i++)
			{
				if(bmr_model_mode == i)
					text_color_cache = DEF_DRAW_RED;
				else if(bmr_dl_log_request)
					text_color_cache = DEF_DRAW_WEAK_BLACK;
				else
					text_color_cache = color;

				Draw(bmr_msg[DEF_BMR_MODEL_SELECTION_MSG + i], 40, 40 + (i * 20), 0.5, 0.5, text_color_cache, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 240, 20,
				DEF_DRAW_BACKGROUND_ENTIRE_BOX, &bmr_model_selection_button[i], bmr_model_selection_button[i].selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			}
			//download ranking
			Draw(bmr_msg[DEF_BMR_DL_RANKING_MSG], 100, 190, 0.45, 0.45, bmr_dl_log_request ? DEF_DRAW_WEAK_BLACK : color, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 120, 20,
			DEF_DRAW_BACKGROUND_ENTIRE_BOX, &bmr_dl_ranking_button, bmr_dl_ranking_button.selected ? DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			Draw_bot_ui();
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}
