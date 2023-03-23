#include "system/headers.hpp"

u8 bmark_battery_level_history[DEF_BMARK_BMR_NUM_OF_HISTORY];
u8 bmark_battery_temp_history[DEF_BMARK_BMR_NUM_OF_HISTORY];
int bmark_copied = 0;
int bmark_cache_copied = 0;
int bmark_fps = 0;
int bmark_cache_fps = 0;
int bmark_remain_test = 0;
int bmark_initial_battery = 0;
int bmark_final_battery = 0;
int bmark_graph_index = 0;
int bmark_selected_graph_pos = 0;
double bmark_max_time = 0;
double bmark_min_time = 0;
double bmark_avg_time = 0;
double bmark_elapsed_time = 0;
double bmark_total_elapsed_time = 0;
double bmark_remain_time = 0;
double bmark_battery_voltage_history[DEF_BMARK_BMR_NUM_OF_HISTORY];
bool bmark_main_run = false;
bool bmark_thread_run = false;
bool bmark_already_init = false;
bool bmark_thread_suspend = true;
bool bmark_start_mark_request = false;
bool bmark_stop_mark_request = false;
bool bmark_send_data = true;
bool bmark_sending_data = false;
bool bmark_stop_check_request = false;
bool bmark_type_user_name_request = false;
std::string bmark_user_name = "user";
std::string bmark_system_ver = "unknown";
std::string bmark_msg[DEF_BMARK_NUM_OF_MSG];
std::string bmark_status = "";
Thread bmark_video_thread, bmark_check_thread, bmark_update_thread, bmark_copy_thread, bmark_init_thread, bmark_exit_thread;
Image_data bmark_image, bmark_start_button, bmark_stop_button, bmark_send_data_button, bmark_yes_button, bmark_no_button, bmark_graph_area;

extern "C" void memcpy_asm(u8*, u8*, int);

bool Bmark_query_init_flag(void)
{
	return bmark_already_init;
}

bool Bmark_query_running_flag(void)
{
	return bmark_main_run;
}

Result_with_string Bmark_load_msg(std::string lang)
{
	return Util_load_msg("bmark_" + lang + ".txt", bmark_msg, DEF_BMARK_NUM_OF_MSG);
}

