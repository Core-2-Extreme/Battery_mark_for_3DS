#include "system/headers.hpp"

struct Graph_data
{
	u8 battery_level[DEF_BMARK_BMR_NUM_OF_HISTORY];
	u8 battery_temp[DEF_BMARK_BMR_NUM_OF_HISTORY];
	double battery_voltage[DEF_BMARK_BMR_NUM_OF_HISTORY];
};

bool bmr_main_run = false;
bool bmr_thread_run = false;
bool bmr_already_init = false;
bool bmr_thread_suspend = true;
bool bmr_dl_log_request = false;
bool bmr_show_graph_request = false;
int bmr_ranking[DEF_BMR_NUM_OF_LOGS];
int bmr_model_mode = 0;
int bmr_selected_ranking = 0;
int bmr_selected_graph_pos = 0;
int bmr_wait = 0;
int bmr_page_num = 0;
int bmr_y_offset = 0;
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
Image_data bmr_model_selection_button[7], bmr_dl_ranking_button, bmr_close_graph_button, bmr_graph_area;
Graph_data bmr_graph_data[DEF_BMR_NUM_OF_LOGS];

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
	int string_offset = 0;
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
			string_offset = 0;
			cache = "";
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
			bmr_y_offset = 0;
			bmr_selected_ranking = 0;
			var_need_reflesh = true;

			log_num = Util_log_save(DEF_BMR_WORKER_THREAD_STR, "Util_httpc_dl_data()...");
			result = Util_httpc_dl_data(DEF_BMARK_BMR_RANKING_SERVER_URL + "?mode=" + std::to_string(bmr_model_mode) + "&start_pos=" + std::to_string(bmr_page_num * 200 + 1) + "&logs=200&ver=" + std::to_string(DEF_CURRENT_APP_VER_INT), &data, size, &dled_size, true, 5);
			Util_log_add(log_num, result.string, result.code);
			if(result.code == 0)
			{
				string_data = (char*)data;
				Util_safe_linear_free(data);
				data = NULL;

				cut_pos = string_data.find(",", string_offset);
				if(!(cut_pos == std::string::npos))
				{
					cache = string_data.substr(0, cut_pos);
					string_offset = cut_pos + 1;
				}

				if(cache == "Success")
				{
					//parse data
					for(int i = 0; i < DEF_BMR_NUM_OF_LOGS; i++)
					{
						cut_pos = string_data.find(",", string_offset);
						if(cut_pos == std::string::npos)
							break;

						bmr_ranking[i] = atoi(string_data.substr(string_offset, cut_pos - string_offset).c_str());
						string_offset = cut_pos + 1;

						cut_pos = string_data.find(",", string_offset);
						if(cut_pos == std::string::npos)
							break;

						bmr_date[i] = string_data.substr(string_offset, cut_pos - string_offset);
						string_offset = cut_pos + 1;

						cut_pos = string_data.find(",", string_offset);
						if(cut_pos == std::string::npos)
							break;

						bmr_app_ver[i] = string_data.substr(string_offset, cut_pos - string_offset);
						string_offset = cut_pos + 1;

						cut_pos = string_data.find(",", string_offset);
						if(cut_pos == std::string::npos)
							break;

						bmr_system_ver[i] = string_data.substr(string_offset, cut_pos - string_offset);
						string_offset = cut_pos + 1;

						cut_pos = string_data.find(",", string_offset);
						if(cut_pos == std::string::npos)
							break;

						bmr_model[i] = string_data.substr(string_offset, cut_pos - string_offset);
						string_offset = cut_pos + 1;

						cut_pos = string_data.find(",", string_offset);
						if(cut_pos == std::string::npos)
							break;

						bmr_user_name[i] = string_data.substr(string_offset, cut_pos - string_offset);
						string_offset = cut_pos + 1;

						cut_pos = string_data.find(",", string_offset);
						if(cut_pos == std::string::npos)
							break;

						bmr_max_time[i] = atof(string_data.substr(string_offset, cut_pos - string_offset).c_str());
						string_offset = cut_pos + 1;

						cut_pos = string_data.find(",", string_offset);
						if(cut_pos == std::string::npos)
							break;

						bmr_avg_time[i] = atof(string_data.substr(string_offset, cut_pos - string_offset).c_str());
						string_offset = cut_pos + 1;

						cut_pos = string_data.find(",", string_offset);
						if(cut_pos == std::string::npos)
							break;

						bmr_min_time[i] = atof(string_data.substr(string_offset, cut_pos - string_offset).c_str());
						string_offset = cut_pos + 1;

						cut_pos = string_data.find(",", string_offset);
						if(cut_pos == std::string::npos)
							break;

						bmr_total_time[i] = atof(string_data.substr(string_offset, cut_pos - string_offset).c_str());
						string_offset = cut_pos + 1;

						for(int k = 0; k < DEF_BMARK_BMR_NUM_OF_HISTORY; k++)
						{
							cut_pos = string_data.find(",", string_offset);
							if(cut_pos == std::string::npos)
								break;

							bmr_graph_data[i].battery_level[k] = atoi(string_data.substr(string_offset, cut_pos - string_offset).c_str());
							string_offset = cut_pos + 1;

							cut_pos = string_data.find(",", string_offset);
							if(cut_pos == std::string::npos)
								break;

							bmr_graph_data[i].battery_temp[k] = atoi(string_data.substr(string_offset, cut_pos - string_offset).c_str());
							string_offset = cut_pos + 1;

							cut_pos = string_data.find(",", string_offset);
							if(cut_pos == std::string::npos)
								break;

							bmr_graph_data[i].battery_voltage[k] = atof(string_data.substr(string_offset, cut_pos - string_offset).c_str());
							string_offset = cut_pos + 1;
						}

						cut_pos = string_data.find("\n", string_offset);
						if(cut_pos == std::string::npos)
							break;

						string_offset = cut_pos + 1;
					}
				}
				else
				{
					Util_err_set_error_message(DEF_ERR_GAS_RETURNED_NOT_SUCCESS_STR, string_data, DEF_BMR_WORKER_THREAD_STR, DEF_ERR_GAS_RETURNED_NOT_SUCCESS);
					Util_err_set_error_show_flag(true);
				}
			}
			else
			{
				Util_err_set_error_message(result.string, result.error_description, DEF_BMR_WORKER_THREAD_STR, result.code);
				Util_err_set_error_show_flag(true);
			}
			Util_safe_linear_free(data);
			data = NULL;

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

		for(int k = 0; k < DEF_BMARK_BMR_NUM_OF_HISTORY; k++)
		{
			bmr_graph_data[i].battery_level[k] = 0;
			bmr_graph_data[i].battery_temp[k] = 0;
			bmr_graph_data[i].battery_voltage[k] = 0;
		}
	}

	bmr_page_num = 0;
	bmr_selected_ranking = 0;
	bmr_y_offset = 0;
	bmr_selected_graph_pos = 0;
	bmr_model_mode = 6;
	bmr_dl_log_request = true;
	bmr_dl_ranking_button.c2d = var_square_image[0];
	bmr_close_graph_button.c2d = var_square_image[0];
	bmr_graph_area.c2d = var_square_image[0];
	for(int i = 0; i < 7; i++)
		bmr_model_selection_button[i].c2d = var_square_image[0];

	Util_add_watch(&bmr_selected_ranking);
	Util_add_watch(&bmr_selected_graph_pos);
	Util_add_watch(&bmr_x_offset);
	Util_add_watch(&bmr_y_offset);
	Util_add_watch(&bmr_model_mode);
	Util_add_watch(&bmr_dl_log_request);
	Util_add_watch(&bmr_show_graph_request);
	Util_add_watch(&bmr_dl_ranking_button.selected);
	Util_add_watch(&bmr_close_graph_button.selected);
	Util_add_watch(&bmr_graph_area.selected);
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

	Util_log_save(DEF_BMR_EXIT_STR, "threadJoin()...", threadJoin(bmr_worker_thread, DEF_THREAD_WAIT_TIME));

	bmr_status += "\nCleaning up...";

	threadFree(bmr_worker_thread);

	Util_remove_watch(&bmr_selected_ranking);
	Util_remove_watch(&bmr_selected_graph_pos);
	Util_remove_watch(&bmr_x_offset);
	Util_remove_watch(&bmr_y_offset);
	Util_remove_watch(&bmr_model_mode);
	Util_remove_watch(&bmr_dl_log_request);
	Util_remove_watch(&bmr_show_graph_request);
	Util_remove_watch(&bmr_dl_ranking_button.selected);
	Util_remove_watch(&bmr_close_graph_button.selected);
	Util_remove_watch(&bmr_graph_area.selected);
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
		else if(Util_hid_is_pressed(key, bmr_dl_ranking_button) && !bmr_dl_log_request && !bmr_show_graph_request)
			bmr_dl_ranking_button.selected = true;
		else if((key.p_b || (Util_hid_is_released(key, bmr_dl_ranking_button) && bmr_dl_ranking_button.selected)) && !bmr_dl_log_request && !bmr_show_graph_request)
			bmr_dl_log_request = true;
		else if(key.p_a)
			bmr_show_graph_request = true;
		else if(Util_hid_is_pressed(key, bmr_close_graph_button) && bmr_show_graph_request)
			bmr_close_graph_button.selected = true;
		else if((key.p_y || (Util_hid_is_released(key, bmr_close_graph_button) && bmr_close_graph_button.selected)) && bmr_show_graph_request)
			bmr_show_graph_request = false;
		else if(key.p_l && !bmr_dl_log_request)
		{
			if(bmr_page_num - 1 >= 0)
			{
				bmr_page_num--;
				bmr_dl_log_request = true;
			}
			else
				bmr_page_num = 0;
		}
		else if(key.p_r && !bmr_dl_log_request)
		{
			if(bmr_page_num + 1 <= 49)
			{
				bmr_page_num++;
				bmr_dl_log_request = true;
			}
			else
				bmr_page_num = 49;
		}
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
			if(bmr_wait - 1 < 0)
			{
				if(key.held_time > 240)
					bmr_wait = 0;
				else if(key.held_time > 120)
					bmr_wait = 1;
				else if(key.held_time > 60)
					bmr_wait = 2;
				else
					bmr_wait = 3;

				if(bmr_selected_ranking + 1 <= 18)
					bmr_selected_ranking++;
				else if(bmr_y_offset + 1 > 181)
					bmr_y_offset = 181;
				else
					bmr_y_offset += 1;
			}
			else
				bmr_wait--;
		}
		else if(key.h_c_up)
		{
			if(bmr_wait - 1 < 0)
			{
				if(key.held_time > 240)
					bmr_wait = 0;
				else if(key.held_time > 120)
					bmr_wait = 1;
				else if(key.held_time > 60)
					bmr_wait = 2;
				else
					bmr_wait = 3;

				if(bmr_selected_ranking - 1 >= 0)
					bmr_selected_ranking--;
				else if(bmr_y_offset - 1 < 0)
					bmr_y_offset = 0;
				else
					bmr_y_offset -= 1;
			}
			else
				bmr_wait--;
		}
		else if(bmr_show_graph_request)
		{
			if(Util_hid_is_pressed(key, bmr_graph_area))
				bmr_graph_area.selected = true;
			if(bmr_graph_area.selected && key.h_touch)
			{
				if(key.touch_x < 20)
					bmr_selected_graph_pos = 0;
				else if(key.touch_x > 20 + DEF_BMARK_BMR_NUM_OF_HISTORY - 1)
					bmr_selected_graph_pos = DEF_BMARK_BMR_NUM_OF_HISTORY - 1;
				else
					bmr_selected_graph_pos = key.touch_x - 20;
			}
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
		bmr_close_graph_button.selected = false;
		bmr_graph_area.selected = false;
		for(int i = 0; i < 7; i++)
			bmr_model_selection_button[i].selected = false;
	}

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Bmr_init(bool draw)
{
	Util_log_save(DEF_BMR_INIT_STR, "Initializing...");
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	Util_add_watch(&bmr_status);
	bmr_status = "";

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		bmr_init_thread = threadCreate(Bmr_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		bmr_init_thread = threadCreate(Bmr_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!bmr_already_init)
	{
		if(draw)
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
		else
			usleep(20000);
	}

	if(!(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) || !var_core_2_available)
		APT_SetAppCpuTimeLimit(10);

	Util_log_save(DEF_BMR_EXIT_STR, "threadJoin()...", threadJoin(bmr_init_thread, DEF_THREAD_WAIT_TIME));
	threadFree(bmr_init_thread);
	Bmr_resume();

	Util_log_save(DEF_BMR_INIT_STR, "Initialized.");
}

void Bmr_exit(bool draw)
{
	Util_log_save(DEF_BMR_EXIT_STR, "Exiting...");

	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	bmr_status = "";

	bmr_exit_thread = threadCreate(Bmr_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(bmr_already_init)
	{
		if(draw)
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
		else
			usleep(20000);
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

			Draw_texture(var_square_image[0], color, 23 + bmr_x_offset, 15, 1, 205);
			Draw_texture(var_square_image[0], color, 93 + bmr_x_offset, 15, 1, 205);
			Draw_texture(var_square_image[0], color, 138 + bmr_x_offset, 15, 1, 205);
			Draw_texture(var_square_image[0], color, 178 + bmr_x_offset, 15, 1, 205);
			Draw_texture(var_square_image[0], color, 218 + bmr_x_offset, 15, 1, 205);
			Draw_texture(var_square_image[0], color, 258 + bmr_x_offset, 15, 1, 205);
			Draw_texture(var_square_image[0], color, 373 + bmr_x_offset, 15, 1, 205);
			Draw_texture(var_square_image[0], color, 423 + bmr_x_offset, 15, 1, 205);
			Draw_texture(var_square_image[0], color, 493 + bmr_x_offset, 15, 1, 205);
			Draw(bmr_msg[DEF_BMR_MODEL_MSG], 25 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_TOTAL_MSG], 95 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_AVG_MSG], 140 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_MAX_MSG], 180 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_MIN_MSG], 220 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_USER_NAME_MSG], 260 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_APP_VER_MSG], 375 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_SYS_VER_MSG], 425 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			Draw(bmr_msg[DEF_BMR_DATE_MSG], 495 + bmr_x_offset, 17.5, 0.425, 0.425, color);
			for(int i = 0; i < 19; i++)
			{
				if(i == bmr_selected_ranking)
					Draw_texture(var_square_image[0], DEF_DRAW_WEAK_GREEN, 0, 30 + (i * 10), 400, 10);

				Draw_texture(var_square_image[0], color, 0, 30 + (i * 10), 400, 1);
				Draw(std::to_string(bmr_ranking[i + bmr_y_offset]), 0 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(bmr_model[i + bmr_y_offset], 25 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(Util_convert_seconds_to_time(bmr_total_time[i + bmr_y_offset]).substr(0, Util_convert_seconds_to_time(bmr_total_time[i + bmr_y_offset]).length() - 2), 95 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(Util_convert_seconds_to_time(bmr_avg_time[i + bmr_y_offset]).substr(0, Util_convert_seconds_to_time(bmr_avg_time[i + bmr_y_offset]).length() - 2), 140 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(Util_convert_seconds_to_time(bmr_max_time[i + bmr_y_offset]).substr(0, Util_convert_seconds_to_time(bmr_max_time[i + bmr_y_offset]).length() - 2), 180 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(Util_convert_seconds_to_time(bmr_min_time[i + bmr_y_offset]).substr(0, Util_convert_seconds_to_time(bmr_min_time[i + bmr_y_offset]).length() - 2), 220 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(bmr_user_name[i + bmr_y_offset], 260 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(bmr_app_ver[i + bmr_y_offset], 375 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(bmr_system_ver[i + bmr_y_offset], 425 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
				Draw(bmr_date[i + bmr_y_offset], 495 + bmr_x_offset, 30 + (i * 10), 0.375, 0.375, color);
			}
			Draw(bmr_msg[DEF_BMR_CONTROLS_MSG], 0, 220, 0.45, 0.45, color, DEF_DRAW_X_ALIGN_LEFT, DEF_DRAW_Y_ALIGN_CENTER, 400, 20);
			Draw(bmr_msg[DEF_BMR_PAGE_MSG] + std::to_string(bmr_page_num + 1), 0, 220, 0.45, 0.45, color, DEF_DRAW_X_ALIGN_RIGHT, DEF_DRAW_Y_ALIGN_CENTER, 400, 20);

			if(bmr_dl_log_request)
			{
				Draw(bmr_msg[DEF_BMR_DL_MSG], 0, 220, 0.45, 0.45, DEF_DRAW_WHITE, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 400, 20,
				DEF_DRAW_BACKGROUND_UNDER_TEXT, var_square_image[0], DEF_DRAW_BLACK);
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

			//Graph for battery level/temp/voltage
			if(bmr_show_graph_request)
			{
				Draw_texture(&bmr_graph_area, DEF_DRAW_AQUA, 20, 40, DEF_BMARK_BMR_NUM_OF_HISTORY, 100);
				Draw_texture(var_square_image[0], DEF_DRAW_AQUA, 20, 140, DEF_BMARK_BMR_NUM_OF_HISTORY, 60);
				Draw("(V)", 0, 20, 0.45, 0.45, 0xFF00A0FF, DEF_DRAW_X_ALIGN_RIGHT, DEF_DRAW_Y_ALIGN_TOP, 20, 20);
				Draw("(%)", 275, 20, 0.45, 0.45, DEF_DRAW_RED);
				Draw("(゜C)", 295, 20, 0.45, 0.45, 0xFF00A000);
				for(int i = 0; i < 6; i++)
				{
					Draw_line(20, 140 - (i * 20), line_color, 300, 140 - (i * 20), line_color, 1);
					Draw(std::to_string(i * 0.25 + 3).substr(0, 4), 0, 135 - (i * 20), 0.4, 0.4, color, DEF_DRAW_X_ALIGN_RIGHT, DEF_DRAW_Y_ALIGN_TOP, 20, 20);
					Draw(std::to_string(i * 20), 300, 135 - (i * 20), 0.45, 0.45, color);
				}
				Draw_line(bmr_selected_graph_pos + 20, 140, DEF_DRAW_BLACK, bmr_selected_graph_pos + 20, 40, DEF_DRAW_BLACK, 1);
				for(int i = 0; i < DEF_BMARK_BMR_NUM_OF_HISTORY - 1; i++)
				{
					Draw_line(i + 20, 140 - bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_level[i], DEF_DRAW_RED, i + 21, 140 - bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_level[i + 1], DEF_DRAW_RED, 1);
					Draw_line(i + 20, 140 - bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_temp[i], 0xFF00A000, i + 21, 140 - bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_temp[i + 1], 0xFF00A000, 1);
					Draw_line(i + 20, 140 - (bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_voltage[i] == 0 ? 0 : (bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_voltage[i] - 3) * 80), 0xFF00A0FF,
					i + 21, 140 - (bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_voltage[i + 1] == 0 ? 0 : (bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_voltage[i + 1] - 3) * 80), 0xFF00A0FF, 1);
				}

				Draw(bmr_msg[DEF_BMR_BATTERY_LEVEL_MSG] + std::to_string(bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_level[bmr_selected_graph_pos]) + "%",
				20, 140, 0.5, 0.5, DEF_DRAW_RED, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 140, 15);
				Draw(bmr_msg[DEF_BMR_BATTERY_TEMP_MSG] + std::to_string(bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_temp[bmr_selected_graph_pos]) + "゜C",
				160, 140, 0.5, 0.5, 0xFF00A000, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 140, 15);
				Draw(bmr_msg[DEF_BMR_BATTERY_VOLTAGE_MSG] + std::to_string(bmr_graph_data[bmr_selected_ranking + bmr_y_offset].battery_voltage[bmr_selected_graph_pos]).substr(0, 5) + "V",
				20, 160, 0.5, 0.5, 0xFF00A0FF, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 280, 15);
				Draw(bmr_msg[DEF_BMR_CLOSE_MSG], 20, 180, 0.45, 0.45, DEF_DRAW_BLACK, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, DEF_BMARK_BMR_NUM_OF_HISTORY, 20,
				DEF_DRAW_BACKGROUND_ENTIRE_BOX, &bmr_close_graph_button, bmr_close_graph_button.selected ? DEF_DRAW_GREEN : DEF_DRAW_WEAK_GREEN);
			}

			if(Util_err_query_error_show_flag())
				Util_err_draw();

			Draw_bot_ui();
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();
}