void Bmark_check_thread(void* arg)
{
	Util_log_save(DEF_BMARK_WATCH_THREAD_STR, "Thread started.");
	bool eco_mode = false;
	bool night_mode = false;
	bool plugged = false;
	float time = 0;
	int screen_brightness = 100;
	int battery_level = 0;
	int pre_battery_level = 0;
	int test_amount = 0;
	int rotated_screen_width = 0, rotated_screen_height = 0;
	u8* frame_buffer = NULL;
	u8* graph_buffer = NULL;
	u8* data = NULL;
	u8 shell_state = 0;
	u16* u16_title= NULL;
	u16* u16_message= NULL;
	u16 screen_width = 0, screen_height = 0;
	u32 dled_size = 0;
	char time_cache[64];
	std::string title = "";
	std::string message = "";
	std::string file_name = "";
	std::string save_data = "";
	std::string time_data = "";
	std::string post_data = "";
	std::string cache = "";
	std::string graph_csv_data = "";
	Result_with_string result;
	TickCounter stop_watch, elapsed_time_stop_watch;
	osTickCounterStart(&stop_watch);
	osTickCounterStart(&elapsed_time_stop_watch);
	u16_title = (u16*)malloc(0x100);
	u16_message = (u16*)malloc(0x1000);
	
	while (bmark_thread_run)
	{
		if(bmark_start_mark_request)
		{
			osTickCounterUpdate(&stop_watch);
			osTickCounterUpdate(&elapsed_time_stop_watch);
			battery_level = var_battery_level_raw;
			pre_battery_level = battery_level;
			bmark_initial_battery = battery_level - 1;
			bmark_final_battery = bmark_initial_battery - bmark_remain_test;
			time = 0;
			bmark_max_time = -1;
			bmark_avg_time = -1;
			bmark_min_time = -1;
			bmark_elapsed_time = 0;
			bmark_total_elapsed_time = 0;
			bmark_remain_time = 1234567890;
			test_amount = bmark_remain_test;
			graph_csv_data = "";
			bmark_graph_index = 0;
			for(int i = 0; i < DEF_BMARK_BMR_NUM_OF_HISTORY; i++)
			{
				bmark_battery_level_history[i] = 0;
				bmark_battery_temp_history[i] = 0;
				bmark_battery_voltage_history[i] = 0;
			}
			bmark_battery_level_history[bmark_graph_index] = var_battery_level_raw;
			bmark_battery_temp_history[bmark_graph_index] = var_battery_temp;
			bmark_battery_voltage_history[bmark_graph_index] = var_battery_voltage;
			bmark_graph_index++;

			eco_mode = var_eco_mode;
			screen_brightness = var_lcd_brightness;
			night_mode = var_night_mode;
			var_eco_mode = false;
			var_lcd_brightness = 164;
			var_top_lcd_brightness = var_lcd_brightness;
			var_bottom_lcd_brightness = var_lcd_brightness;
			var_night_mode = false;
			Util_cset_set_screen_brightness(true, true, var_lcd_brightness);
			aptSetHomeAllowed(false);

			memset(time_cache, 0, 64);
			sprintf(time_cache, "%04d_%02d_%02d_%02d_%02d_%02d", var_years, var_months, var_days, var_hours, var_minutes, var_seconds);

			time_data = time_cache;
			file_name = time_data;
			//csv format
			save_data = bmark_msg[DEF_BMARK_TIME_MSG] + "," + bmark_msg[DEF_BMARK_BATTERY_MSG] + "," + bmark_msg[DEF_BMARK_BATTERY_TIME_MSG]
			+ "," + bmark_msg[DEF_BMARK_BATTERY_VOLTAGE_MSG] + "," + bmark_msg[DEF_BMARK_BATTERY_TEMP_MSG] + "\n";
			result = Util_file_save_to_file(file_name + ".csv", DEF_MAIN_DIR + "result/", (u8*)save_data.c_str(), save_data.length(), true);
			Util_log_save(DEF_BMARK_WATCH_THREAD_STR, "Util_file_save_to_file()..." + result.string, result.code);

			if(battery_level < (bmark_remain_test + 3))
			{
				bmark_stop_mark_request = true;
				Util_err_set_error_message(bmark_msg[DEF_BMARK_NO_ENOUGH_BATTERY_MSG], bmark_msg[DEF_BMARK_NO_ENOUGH_BATTERY_DESCRIPTION_0_MSG] + std::to_string(bmark_remain_test + 3) + bmark_msg[DEF_BMARK_NO_ENOUGH_BATTERY_DESCRIPTION_1_MSG], DEF_BMARK_WATCH_THREAD_STR, -1);
				Util_err_set_error_show_flag(true);
			}
			else
			{
				while(true)
				{
					PTMU_GetShellState(&shell_state);
					PTMU_GetAdapterState(&plugged);
					if(plugged)
					{
						bmark_stop_mark_request = true;
						Util_err_set_error_message(bmark_msg[DEF_BMARK_DO_NOT_CHARGE_MSG], bmark_msg[DEF_BMARK_ABORTED_MSG], DEF_BMARK_WATCH_THREAD_STR, -2);
						Util_err_set_error_show_flag(true);
					}
					if(shell_state == 0)
					{
						bmark_stop_mark_request = true;
						Util_err_set_error_message(bmark_msg[DEF_BMARK_DO_NOT_CLOSE_MSG], bmark_msg[DEF_BMARK_ABORTED_MSG], DEF_BMARK_WATCH_THREAD_STR, -2);
						Util_err_set_error_show_flag(true);
					}

					battery_level = var_battery_level_raw;
					if(pre_battery_level > battery_level)
					{
						pre_battery_level = battery_level;
						break;
					}
					
					if(bmark_stop_mark_request)
						break;

					usleep(10000);
				}
				
				osTickCounterUpdate(&stop_watch);
				osTickCounterUpdate(&elapsed_time_stop_watch);
				while(true)
				{
					PTMU_GetShellState(&shell_state);
					PTMU_GetAdapterState(&plugged);
					if(plugged)
					{
						bmark_stop_mark_request = true;
						Util_err_set_error_message(bmark_msg[DEF_BMARK_DO_NOT_CHARGE_MSG], bmark_msg[DEF_BMARK_ABORTED_MSG], DEF_BMARK_WATCH_THREAD_STR, -2);
						Util_err_set_error_show_flag(true);
					}
					if(shell_state == 0)
					{
						bmark_stop_mark_request = true;
						Util_err_set_error_message(bmark_msg[DEF_BMARK_DO_NOT_CLOSE_MSG], bmark_msg[DEF_BMARK_ABORTED_MSG], DEF_BMARK_WATCH_THREAD_STR, -2);
						Util_err_set_error_show_flag(true);
					}

					osTickCounterUpdate(&elapsed_time_stop_watch);
					time = osTickCounterRead(&elapsed_time_stop_watch) / 1000;
					bmark_total_elapsed_time += time;
					bmark_elapsed_time += time;
					if(bmark_remain_time < 0 && bmark_remain_time != -1234567890)
						bmark_remain_time = 0;
					else if(bmark_remain_time != 1234567890)
						bmark_remain_time -= time;

					battery_level = var_battery_level_raw;
					if(pre_battery_level > battery_level)
					{
						osTickCounterUpdate(&stop_watch);
						time = (osTickCounterRead(&stop_watch) / 1000) / (pre_battery_level - battery_level);
						bmark_elapsed_time = 0;
						bmark_remain_test -= pre_battery_level - battery_level;
						pre_battery_level = battery_level;
						if(bmark_remain_test < 0)
							bmark_remain_test = 0;
						
						if(bmark_max_time < time || bmark_max_time == -1)
							bmark_max_time = time;//set max time

						if(bmark_min_time > time || bmark_min_time == -1)
							bmark_min_time = time;//set min time
						
	
						bmark_avg_time = bmark_total_elapsed_time / (test_amount - bmark_remain_test); //calc avg battery time
						bmark_remain_time = bmark_avg_time * bmark_remain_test;//calc remain time

						//csv format
						memset(time_cache, 0, 64);
						sprintf(time_cache, "%02d:%02d:%02d", var_hours, var_minutes, var_seconds);
						save_data = (std::string)time_cache + "," + std::to_string(battery_level) + "," + std::to_string(time).substr(0, 5) + ","
						+ std::to_string(var_battery_voltage).substr(0, 4) + "," + std::to_string(var_battery_temp) + "\n";
						result = Util_file_save_to_file(file_name + ".csv", DEF_MAIN_DIR + "result/", (u8*)save_data.c_str(), save_data.length(), false);
						Util_log_save(DEF_BMARK_WATCH_THREAD_STR, "Util_file_save_to_file()..." + result.string, result.code);

						if(bmark_remain_test == 0)
						{
							bmark_start_mark_request = false;
							bmark_stop_mark_request = true;
							//save result
							save_data = bmark_msg[DEF_BMARK_TIME_MSG] + "," + bmark_msg[DEF_BMARK_BATTERY_MSG] + "," + bmark_msg[DEF_BMARK_BATTERY_TIME_MSG]
							+ "," + bmark_msg[DEF_BMARK_BATTERY_VOLTAGE_MSG] + "\n"+ DEF_BMARK_VER + "\n" + time_data + 
							"\n" + std::to_string(bmark_initial_battery) + "% -> " + std::to_string(bmark_final_battery) + "%\n"
							+ bmark_msg[DEF_BMARK_BATTERY_TIME_MAX_MSG] + Util_convert_seconds_to_time(bmark_max_time) + "\n" 
							+ bmark_msg[DEF_BMARK_BATTERY_TIME_AVG_MSG] + Util_convert_seconds_to_time(bmark_avg_time) + "\n"
							+ bmark_msg[DEF_BMARK_BATTERY_TIME_MIN_MSG] + Util_convert_seconds_to_time(bmark_min_time) + "\n"
							+ bmark_msg[DEF_BMARK_TOTAL_ELAPSED_TIME_MSG] + Util_convert_seconds_to_time(bmark_total_elapsed_time);
							result = Util_file_save_to_file(file_name + ".csv", DEF_MAIN_DIR + "result/", (u8*)save_data.c_str(), save_data.length(), false);
							Util_log_save(DEF_BMARK_WATCH_THREAD_STR, "Util_file_save_to_file()..." + result.string, result.code);

							//save notification
							memset(u16_title, 0x0, 0x100);
							memset(u16_message, 0x0, 0x1000);
							title = bmark_msg[DEF_BMARK_RESULT_TITLE_MSG];
							message = DEF_BMARK_VER + "\n" + time_data + "\n" + std::to_string(bmark_initial_battery) + "% -> " + std::to_string(bmark_final_battery) + "%\n"
							+ bmark_msg[DEF_BMARK_BATTERY_TIME_MAX_MSG] + Util_convert_seconds_to_time(bmark_max_time) + "\n" 
							+ bmark_msg[DEF_BMARK_BATTERY_TIME_AVG_MSG] + Util_convert_seconds_to_time(bmark_avg_time) + "\n"
							+ bmark_msg[DEF_BMARK_BATTERY_TIME_MIN_MSG] + Util_convert_seconds_to_time(bmark_min_time) + "\n"
							+ bmark_msg[DEF_BMARK_TOTAL_ELAPSED_TIME_MSG] + Util_convert_seconds_to_time(bmark_total_elapsed_time);;
							utf8_to_utf16(u16_title, (u8*)title.c_str(), title.length());
							utf8_to_utf16(u16_message, (u8*)message.c_str(), message.length());

							result.code = newsInit();
							Util_log_save(DEF_BMARK_WATCH_THREAD_STR, "newsInit()...", result.code);
							if(result.code == 0)
							{
								result.code = NEWS_AddNotification(u16_title, title.length(), u16_message, message.length(), NULL, 0, false);
								Util_log_save(DEF_BMARK_WATCH_THREAD_STR, "NEWS_AddNotification()...", result.code);
							}
							newsExit();
							
							if(result.code != 0)
							{
								Util_err_set_error_message(result.string, result.error_description, DEF_BMARK_WATCH_THREAD_STR, result.code);
								Util_err_set_error_show_flag(true);
							}

							//save bottom screen (graph) as jpg
							frame_buffer = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &screen_width, &screen_height);
							if(frame_buffer)
							{
								result = Util_converter_rgb888_rotate_90_degree(frame_buffer, &graph_buffer, screen_width, screen_height, &rotated_screen_width, &rotated_screen_height);
								Util_log_save(DEF_BMARK_WATCH_THREAD_STR, "Util_converter_rgb888_rotate_90_degree()...", result.code);
								if(result.code == 0)
								{
									Util_converter_rgb888be_to_rgb888le(graph_buffer, rotated_screen_width, rotated_screen_height);
									result = Util_image_encoder_encode(DEF_MAIN_DIR + "result/" + file_name + "_graph.jpg", graph_buffer, rotated_screen_width, rotated_screen_height, DEF_ENCODER_IMAGE_CODEC_JPG, 85);
									Util_log_save(DEF_BMARK_WATCH_THREAD_STR, "Util_image_encoder_encode()...", result.code);
								}
								Util_safe_linear_free(graph_buffer);
								graph_buffer = NULL;
							}

							if(bmark_send_data)
							{
								bmark_sending_data = true;
								for(int i = 0; i < DEF_BMARK_BMR_NUM_OF_HISTORY; i++)
									graph_csv_data += std::to_string(bmark_battery_level_history[i]) + "," + std::to_string(bmark_battery_temp_history[i]) + "," + std::to_string(bmark_battery_voltage_history[i]).substr(0, 4) + ",";

								//send data to the ranking server
								post_data = "{ \"ver\" : \"" + DEF_BMARK_VER + "\","
									+ "\"system_ver\" : \"" + bmark_system_ver + "\","
									+ "\"model\" : \"" + var_model_name[var_model] + "\","
									+ "\"user_name\" : \"" + bmark_user_name + "\","
									+ "\"max_time\" : \"" + std::to_string(bmark_max_time) + "\","
									+ "\"avg_time\" : \"" + std::to_string(bmark_avg_time) + "\","
									+ "\"min_time\" : \"" + std::to_string(bmark_min_time) + "\","
									+ "\"total_time\" : \"" + std::to_string(bmark_total_elapsed_time) + "\","
									+ "\"graph_csv\" : \"" + graph_csv_data + "\""
								+ "}";

								result = Util_httpc_post_and_dl_data(DEF_BMARK_BMR_RANKING_SERVER_URL, (u8*)post_data.c_str(), post_data.length(), &data, 0x10000, &dled_size, true, 5);
								Util_log_save(DEF_BMARK_WATCH_THREAD_STR, "Util_httpc_post_and_dl_data()..." + result.string, result.code);
								if(result.code == 0)
								{
									cache = (char*)data;
									if(cache != "Success")
									{
										Util_err_set_error_message(DEF_ERR_GAS_RETURNED_NOT_SUCCESS_STR, (char*)data, DEF_BMARK_WATCH_THREAD_STR, DEF_ERR_GAS_RETURNED_NOT_SUCCESS);
										Util_err_set_error_show_flag(true);
									}
								}
								Util_safe_linear_free(data);
								data = NULL;
							}
							bmark_sending_data = false;
						}
					}
					
					if(bmark_stop_mark_request)
						break;

					usleep(10000);
				}
			}

			var_night_mode = night_mode;
			var_eco_mode = eco_mode;
			var_lcd_brightness = screen_brightness;
			var_top_lcd_brightness = var_lcd_brightness;
			var_bottom_lcd_brightness = var_lcd_brightness;
			Util_cset_set_screen_brightness(true, true, var_lcd_brightness);
			aptSetHomeAllowed(true);
			bmark_start_mark_request = false;
			bmark_stop_check_request = false;

			usleep(500000);
		}
		else
			usleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (bmark_thread_suspend)
			usleep(DEF_INACTIVE_THREAD_SLEEP_TIME);		
	}

	free(u16_title);
	free(u16_message);
	u16_title = NULL;
	u16_message = NULL;

	Util_log_save(DEF_BMARK_WATCH_THREAD_STR, "Thread exit.");
	threadExit(0);
}

void Bmark_video_thread(void* arg)
{
	Util_log_save(DEF_BMARK_VIDEO_THREAD_STR, "Thread started.");
	bool key_frame = false;
	bool error = false;
	int packet_type = DEF_DECODER_PACKET_TYPE_UNKNOWN;
	int packet_index = 0;
	int audio_tracks = 0;
	int video_tracks = 0;
	int subtitle_tracks = 0;
	double current_pos = 0;
	u8* video_yuv_image = NULL;
	u8* video_rgb_image = NULL;
	u8* cache = NULL;
	u32 read_size = 0;
	Result_with_string result;
	Video_info video_info;
	
	while (bmark_thread_run)
	{
		if(bmark_start_mark_request)
		{
			bmark_stop_mark_request = false;
			error = false;
			bmark_cache_fps = 0;
			APT_SetAppCpuTimeLimit(80);
			
			if(Util_file_check_file_exist("sample_video.mp4", DEF_MAIN_DIR).code != 0)
			{
				result = Util_file_load_from_rom("sample_video.mp4", "romfs:/gfx/", &cache, 0x40000, &read_size);
				Util_log_save(DEF_BMARK_VIDEO_THREAD_STR, "Util_file_load_from_rom()..." + result.string, result.code);
				if(result.code == 0)
				{
					result = Util_file_save_to_file("sample_video.mp4", DEF_MAIN_DIR, cache, read_size, true);
					Util_log_save(DEF_BMARK_VIDEO_THREAD_STR, "Util_file_save_to_file()..." + result.string, result.code);
				}

				Util_safe_linear_free(cache);
				cache = NULL;
				if(result.code != 0)
					error = true;
			}

			result = Util_decoder_open_file(DEF_MAIN_DIR + "sample_video.mp4", &audio_tracks, &video_tracks, &subtitle_tracks, 0);
			Util_log_save(DEF_BMARK_VIDEO_THREAD_STR, "Util_decoder_open_file()..." + result.string, result.code);
			if(result.code != 0)
				error = true;

			if(!error)
			{
				result = Util_video_decoder_init(0, video_tracks, 1, DEF_DECODER_THREAD_TYPE_NONE, 0);
				Util_log_save(DEF_BMARK_VIDEO_THREAD_STR, "Util_video_decoder_init()..." + result.string, result.code);
				if(result.code != 0)
					error = true;
				else
					Util_video_decoder_get_info(&video_info, 0, 0);
			}
			
			if(!error)
			{
				while(true)
				{
					var_afk_time = 0;
					result = Util_decoder_read_packet(0);
					if(result.code != 0)
						Util_decoder_seek(0, DEF_DECODER_SEEK_FLAG_BACKWARD, 0);
					else
					{
						Util_decoder_parse_packet(&packet_type, &packet_index, &key_frame, 0);
						if(packet_type == DEF_DECODER_PACKET_TYPE_VIDEO)
						{
							result = Util_decoder_ready_video_packet(packet_index, 0);
							if(result.code != 0)
								break;

							result = Util_video_decoder_decode(packet_index, 0);
							if(result.code == 0)
							{
								result = Util_video_decoder_get_image(&video_yuv_image, &current_pos, video_info.width, video_info.height, packet_index, 0);
								if(result.code == 0)
								{
									result = Util_converter_yuv420p_to_rgb565le_asm(video_yuv_image, &video_rgb_image, video_info.width, video_info.height);
									if(result.code == 0)
									{
										Draw_set_texture_data(&bmark_image, video_rgb_image, video_info.width, video_info.height, 256, 256, DEF_DRAW_FORMAT_RGB565);
										bmark_cache_fps++;
										var_need_reflesh = true;
									}
								}
								Util_safe_linear_free(video_yuv_image);
								Util_safe_linear_free(video_rgb_image);
								video_yuv_image = NULL;
								video_rgb_image = NULL;
							}
						}
						else if(DEF_DECODER_PACKET_TYPE_AUDIO)
							Util_decoder_skip_audio_packet(packet_index, 0);
						else if(DEF_DECODER_PACKET_TYPE_SUBTITLE)
							Util_decoder_skip_subtitle_packet(packet_index, 0);
					}

					if(bmark_stop_mark_request)
						break;
				}
			}

			Util_decoder_close_file(0);
			APT_SetAppCpuTimeLimit(5);

			if(error)
			{
				Util_err_set_error_message(result.string, result.error_description, DEF_BMARK_VIDEO_THREAD_STR, result.code);
				Util_err_set_error_show_flag(true);
			}
			usleep(500000);
		}
		else
			usleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (bmark_thread_suspend)
			usleep(DEF_INACTIVE_THREAD_SLEEP_TIME);
	}
	Util_log_save(DEF_BMARK_VIDEO_THREAD_STR, "Thread exit.");
	threadExit(0);
}

void Bmark_copy_thread(void* arg)
{
	Util_log_save(DEF_BMARK_COPY_THREAD_STR, "Thread started.");
	int size = 0x400000;
	u8* data[2] = { NULL, NULL, };

	while (bmark_thread_run)
	{
		if(bmark_start_mark_request)
		{
			bmark_cache_copied = 0;
			data[0] = (u8*)Util_safe_linear_alloc(size);
			data[1] = (u8*)Util_safe_linear_alloc(size);
			if(!data[0] || !data[1])
			{
				Util_err_set_error_message(DEF_ERR_OUT_OF_LINEAR_MEMORY_STR, "", DEF_BMARK_COPY_THREAD_STR, DEF_ERR_OUT_OF_LINEAR_MEMORY);
				Util_err_set_error_show_flag(true);
				bmark_stop_mark_request = true;
			}
			else
			{
				while(!bmark_stop_mark_request)
				{
					memcpy_asm(data[0], data[1], size);
					bmark_cache_copied += size;
					memcpy_asm(data[1], data[0], size);
					bmark_cache_copied += size;
				}
			}

			Util_safe_linear_free(data[0]);
			Util_safe_linear_free(data[1]);
			data[0] = NULL;
			data[1] = NULL;
			usleep(500000);
		}
		else
			usleep(DEF_ACTIVE_THREAD_SLEEP_TIME);

		while (bmark_thread_suspend)
			usleep(DEF_INACTIVE_THREAD_SLEEP_TIME);		
	}
	Util_log_save(DEF_BMARK_COPY_THREAD_STR, "Thread exit.");
	threadExit(0);
}

void Bmark_update_thread(void* arg)
{
	Util_log_save(DEF_BMARK_UPDATE_THREAD_STR, "Thread started.");
	u64 graph_update_ts = -1, performance_update_ts = -1, current_ts = -1;

	while (bmark_thread_run)
	{
		usleep(50000);
		current_ts = osGetTime();

		if(current_ts > performance_update_ts + 1000)
		{
			performance_update_ts = current_ts;
			//Update performance data
			bmark_copied = bmark_cache_copied;
			bmark_fps = bmark_cache_fps;
			bmark_cache_copied = 0;
			bmark_cache_fps = 0;
		}

		if(current_ts > graph_update_ts + 60000)
		{
			graph_update_ts = current_ts;

			//Shift graph content if graph is full
			if(bmark_graph_index + 1 >= DEF_BMARK_BMR_NUM_OF_HISTORY)
			{
				bmark_battery_level_history[bmark_graph_index] = var_battery_level_raw;
				bmark_battery_temp_history[bmark_graph_index] = var_battery_temp;
				bmark_battery_voltage_history[bmark_graph_index] = var_battery_voltage;
				for(int i = 1; i < DEF_BMARK_BMR_NUM_OF_HISTORY; i++)
				{
					bmark_battery_level_history[i - 1] = bmark_battery_level_history[i];
					bmark_battery_temp_history[i - 1] = bmark_battery_temp_history[i];
					bmark_battery_voltage_history[i - 1] = bmark_battery_voltage_history[i];
				}
			}
			else
			{
				bmark_battery_level_history[bmark_graph_index] = var_battery_level_raw;
				bmark_battery_temp_history[bmark_graph_index] = var_battery_temp;
				bmark_battery_voltage_history[bmark_graph_index] = var_battery_voltage;
				bmark_graph_index++;
			}
		}
	}
	Util_log_save(DEF_BMARK_UPDATE_THREAD_STR, "Thread exit.");
	threadExit(0);
}

void Bmark_resume(void)
{
	bmark_thread_suspend = false;
	bmark_main_run = true;
	var_need_reflesh = true;
	Menu_suspend();
}

void Bmark_suspend(void)
{
	bmark_thread_suspend = true;
	bmark_main_run = false;
	Menu_resume();
}

void Bmark_hid(Hid_info key)
{
	//Do nothing if app is suspended.
	if(aptShouldJumpToHome())
		return;

	if(Util_err_query_error_show_flag())
		Util_err_main(key);
	else
	{
		if(Util_hid_is_pressed(key, *Draw_get_bot_ui_button()))
			Draw_get_bot_ui_button()->selected = true;
		else if ((key.p_start || (Util_hid_is_released(key, *Draw_get_bot_ui_button()) && Draw_get_bot_ui_button()->selected)) && !bmark_start_mark_request)
			Bmark_suspend();

		if(bmark_stop_check_request)
		{
			if(Util_hid_is_pressed(key, bmark_yes_button))
				bmark_yes_button.selected = true;
			else if(key.p_a || (Util_hid_is_released(key, bmark_yes_button) && bmark_yes_button.selected))
			{
				bmark_stop_check_request = false;
				bmark_stop_mark_request = true;
			}
			else if(Util_hid_is_pressed(key, bmark_no_button))
				bmark_no_button.selected = true;
			else if(key.p_b || (Util_hid_is_released(key, bmark_no_button) && bmark_no_button.selected))
				bmark_stop_check_request = false;
		}
		else
		{
			if(Util_hid_is_pressed(key, bmark_graph_area))
				bmark_graph_area.selected = true;
			else if(bmark_graph_area.selected && key.h_touch)
			{
				if(key.touch_x < 20)
					bmark_selected_graph_pos = 0;
				else if(key.touch_x > 20 + DEF_BMARK_BMR_NUM_OF_HISTORY - 1)
					bmark_selected_graph_pos = DEF_BMARK_BMR_NUM_OF_HISTORY - 1;
				else
					bmark_selected_graph_pos = key.touch_x - 20;
			}
			else if(Util_hid_is_pressed(key, bmark_start_button) && !bmark_start_mark_request)
				bmark_start_button.selected = true;
			else if((key.p_a || (Util_hid_is_released(key, bmark_start_button) && bmark_start_button.selected)) && !bmark_start_mark_request)
			{
				if(bmark_send_data)
					bmark_type_user_name_request = true;

				//Wait user to complete typing
				while(bmark_type_user_name_request)
					usleep(100000);

				bmark_start_mark_request = true;
				bmark_remain_test = 90;
			}
			else if(Util_hid_is_pressed(key, bmark_stop_button) && bmark_start_mark_request)
				bmark_stop_button.selected = true;
			else if((key.p_b || (Util_hid_is_released(key, bmark_stop_button) && bmark_stop_button.selected)) && bmark_start_mark_request)
				bmark_stop_check_request = true;
			else if(Util_hid_is_pressed(key, bmark_send_data_button) && !bmark_start_mark_request)
				bmark_send_data_button.selected = true;
			else if(Util_hid_is_released(key, bmark_send_data_button) && bmark_send_data_button.selected && !bmark_start_mark_request && !bmark_sending_data)
				bmark_send_data = !bmark_send_data;
		}
	}

	if(!key.p_touch && !key.h_touch)
	{
		Draw_get_bot_ui_button()->selected = false;
		bmark_graph_area.selected = false;
		bmark_start_button.selected = false;
		bmark_stop_button.selected = false;
		bmark_send_data_button.selected = false;
		bmark_yes_button.selected = false;
		bmark_no_button.selected = false;
	}

	if(Util_log_query_log_show_flag())
		Util_log_main(key);
}

void Bmark_init_thread(void* arg)
{
	Util_log_save(DEF_BMARK_INIT_STR, "Thread started.");
	OS_VersionBin os_ver;
	char system_ver_char[0x50] = " ";

	bmark_status = "Starting threads...";

	bmark_thread_run = true;
	if(var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DS || var_model == CFG_MODEL_N3DSXL)
	{
		bmark_video_thread = threadCreate(Bmark_video_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_HIGH, 2, false);
		bmark_copy_thread = threadCreate(Bmark_copy_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}
	else
	{
		bmark_video_thread = threadCreate(Bmark_video_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_HIGH, 1, false);
		bmark_copy_thread = threadCreate(Bmark_copy_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_LOW, 0, false);
	}

	bmark_check_thread = threadCreate(Bmark_check_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_HIGH, 0, false);
	bmark_update_thread = threadCreate(Bmark_update_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_REALTIME, 0, false);

	bmark_status += "\nInitializing variables...";
	bmark_graph_index = 0;
	bmark_selected_graph_pos = 0;
	for(int i = 0; i < DEF_BMARK_BMR_NUM_OF_HISTORY; i++)
	{
		bmark_battery_level_history[i] = 0;
		bmark_battery_temp_history[i] = 0;
		bmark_battery_voltage_history[i] = 0;
	}
	osGetSystemVersionDataString(&os_ver, &os_ver, system_ver_char, 0x50);
	bmark_system_ver = system_ver_char;
	bmark_graph_area.c2d = var_square_image[0];
	bmark_start_button.c2d = var_square_image[0];
	bmark_stop_button.c2d = var_square_image[0];
	bmark_send_data_button.c2d = var_square_image[0];
	bmark_yes_button.c2d = var_square_image[0];
	bmark_no_button.c2d = var_square_image[0];
	Draw_texture_init(&bmark_image, 256, 256, DEF_DRAW_FORMAT_RGB565);
	Draw_set_texture_filter(&bmark_image, false);
	Util_add_watch(&bmark_selected_graph_pos);
	Util_add_watch(&bmark_graph_area.selected);
	Util_add_watch(&bmark_start_button.selected);
	Util_add_watch(&bmark_stop_button.selected);
	Util_add_watch(&bmark_send_data_button.selected);
	Util_add_watch(&bmark_yes_button.selected);
	Util_add_watch(&bmark_no_button.selected);
	Util_add_watch(&bmark_send_data);
	Util_add_watch(&bmark_stop_check_request);

	bmark_already_init = true;

	Util_log_save(DEF_BMARK_INIT_STR, "Thread exit.");
	threadExit(0);
}

void Bmark_exit_thread(void* arg)
{
	Util_log_save(DEF_BMARK_EXIT_STR, "Thread started.");

	bmark_stop_mark_request = true;
	bmark_thread_suspend = false;
	bmark_thread_run = false;

	bmark_status = "Exiting threads...";
	Util_log_save(DEF_BMARK_EXIT_STR, "threadJoin()...", threadJoin(bmark_video_thread, DEF_THREAD_WAIT_TIME));	

	bmark_status += ".";
	Util_log_save(DEF_BMARK_EXIT_STR, "threadJoin()...", threadJoin(bmark_check_thread, DEF_THREAD_WAIT_TIME));

	bmark_status += ".";
	Util_log_save(DEF_BMARK_EXIT_STR, "threadJoin()...", threadJoin(bmark_copy_thread, DEF_THREAD_WAIT_TIME));

	bmark_status += ".";
	Util_log_save(DEF_BMARK_EXIT_STR, "threadJoin()...", threadJoin(bmark_update_thread, DEF_THREAD_WAIT_TIME));

	bmark_status += "\nCleaning up...";	
	Draw_texture_free(&bmark_image);
	Util_remove_watch(&bmark_selected_graph_pos);
	Util_remove_watch(&bmark_graph_area.selected);
	Util_remove_watch(&bmark_start_button.selected);
	Util_remove_watch(&bmark_stop_button.selected);
	Util_remove_watch(&bmark_send_data_button.selected);
	Util_remove_watch(&bmark_yes_button.selected);
	Util_remove_watch(&bmark_no_button.selected);
	Util_remove_watch(&bmark_send_data);
	Util_remove_watch(&bmark_stop_check_request);
	threadFree(bmark_video_thread);
	threadFree(bmark_check_thread);
	threadFree(bmark_copy_thread);
	threadFree(bmark_update_thread);

	bmark_already_init = false;
	Util_log_save(DEF_BMARK_EXIT_STR, "Thread exit.");
	threadExit(0);
}

void Bmark_init(bool draw)
{
	Util_log_save(DEF_BMARK_INIT_STR, "Initializing...");
	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	Util_add_watch(&bmark_status);
	bmark_status = "";

	if((var_model == CFG_MODEL_N2DSXL || var_model == CFG_MODEL_N3DSXL || var_model == CFG_MODEL_N3DS) && var_core_2_available)
		bmark_init_thread = threadCreate(Bmark_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 2, false);
	else
	{
		APT_SetAppCpuTimeLimit(80);
		bmark_init_thread = threadCreate(Bmark_init_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);
	}

	while(!bmark_already_init)
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
				if(var_monitor_cpu_usage)
					Draw_cpu_usage_info();

				Draw(bmark_status, 0, 20, 0.65, 0.65, color);

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

	Util_log_save(DEF_BMARK_EXIT_STR, "threadJoin()...", threadJoin(bmark_init_thread, DEF_THREAD_WAIT_TIME));
	threadFree(bmark_init_thread);
	Bmark_resume();

	Util_log_save(DEF_BMARK_INIT_STR, "Initialized.");
}

void Bmark_exit(bool draw)
{
	Util_log_save(DEF_BMARK_EXIT_STR, "Exiting...");

	int color = DEF_DRAW_BLACK;
	int back_color = DEF_DRAW_WHITE;

	bmark_status = "";

	bmark_exit_thread = threadCreate(Bmark_exit_thread, (void*)(""), DEF_STACKSIZE, DEF_THREAD_PRIORITY_NORMAL, 1, false);

	while(bmark_already_init)
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
				if(var_monitor_cpu_usage)
					Draw_cpu_usage_info();

				Draw(bmark_status, 0, 20, 0.65, 0.65, color);

				Draw_apply_draw();
			}
			else
				gspWaitForVBlank();
		}
		else
			usleep(20000);
	}

	Util_log_save(DEF_BMARK_EXIT_STR, "threadJoin()...", threadJoin(bmark_exit_thread, DEF_THREAD_WAIT_TIME));	
	threadFree(bmark_exit_thread);
	Util_remove_watch(&bmark_status);
	var_need_reflesh = true;

	Util_log_save(DEF_BMARK_EXIT_STR, "Exited.");
}

void Bmark_main(void)
{
	int color = DEF_DRAW_BLACK;
	int weak_color = DEF_DRAW_WEAK_BLACK;
	int back_color = DEF_DRAW_WHITE;

	if (var_night_mode)
	{
		color = DEF_DRAW_WHITE;
		weak_color = DEF_DRAW_WEAK_WHITE;
		back_color = DEF_DRAW_BLACK;
	}

	if(Util_is_watch_changed() || var_need_reflesh || !var_eco_mode)
	{
		var_need_reflesh = false;
		Draw_frame_ready();

		if(var_turn_on_top_lcd)
		{
			Draw_screen_ready(0, back_color);

			Draw_texture(&bmark_image, 0, 15, 256, 144);
			Draw(std::to_string(bmark_initial_battery) + "% -> " + std::to_string(bmark_final_battery) + "%", 0, 160, 0.425, 0.425, color);
			Draw(bmark_msg[DEF_BMARK_BATTERY_TIME_MAX_MSG] + Util_convert_seconds_to_time(bmark_max_time), 0, 170, 0.425, 0.425, color);
			Draw(bmark_msg[DEF_BMARK_BATTERY_TIME_AVG_MSG] + Util_convert_seconds_to_time(bmark_avg_time), 0, 180, 0.425, 0.425, color);
			Draw(bmark_msg[DEF_BMARK_BATTERY_TIME_MIN_MSG] + Util_convert_seconds_to_time(bmark_min_time), 0, 190, 0.425, 0.425, color);
			Draw(bmark_msg[DEF_BMARK_ELAPSED_TIME_MSG] + Util_convert_seconds_to_time(bmark_elapsed_time), 0, 200, 0.425, 0.425, color);
			Draw(bmark_msg[DEF_BMARK_TOTAL_ELAPSED_TIME_MSG] + Util_convert_seconds_to_time(bmark_total_elapsed_time), 0, 210, 0.425, 0.425, color);
			if(bmark_remain_time == 1234567890)
				Draw(bmark_msg[DEF_BMARK_REMAINING_TIME_MSG] + bmark_msg[DEF_BMARK_CALCULATING_MSG], 0, 220, 0.425, 0.425, color);
			else
				Draw(bmark_msg[DEF_BMARK_REMAINING_TIME_MSG] + Util_convert_seconds_to_time(bmark_remain_time), 0, 220, 0.425, 0.425, color);

			if(bmark_start_mark_request)
				Draw(bmark_msg[DEF_BMARK_BATTERY_MARK_IN_PROGRESS_MSG], 200, 180, 0.5, 0.5, DEF_DRAW_RED);
			else if(bmark_sending_data)
				Draw(bmark_msg[DEF_BMARK_SUBMITING_RESULT_MSG], 230, 180, 0.5, 0.5, DEF_DRAW_RED);

			Draw(std::to_string(var_battery_voltage).substr(0, 5) + "V " + std::to_string(var_battery_level_raw) + "% " + std::to_string(var_battery_temp) + "゜C", 240, 210, 0.65, 0.65, DEF_DRAW_BLUE);

			Draw("memcpy thread " + std::to_string(bmark_copied / 1024 / 1024) + "MB/s", 260, 20, 0.425, 0.425, color);
			Draw("video thread " + std::to_string(bmark_fps) + "fps", 260, 30, 0.425, 0.425, color);

			if(Util_log_query_log_show_flag())
				Util_log_draw();

			Draw_top_ui();

			if(var_monitor_cpu_usage)
				Draw_cpu_usage_info();

			if(var_3d_mode)
			{
				Draw_screen_ready(2, back_color);

				if(Util_log_query_log_show_flag())
					Util_log_draw();

				Draw_top_ui();

				if(var_monitor_cpu_usage)
					Draw_cpu_usage_info();
			}
		}

		if(var_turn_on_bottom_lcd)
		{
			Draw_screen_ready(1, back_color);

			Draw(DEF_BMARK_VER, 0, 0, 0.4, 0.4, DEF_DRAW_GREEN);

			//Graph for battery level/temp/voltage
			Draw_texture(&bmark_graph_area, DEF_DRAW_WEAK_AQUA, 20, 30, DEF_BMARK_BMR_NUM_OF_HISTORY, 100);
			Draw_texture(var_square_image[0], DEF_DRAW_WEAK_AQUA, 20, 130, DEF_BMARK_BMR_NUM_OF_HISTORY, 30);
			Draw("(V)", 0, 10, 0.45, 0.45, 0xFF00A0FF, DEF_DRAW_X_ALIGN_RIGHT, DEF_DRAW_Y_ALIGN_TOP, 20, 20);
			Draw("(%)", 275, 10, 0.45, 0.45, DEF_DRAW_RED);
			Draw("(゜C)", 295, 10, 0.45, 0.45, 0xFF00A000);
			for(int i = 0; i < 6; i++)
			{
				Draw_line(20, 130 - (i * 20), weak_color, 300, 130 - (i * 20), weak_color, 1);
				Draw(std::to_string(i * 0.25 + 3).substr(0, 4), 0, 125 - (i * 20), 0.4, 0.4, color, DEF_DRAW_X_ALIGN_RIGHT, DEF_DRAW_Y_ALIGN_TOP, 20, 20);
				Draw(std::to_string(i * 20), 300, 125 - (i * 20), 0.45, 0.45, color);
			}
			Draw_line(bmark_selected_graph_pos + 20, 130, DEF_DRAW_BLACK, bmark_selected_graph_pos + 20, 30, DEF_DRAW_BLACK, 1);
			for(int i = 0; i < DEF_BMARK_BMR_NUM_OF_HISTORY - 1; i++)
			{
				Draw_line(i + 20, 130 - bmark_battery_level_history[i], DEF_DRAW_RED, i + 21, 130 - bmark_battery_level_history[i + 1], DEF_DRAW_RED, 1);
				Draw_line(i + 20, 130 - bmark_battery_temp_history[i], 0xFF00A000, i + 21, 130 - bmark_battery_temp_history[i + 1], 0xFF00A000, 1);
				Draw_line(i + 20, 130 - (bmark_battery_voltage_history[i] == 0 ? 0 : (bmark_battery_voltage_history[i] - 3) * 80), 0xFF00A0FF,
				i + 21, 130 - (bmark_battery_voltage_history[i + 1] == 0 ? 0 : (bmark_battery_voltage_history[i + 1] - 3) * 80), 0xFF00A0FF, 1);
			}
			Draw(bmark_msg[DEF_BMARK_GRAPH_BATTERY_LEVEL_MSG] + std::to_string(bmark_battery_level_history[bmark_selected_graph_pos]) + "%",
			20, 130, 0.5, 0.5, DEF_DRAW_RED, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 140, 15);
			Draw(bmark_msg[DEF_BMARK_GRAPH_BATTERY_TEMP_MSG] + std::to_string(bmark_battery_temp_history[bmark_selected_graph_pos]) + "゜C",
			160, 130, 0.5, 0.5, 0xFF00A000, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 140, 15);
			Draw(bmark_msg[DEF_BMARK_GRAPH_BATTERY_VOLTAGE_MSG] + std::to_string(bmark_battery_voltage_history[bmark_selected_graph_pos]).substr(0, 5) + "V",
			20, 145, 0.5, 0.5, 0xFF00A0FF, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 280, 15);

			//start, stop and option button
			Draw(bmark_msg[DEF_BMARK_START_MSG], 20, 175, 0.425, 0.425, bmark_start_mark_request ? weak_color : color, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 135, 15,
			DEF_DRAW_BACKGROUND_ENTIRE_BOX, &bmark_start_button, bmark_start_button.selected ?  DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			Draw(bmark_msg[DEF_BMARK_STOP_MSG], 165, 175, 0.425, 0.425, bmark_start_mark_request ? color : weak_color, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 135, 15,
			DEF_DRAW_BACKGROUND_ENTIRE_BOX, &bmark_stop_button, bmark_stop_button.selected ?  DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);
			Draw(bmark_msg[DEF_BMARK_SEND_DATA_MSG] + bmark_msg[DEF_BMARK_OFF_MSG + bmark_send_data], 20, 200, 0.425, 0.425, bmark_start_mark_request ? weak_color : color, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER,
			280, 15, DEF_DRAW_BACKGROUND_ENTIRE_BOX, &bmark_send_data_button, bmark_send_data_button.selected ?  DEF_DRAW_AQUA : DEF_DRAW_WEAK_AQUA);

			if(bmark_stop_check_request)
			{
				//Stop benchmark confirmation dialog
				Draw_texture(var_square_image[0], DEF_DRAW_AQUA, 40, 100, 240, 60);
				Draw(bmark_msg[DEF_BMARK_ABORT_CONFIRMATION_MSG], 40, 100, 0.45, 0.45, color, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_TOP, 240, 20);
				Draw(bmark_msg[DEF_BMARK_YES_MSG], 40, 140, 0.45, 0.45, color, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 110, 20,
				DEF_DRAW_BACKGROUND_ENTIRE_BOX, &bmark_yes_button, bmark_yes_button.selected ? DEF_DRAW_YELLOW : DEF_DRAW_WEAK_YELLOW);
				Draw(bmark_msg[DEF_BMARK_NO_MSG], 170, 140, 0.45, 0.45, color, DEF_DRAW_X_ALIGN_CENTER, DEF_DRAW_Y_ALIGN_CENTER, 110, 20,
				DEF_DRAW_BACKGROUND_ENTIRE_BOX, &bmark_no_button, bmark_no_button.selected ? DEF_DRAW_YELLOW : DEF_DRAW_WEAK_YELLOW);
			}

			if(Util_err_query_error_show_flag())
				Util_err_draw();

			Draw_bot_ui();
		}

		Draw_apply_draw();
	}
	else
		gspWaitForVBlank();

	if(bmark_type_user_name_request)
	{
		Util_swkbd_init(SWKBD_TYPE_NORMAL, SWKBD_NOTEMPTY_NOTBLANK, 1, 16, bmark_msg[DEF_BMARK_USER_NAME_HINT_MSG], bmark_user_name, SWKBD_PREDICTIVE_INPUT);
		Util_swkbd_launch(&bmark_user_name);
		Util_swkbd_exit();
		bmark_type_user_name_request = false;
	}
}
