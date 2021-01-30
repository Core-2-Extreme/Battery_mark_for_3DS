#include <3ds.h>
#include <string>
#include <unistd.h>

#include "headers.hpp"

#include "battery_mark.hpp"

#include "menu.hpp"
#include "setting_menu.hpp"

int bmark_copied = 0;
int bmark_cache_copied = 0;
int bmark_fps = 0;
int bmark_cache_fps = 0;
int bmark_remain_test = 0;
int bmark_initial_battery = 0;
int bmark_final_battery = 0;
double bmark_max_time = 0;
double bmark_min_time = 0;
double bmark_avg_time = 0;
double bmark_elapsed_time = 0;
double bmark_total_elapsed_time = 0;
double bmark_remain_time = 0;
bool bmark_need_reflesh = false;
bool bmark_main_run = false;
bool bmark_thread_run = false;
bool bmark_already_init = false;
bool bmark_thread_suspend = true;
bool bmark_start_mark_request = false;
bool bmark_stop_mark_request = false;
bool bmark_send_data = true;
bool bmark_new_3ds = false;
bool bmark_sending_data = false;
bool bmark_stop_check_request = false;
std::string bmark_user_name = "user";
std::string bmark_system_ver = "unknown";
std::string bmark_msg[BMARK_NUM_OF_MSG];
std::string bmark_ver = "v2.0.0";
std::string bmark_video_thread_string = "Bmark/Decoder thread";
std::string bmark_check_thread_string = "Bmark/Check thread";
std::string bmark_copy_thread_string = "Bmark/Copy thread";
std::string bmark_update_thread_string = "Bmark/Update thread";
std::string bmark_init_string = "Bmark/Init";
std::string bmark_exit_string = "Bmark/Exit";
std::string bmark_logs[18];
Thread bmark_video_thread, bmark_check_thread, bmark_update_thread, bmark_copy_thread;
C2D_Image bmark_image[1] = { NULL, };

extern "C" void memcpy_asm(u8*, u8*, int);

bool Bmark_query_init_flag(void)
{
	return bmark_already_init;
}

bool Bmark_query_running_flag(void)
{
	return bmark_main_run;
}

void Bmark_set_msg(int msg_num, std::string msg)
{
	if (msg_num >= 0 && msg_num <= BMARK_NUM_OF_MSG)
		bmark_msg[msg_num] = msg;
}

Result_with_string Bmark_load_msg(std::string lang)
{
	u8* fs_buffer = NULL;
	u32 read_size;
	std::string setting_data[128];
	Result_with_string result;
	fs_buffer = (u8*)malloc(0x2000);

	result = File_load_from_rom("bmark_" + lang + ".txt", fs_buffer, 0x2000, &read_size, "romfs:/gfx/msg/");
	if (result.code != 0)
	{
		free(fs_buffer);
		return result;
	}

	result = Sem_parse_file((char*)fs_buffer, BMARK_NUM_OF_MSG, setting_data);
	if (result.code != 0)
	{
		free(fs_buffer);
		return result;
	}

	for (int k = 0; k < BMARK_NUM_OF_MSG; k++)
		Bmark_set_msg(k, setting_data[k]);

	free(fs_buffer);
	return result;
}

void Bmark_check_thread(void* arg)
{
	Log_log_save(bmark_check_thread_string, "Thread started.", 1234567890, false);
	bool eco_mode = false;
	bool night_mode = false;
	float time = 0;
	int screen_brightness = 100;
	int battery_level = 0;
	int pre_battery_level = 0;
	int test_amount = 0;
	u8* data = NULL;
	u8 shell_state = 0;
	u8 model = 0;
	u16* u16_title= NULL;
	u16* u16_message= NULL;
	u32 dled_size = 0;
	u32 status_code = 0;
	std::string title = "Battery mark v2 result";
	std::string message = "Battery mark v2 result message";
	std::string file_name = "";
	std::string save_data = "";
	std::string time_data = "";
	std::string post_data = "";
	std::string cache = "";
	Result_with_string result;
	TickCounter stop_watch, elapsed_time_stop_watch;
	osTickCounterStart(&stop_watch);
	osTickCounterStart(&elapsed_time_stop_watch);
	u16_title = (u16*)malloc(0x100);
	u16_message = (u16*)malloc(0x1000);
	data = (u8*)malloc(0x10000);

	while (bmark_thread_run)
	{
		if(bmark_start_mark_request)
		{
			osTickCounterUpdate(&stop_watch);
			osTickCounterUpdate(&elapsed_time_stop_watch);
			battery_level = Menu_query_battery_level_raw();
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
			for(int i = 0; i < 18; i++)
				bmark_logs[i] = "";

			eco_mode = Sem_query_settings(SEM_ECO_MODE);
			screen_brightness = Sem_query_settings_i(SEM_LCD_BRIGHTNESS);
			night_mode = Sem_query_settings(SEM_NIGHT_MODE);
			Sem_set_settings(SEM_NIGHT_MODE, false);
			Sem_set_settings(SEM_ECO_MODE, false);
			Sem_set_settings_i(SEM_LCD_BRIGHTNESS, 164);
			Menu_set_operation_flag(MENU_CHANGE_BRIGHTNESS_REQUEST, true);
			aptSetHomeAllowed(false);

			time_data = Menu_query_time(0);
			file_name = time_data + ".csv";
			//csv format
			save_data = bmark_ver + "\n" + time_data + "\n" + std::to_string(bmark_initial_battery) 
			+ "% -> " + std::to_string(bmark_final_battery) + "%\n" 
			+ bmark_msg[25] + "," + bmark_msg[0] + "," + bmark_msg[1] + "," + bmark_msg[18] + "\n";
			result = File_save_to_file(file_name, (u8*)save_data.c_str(), save_data.length(), "/Battery_mark/result/", true);
			Log_log_save(bmark_check_thread_string, "File_save_to_file()..." + result.string, result.code);

			if(battery_level < (bmark_remain_test + 6))
			{
				bmark_stop_mark_request = true;
				Err_set_error_message(bmark_msg[15], bmark_msg[16] + std::to_string(bmark_remain_test + 6) + bmark_msg[17], bmark_check_thread_string, -1);
				Err_set_error_show_flag(true);
			}
			else
			{
				while(true)
				{
					PTMU_GetShellState(&shell_state);
					if(Menu_query_battery_charge())
					{
						bmark_stop_mark_request = true;
						Err_set_error_message(bmark_msg[2], bmark_msg[3], bmark_check_thread_string, -2);
						Err_set_error_show_flag(true);
					}
					if(shell_state == 0)
					{
						bmark_stop_mark_request = true;
						Err_set_error_message(bmark_msg[24], bmark_msg[3], bmark_check_thread_string, -2);
						Err_set_error_show_flag(true);
					}

					battery_level = Menu_query_battery_level_raw();
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
					if(Menu_query_battery_charge())
					{
						bmark_stop_mark_request = true;
						Err_set_error_message(bmark_msg[2], bmark_msg[3], bmark_check_thread_string, -2);
						Err_set_error_show_flag(true);
					}
					if(shell_state == 0)
					{
						bmark_stop_mark_request = true;
						Err_set_error_message(bmark_msg[24], bmark_msg[3], bmark_check_thread_string, -2);
						Err_set_error_show_flag(true);
					}

					osTickCounterUpdate(&elapsed_time_stop_watch);
					time = osTickCounterRead(&elapsed_time_stop_watch) / 1000;
					bmark_total_elapsed_time += time;
					bmark_elapsed_time += time;
					if(bmark_remain_time < 0 && bmark_remain_time != -1234567890)
						bmark_remain_time = 0;
					else if(bmark_remain_time != 1234567890)
						bmark_remain_time -= time;

					battery_level = Menu_query_battery_level_raw();
					if(pre_battery_level > battery_level)
					{
						osTickCounterUpdate(&stop_watch);
						time = (osTickCounterRead(&stop_watch) / 1000) / (pre_battery_level - battery_level);
						bmark_elapsed_time = 0;
						bmark_remain_test -= pre_battery_level - battery_level;
						pre_battery_level = battery_level;
						if(bmark_remain_test < 0)
							bmark_remain_test = 0;
	
						for(int i = 16; i >= 0; i--)
							bmark_logs[i + 1] = bmark_logs[i];

						if(bmark_max_time < time || bmark_max_time == -1)
							bmark_max_time = time;//set max time

						if(bmark_min_time > time || bmark_min_time == -1)
							bmark_min_time = time;//set min time
						
	
						bmark_avg_time = bmark_total_elapsed_time / (test_amount - bmark_remain_test); //calc avg battery time
						bmark_remain_time = bmark_avg_time * bmark_remain_test;//calc remain time

						//save battery log
						bmark_logs[0] = bmark_msg[0] + std::to_string(battery_level) + "%" + bmark_msg[1] + Sem_convert_seconds_to_time(time) + " " + std::to_string(Menu_query_battery_voltage()).substr(0, 4) + "v";
						//csv format
						save_data = Menu_query_time(3) + "," + std::to_string(battery_level) + "," + std::to_string(time).substr(0, 5) + "," + std::to_string(Menu_query_battery_voltage()).substr(0, 4) + "\n";
						result = File_save_to_file(file_name, (u8*)save_data.c_str(), save_data.length(), "/Battery_mark/result/", false);
						Log_log_save(bmark_check_thread_string, "File_save_to_file()..." + result.string, result.code);

						if(bmark_remain_test == 0)
						{
							bmark_start_mark_request = false;
							bmark_stop_mark_request = true;
							//save result
							save_data = bmark_msg[25] + "," + bmark_msg[0] + "," + bmark_msg[1] + "," + bmark_msg[18] + "\n";
							save_data += bmark_msg[4] + Sem_convert_seconds_to_time(bmark_max_time) + "\n" 
							+ bmark_msg[5] + Sem_convert_seconds_to_time(bmark_avg_time) + "\n"
							+ bmark_msg[6] + Sem_convert_seconds_to_time(bmark_min_time) + "\n"
							+ bmark_msg[8] + Sem_convert_seconds_to_time(bmark_total_elapsed_time);
							result = File_save_to_file(file_name, (u8*)save_data.c_str(), save_data.length(), "/Battery_mark/result/", false);
							Log_log_save(bmark_check_thread_string, "File_save_to_file()..." + result.string, result.code);

							memset(u16_title, 0x0, 0x100);
							memset(u16_message, 0x0, 0x1000);
							title = bmark_msg[13];
							message = bmark_ver + "\n" + time_data + "\n" + std::to_string(bmark_initial_battery) + "% -> " + std::to_string(bmark_final_battery) + "%\n"
							+ bmark_msg[4] + Sem_convert_seconds_to_time(bmark_max_time) + "\n" 
							+ bmark_msg[5] + Sem_convert_seconds_to_time(bmark_avg_time) + "\n"
							+ bmark_msg[6] + Sem_convert_seconds_to_time(bmark_min_time) + "\n"
							+ bmark_msg[8] + Sem_convert_seconds_to_time(bmark_total_elapsed_time);;
							utf8_to_utf16(u16_title, (u8*)title.c_str(), title.length());
							utf8_to_utf16(u16_message, (u8*)message.c_str(), message.length());
							//save notification
							result.code = NEWS_AddNotification(u16_title, title.length(), u16_message, message.length(), NULL, 0, false);
							Log_log_save(bmark_check_thread_string, "NEWS_AddNotification()...", result.code);
							
							if(bmark_send_data)
							{
								bmark_sending_data = true;
								CFGU_GetSystemModel(&model);
								if(model == 0)
									cache = "OLD 3DS";
								else if(model == 1)
									cache = "OLD 3DS XL";
								else if(model == 2)
									cache = "NEW 3DS";
								else if(model == 3)
									cache = "OLD 2DS";
								else if(model == 4)
									cache = "NEW 3DS XL";
								else if(model == 5)
									cache = "NEW 2DS XL";
								else
									cache = "Unknown";
								
								//json format
								post_data = "{ \"ver\" : \"" + bmark_ver + "\","
									+ "\"system_ver\" : \"" + bmark_system_ver + "\","
									+ "\"model\" : \"" + cache + "\","
									+ "\"user_name\" : \"" + bmark_user_name + "\","
									+ "\"max_time\" : \"" + std::to_string(bmark_max_time) + "\","
									+ "\"avg_time\" : \"" + std::to_string(bmark_avg_time) + "\","
									+ "\"min_time\" : \"" + std::to_string(bmark_min_time) + "\","
									+ "\"total_time\" : \"" + std::to_string(bmark_total_elapsed_time) + "\""
								+ "}";

								memset(data, 0x0, 0x10000);
								//send data to the ranking server
								result = Httpc_post_and_dl_data("https://script.google.com/macros/s/AKfycbxo4iwCbxtI2ZuYQP7bKveRdHx6kPTH4rZ8Pg8nUguIq_7zcoooEUszJQ/exec", (char*)post_data.c_str(), post_data.length(), data, 0x10000, &dled_size, &status_code, true, BMARK_HTTP_POST_PORT0);
								Log_log_save(bmark_check_thread_string, "Httpc_post_and_dl_data()..." + result.string, result.code);
								cache = (char*)data;
								if(cache != "Success")
								{
									Err_set_error_message(Err_query_template_summary(GAS_RETURNED_NOT_SUCCESS), (char*)data, bmark_check_thread_string, GAS_RETURNED_NOT_SUCCESS);
									Err_set_error_show_flag(true);
								}
							}
							bmark_sending_data = false;
						}
					}
					
					if(bmark_stop_mark_request)
						break;

					usleep(10000);
				}
			}

			Sem_set_settings(SEM_NIGHT_MODE, night_mode);
			Sem_set_settings(SEM_ECO_MODE, eco_mode);
			Sem_set_settings_i(SEM_LCD_BRIGHTNESS, screen_brightness);
			Menu_set_operation_flag(MENU_CHANGE_BRIGHTNESS_REQUEST, true);
			aptSetHomeAllowed(true);
			bmark_start_mark_request = false;
			bmark_stop_check_request = false;

			usleep(500000);
		}
		else
			usleep(ACTIVE_THREAD_SLEEP_TIME);

		while (bmark_thread_suspend)
			usleep(INACTIVE_THREAD_SLEEP_TIME);		
	}

	free(u16_title);
	free(u16_message);
	u16_title = NULL;
	u16_message = NULL;

	Log_log_save(bmark_check_thread_string, "Thread exit.", 1234567890, false);
	threadExit(0);
}

void Bmark_video_thread(void* arg)
{
	Log_log_save(bmark_video_thread_string, "Thread started.", 1234567890, false);
	AVMediaType type = AVMEDIA_TYPE_UNKNOWN;
	bool key_frame = false;
	bool audio = false;
	bool video = false;
	bool error = false;
	int width = 0;
	int height = 0;
	u8* video_yuv_image = NULL;
	u8* video_rgb_image = NULL;
	u8* cache = NULL;
	u32 read_size = 0;
	double current_pos = 0;
	Result_with_string result;
	
	while (bmark_thread_run)
	{
		if(bmark_start_mark_request)
		{
			bmark_stop_mark_request = false;
			error = false;
			bmark_cache_fps = 0;
			result.code = APT_SetAppCpuTimeLimit(80);
			
			if(File_check_file_exist("sample_video.mp4", "/Battery_mark/").code != 0)
			{
				cache = (u8*)malloc(0x40000);
				if(cache == NULL)
				{
					result.string = Err_query_template_summary(OUT_OF_MEMORY);
					result.error_description = Err_query_template_detail(OUT_OF_MEMORY);
					result.code = OUT_OF_MEMORY;
					error = true;
				}
				result = File_load_from_rom("sample_video.mp4", cache, 0x40000, &read_size, "romfs:/gfx/");
				Log_log_save(bmark_video_thread_string, "File_load_from_rom()..." + result.string, result.code);
				if(result.code == 0)
				{
					result = File_save_to_file("sample_video.mp4", cache, read_size, "/Battery_mark/", true);
					Log_log_save(bmark_video_thread_string, "File_save_to_file()..." + result.string, result.code);
				}

				free(cache);
				cache = NULL;
				if(result.code != 0)
					error = true;
			}

			result = Util_open_file("/Battery_mark/sample_video.mp4", &audio, &video, UTIL_DECODER_0);
			Log_log_save(bmark_video_thread_string, "Util_open_file()..." + result.string, result.code);
			if(result.code != 0)
				error = true;

			if(!error)
			{
				result = Util_init_video_decoder(0, UTIL_DECODER_0);
				Log_log_save(bmark_video_thread_string, "Util_init_video_decoder()..." + result.string, result.code);
				if(result.code != 0)
					error = true;
			}
			
			if(!error)
			{
				while(true)
				{
					Menu_reset_afk_time();
					result = Util_read_packet(&type, UTIL_DECODER_0);
					if(result.code != 0)
						Util_seek(0, 8, UTIL_DECODER_0);

					if(type == AVMEDIA_TYPE_VIDEO)
					{
						result = Util_ready_video_packet(UTIL_DECODER_0);
						if(result.code != 0)
						{
							error = true;
							break;
						}

						result = Util_decode_video(&width, &height, &key_frame, &current_pos, UTIL_DECODER_0);
						if(result.code == 0)
						{
							video_yuv_image = (u8*)malloc(width * height * 1.5);
							video_rgb_image = (u8*)malloc(width * height * 2);
							if(!video_yuv_image || !video_rgb_image)
							{
								result.string = Err_query_template_summary(OUT_OF_MEMORY);
								result.error_description = Err_query_template_detail(OUT_OF_MEMORY);
								result.code = OUT_OF_MEMORY;
								error = true;
								break;
							}

							Util_get_video_image(video_yuv_image, width, height, UTIL_DECODER_0);
							Draw_yuv420p_to_rgb565(video_yuv_image, video_yuv_image + (width * height), video_yuv_image + (width * height) + (width * height / 4), video_rgb_image, width, height);
							
							linearFree(bmark_image[0].tex->data);
							bmark_image[0].tex->data = NULL;
							Draw_create_texture(bmark_image[0].tex, (Tex3DS_SubTexture*)bmark_image[0].subtex, video_rgb_image, width * height * 2, width, height, 2, 0, 0, 256, 256, GPU_RGB565);
							free(video_yuv_image);
							free(video_rgb_image);
							video_yuv_image = NULL;
							video_rgb_image = NULL;

							bmark_cache_fps++;
							bmark_need_reflesh = true;
						}
					}

					if(bmark_stop_mark_request)
						break;
				}
			}

			Util_close_file(UTIL_DECODER_0);
			Util_exit_video_decoder(UTIL_DECODER_0);
			result.code = APT_SetAppCpuTimeLimit(5);

			free(video_yuv_image);
			free(video_rgb_image);
			video_yuv_image = NULL;
			video_rgb_image = NULL;

			if(error)
			{
				Err_set_error_message(result.string, result.error_description, bmark_video_thread_string, result.code);
				Err_set_error_show_flag(true);
			}
			usleep(500000);
		}
		else
			usleep(ACTIVE_THREAD_SLEEP_TIME);

		while (bmark_thread_suspend)
			usleep(INACTIVE_THREAD_SLEEP_TIME);
	}
	Log_log_save(bmark_video_thread_string, "Thread exit.", 1234567890, false);
	threadExit(0);
}

void Bmark_copy_thread(void* arg)
{
	Log_log_save(bmark_copy_thread_string, "Thread started.", 1234567890, false);
	int size = 0x400000;
	u8* data[2] = { NULL, NULL, };

	while (bmark_thread_run)
	{
		if(bmark_start_mark_request)
		{
			bmark_cache_copied = 0;
			data[0] = (u8*)linearAlloc(size);
			data[1] = (u8*)linearAlloc(size);
			if(data[0] == NULL || data[1] == NULL)
			{
				Err_set_error_message(Err_query_template_summary(OUT_OF_LINEAR_MEMORY), Err_query_template_detail(OUT_OF_LINEAR_MEMORY), bmark_copy_thread_string, OUT_OF_LINEAR_MEMORY);
				Err_set_error_show_flag(true);
				bmark_stop_mark_request = true;
			}
			else
			{
				while(true)
				{
					memcpy_asm(data[0], data[1], size);
					bmark_cache_copied += size;
					memcpy_asm(data[1], data[0], size);
					bmark_cache_copied += size;

					if(bmark_stop_mark_request)
						break;
				}
			}

			linearFree(data[0]);
			linearFree(data[1]);
			data[0] = NULL;
			data[1] = NULL;
			usleep(500000);
		}
		else
			usleep(ACTIVE_THREAD_SLEEP_TIME);

		while (bmark_thread_suspend)
			usleep(INACTIVE_THREAD_SLEEP_TIME);		
	}
	Log_log_save(bmark_copy_thread_string, "Thread exit.", 1234567890, false);
	threadExit(0);
}

void Bmark_update_thread(void* arg)
{
	Log_log_save(bmark_update_thread_string, "Thread started.", 1234567890, false);
	while (bmark_thread_run)
	{
		usleep(1000000);
		bmark_copied = bmark_cache_copied;
		bmark_fps = bmark_cache_fps;
		bmark_cache_copied = 0;
		bmark_cache_fps = 0;
	}
	Log_log_save(bmark_update_thread_string, "Thread exit.", 1234567890, false);
	threadExit(0);
}

void Bmark_resume(void)
{
	bmark_thread_suspend = false;
	bmark_main_run = true;
	bmark_need_reflesh = true;
	Menu_suspend();
}

void Bmark_suspend(void)
{
	bmark_thread_suspend = true;
	bmark_main_run = false;
	Menu_resume();
}

void Bmark_init(void)
{
	Log_log_save(bmark_init_string, "Initializing...", 1234567890, FORCE_DEBUG);
	OS_VersionBin os_ver;
	char system_ver_char[0x50] = " ";

	Draw_progress("[Bmark] Starting threads...");
	bmark_thread_run = true;

	osGetSystemVersionDataString(&os_ver, &os_ver, system_ver_char, 0x50);
	bmark_system_ver = system_ver_char;
	APT_CheckNew3DS(&bmark_new_3ds);
	if(bmark_new_3ds)
	{
		bmark_video_thread = threadCreate(Bmark_video_thread, (void*)(""), STACKSIZE, PRIORITY_HIGH, 2, false);
		bmark_copy_thread = threadCreate(Bmark_copy_thread, (void*)(""), STACKSIZE, PRIORITY_NORMAL, 1, false);
	}
	else
	{
		bmark_video_thread = threadCreate(Bmark_video_thread, (void*)(""), STACKSIZE, PRIORITY_HIGH, 1, false);
		bmark_copy_thread = threadCreate(Bmark_copy_thread, (void*)(""), STACKSIZE, PRIORITY_LOW, 0, false);
	}

	bmark_check_thread = threadCreate(Bmark_check_thread, (void*)(""), STACKSIZE, PRIORITY_NORMAL, 0, false);
	bmark_update_thread = threadCreate(Bmark_update_thread, (void*)(""), STACKSIZE, PRIORITY_REALTIME, 0, false);

	Bmark_resume();
	bmark_already_init = true;
	bmark_image[0].tex = (C3D_Tex*)linearAlloc(sizeof(C3D_Tex));
	bmark_image[0].subtex = (Tex3DS_SubTexture*)linearAlloc(sizeof(Tex3DS_SubTexture));

	Log_log_save(bmark_init_string, "Initialized", 1234567890, FORCE_DEBUG);
}

void Bmark_exit(void)
{
	Log_log_save(bmark_exit_string, "Exiting...", 1234567890, FORCE_DEBUG);
	u64 time_out = 10000000000;
	int log_num;
	Result_with_string result;

	bmark_stop_mark_request = true;
	bmark_already_init = false;
	bmark_thread_suspend = false;
	bmark_thread_run = false;
	
	Draw_progress("[Bmark] Exiting...");

	log_num = Log_log_save(bmark_exit_string, "threadJoin()0/3...", 1234567890, FORCE_DEBUG);
	result.code = threadJoin(bmark_video_thread, time_out);
	if (result.code == 0)
		Log_log_add(log_num, Err_query_template_summary(0), result.code, FORCE_DEBUG);
	else
		Log_log_add(log_num, Err_query_template_summary(-1024), result.code, FORCE_DEBUG);

	log_num = Log_log_save(bmark_exit_string, "threadJoin()1/3...", 1234567890, FORCE_DEBUG);
	result.code = threadJoin(bmark_check_thread, time_out);
	if (result.code == 0)
		Log_log_add(log_num, Err_query_template_summary(0), result.code, FORCE_DEBUG);
	else
		Log_log_add(log_num, Err_query_template_summary(-1024), result.code, FORCE_DEBUG);

	log_num = Log_log_save(bmark_exit_string, "threadJoin()2/3...", 1234567890, FORCE_DEBUG);
	result.code = threadJoin(bmark_copy_thread, time_out);
	if (result.code == 0)
		Log_log_add(log_num, Err_query_template_summary(0), result.code, FORCE_DEBUG);
	else
		Log_log_add(log_num, Err_query_template_summary(-1024), result.code, FORCE_DEBUG);

	log_num = Log_log_save(bmark_exit_string, "threadJoin()3/3...", 1234567890, FORCE_DEBUG);
	result.code = threadJoin(bmark_update_thread, time_out);
	if (result.code == 0)
		Log_log_add(log_num, Err_query_template_summary(0), result.code, FORCE_DEBUG);
	else
		Log_log_add(log_num, Err_query_template_summary(-1024), result.code, FORCE_DEBUG);

	linearFree(bmark_image[0].tex->data);
	linearFree(bmark_image[0].tex);
	linearFree((void*)bmark_image[0].subtex);
	bmark_image[0].tex->data = NULL;
	bmark_image[0].tex = NULL;
	bmark_image[0].subtex = NULL;

	threadFree(bmark_video_thread);
	threadFree(bmark_check_thread);
	threadFree(bmark_copy_thread);
	threadFree(bmark_update_thread);
}

void Bmark_main(void)
{
	float r, g, b, a;
	double x, y;
	Hid_info key;

	if (Sem_query_settings(SEM_NIGHT_MODE))
	{
		r = 1.0;
		g = 1.0;
		b = 1.0;
		a = 0.75;
		white_or_black_tint = white_tint;
	}
	else
	{
		r = 0.0;
		g = 0.0;
		b = 0.0;
		a = 1.0;
		white_or_black_tint = black_tint;
	}

	Hid_query_key_state(&key);
	Log_main();
	if(Draw_query_need_reflesh() || !Sem_query_settings(SEM_ECO_MODE))
		bmark_need_reflesh = true;

	Hid_key_flag_reset();

	if(bmark_need_reflesh)
	{
		Draw_frame_ready();
		if (Sem_query_settings(SEM_NIGHT_MODE))
			Draw_screen_ready_to_draw(0, true, 2, 0.0, 0.0, 0.0);
		else
			Draw_screen_ready_to_draw(0, true, 2, 1.0, 1.0, 1.0);

		Draw_texture(bmark_image, 0, 0, 15, 256, 144);
		Draw(std::to_string(bmark_initial_battery) + "% -> " + std::to_string(bmark_final_battery) + "%", 0, 160, 0.45, 0.45, r, g, b, a);
		Draw(bmark_msg[4] + Sem_convert_seconds_to_time(bmark_max_time), 0, 170, 0.45, 0.45, r, g, b, a);
		Draw(bmark_msg[5] + Sem_convert_seconds_to_time(bmark_avg_time), 0, 180, 0.45, 0.45, r, g, b, a);
		Draw(bmark_msg[6] + Sem_convert_seconds_to_time(bmark_min_time), 0, 190, 0.45, 0.45, r, g, b, a);
		Draw(bmark_msg[7] + Sem_convert_seconds_to_time(bmark_elapsed_time), 0, 200, 0.45, 0.45, r, g, b, a);
		Draw(bmark_msg[8] + Sem_convert_seconds_to_time(bmark_total_elapsed_time), 0, 210, 0.45, 0.45, r, g, b, a);
		if(bmark_remain_time == 1234567890)
			Draw(bmark_msg[9] + bmark_msg[14], 0, 220, 0.45, 0.45, r, g, b, a);
		else
			Draw(bmark_msg[9] + Sem_convert_seconds_to_time(bmark_remain_time), 0, 220, 0.45, 0.45, r, g, b, a);

		if(bmark_start_mark_request)
			Draw(bmark_msg[12], 200, 180, 0.5, 0.5, 1.0, 0.0, 0.0, 1.0);
		else if(bmark_sending_data)
			Draw(bmark_msg[23], 230, 180, 0.5, 0.5, 1.0, 0.0, 0.0, 1.0);

		Draw(std::to_string(Menu_query_battery_voltage()).substr(0, 5) + "v " + std::to_string(Menu_query_battery_level_raw()) + "%", 260, 210, 0.75, 0.75, 0.0, 0.0, 1.0, 1.0);

		Draw("memcpy thread " + std::to_string(bmark_copied / 1024 / 1024) + "MB/s", 260, 20, 0.4, 0.4, r, g, b, a);
		Draw("video thread " + std::to_string(bmark_fps) + "fps", 260, 30, 0.4, 0.4, r, g, b, a);

		Draw_top_ui();

		if (Sem_query_settings(SEM_NIGHT_MODE))
			Draw_screen_ready_to_draw(1, true, 2, 0.0, 0.0, 0.0);
		else
			Draw_screen_ready_to_draw(1, true, 2, 1.0, 1.0, 1.0);

		for(int i = 0; i < 18; i++)
			Draw(bmark_logs[i], 0, i * 10, 0.4, 0.4, 1, 0, 0, 1);

		Draw(bmark_ver, 0.0, 210.0, 0.4, 0.4, 0.0, 1.0, 0.0, 1.0);
		Draw_texture(Square_image, weak_aqua_tint, 0, 40, 185, 110, 10);
		Draw_texture(Square_image, weak_aqua_tint, 0, 170, 185, 110, 10);
		Draw_texture(Square_image, weak_aqua_tint, 0, 40, 200, 240, 10);
		if(bmark_start_mark_request)
		{
			Draw(bmark_msg[10], 42.5, 185, 0.4, 0.4, r, g, b, 0.25);
			Draw(bmark_msg[11], 172.5, 185, 0.4, 0.4, r, g, b, a);
			Draw(bmark_msg[19] + bmark_msg[20 + bmark_send_data], 42.5, 200, 0.4, 0.4, r, g, b, 0.25);
		}
		else
		{
			Draw(bmark_msg[10], 42.5, 185, 0.4, 0.4, r, g, b, a);
			Draw(bmark_msg[11], 172.5, 185, 0.4, 0.4, r, g, b, 0.25);
			Draw(bmark_msg[19] + bmark_msg[20 + bmark_send_data], 42.5, 200, 0.4, 0.4, r, g, b, a);
		}
		
		if(bmark_stop_check_request)
		{
			Draw_texture(Square_image, aqua_tint, 0, 40, 100, 240, 60);
			Draw_texture(Square_image, weak_yellow_tint, 0, 40, 140, 110, 10);
			Draw_texture(Square_image, weak_yellow_tint, 0, 170, 140, 110, 10);
			Draw(bmark_msg[26], 42.5, 100, 0.4, 0.4, r, g, b, a);
			Draw(bmark_msg[27], 42.5, 140, 0.4, 0.4, r, g, b, a);
			Draw(bmark_msg[28], 172.5, 140, 0.4, 0.4, r, g, b, a);
		}
		Draw_bot_ui();
		Draw_touch_pos();

		Draw_apply_draw();
		bmark_need_reflesh = false;
	}
	else
		gspWaitForVBlank();

	if (Err_query_error_show_flag())
	{
		if (key.p_a || (key.p_touch && key.touch_x >= 150 && key.touch_x <= 170 && key.touch_y >= 150 && key.touch_y < 170))
			Err_set_error_show_flag(false);
		else if(key.p_x || (key.p_touch && key.touch_x >= 200 && key.touch_x <= 239 && key.touch_y >= 150 && key.touch_y <= 169))
			Err_save_error();
	}
	else
	{
		if ((key.p_start || (key.p_touch && key.touch_x >= 110 && key.touch_x <= 229 && key.touch_y >= 220 && key.touch_y <= 239)) && !bmark_start_mark_request && !bmark_sending_data)
			Bmark_suspend();
		if(bmark_stop_check_request)
		{
			if(key.p_a || (key.p_touch && key.touch_x >= 40 && key.touch_x <= 149 && key.touch_y >= 140 && key.touch_y <= 149))
			{
				bmark_stop_check_request = false;
				bmark_stop_mark_request = true;
				bmark_need_reflesh = true;
			}
			else if(key.p_b || (key.p_touch && key.touch_x >= 170 && key.touch_x <= 279 && key.touch_y >= 140 && key.touch_y <= 149))
			{
				bmark_stop_check_request = false;
				bmark_need_reflesh = true;
			}
		}
		else
		{
			if((key.p_a || (key.p_touch && key.touch_x >= 40 && key.touch_x <= 149 && key.touch_y >= 185 && key.touch_y <= 194)) && !bmark_start_mark_request)
			{
				if(bmark_send_data)
				{
					Hid_set_disable_flag(true);
					Swkbd_set_parameter(SWKBD_TYPE_NORMAL, SWKBD_NOTEMPTY_NOTBLANK, SWKBD_PREDICTIVE_INPUT, -1, 1, 16, bmark_msg[22], "");
					Swkbd_launch(16, &bmark_user_name, SWKBD_BUTTON_LEFT);
				}

				bmark_start_mark_request = true;
				bmark_remain_test = 90;
				bmark_need_reflesh = true;
			}
			else if((key.p_b || (key.p_touch && key.touch_x >= 170 && key.touch_x <= 279 && key.touch_y >= 185 && key.touch_y <= 194)) && bmark_start_mark_request)
			{
				bmark_stop_check_request = true;
				bmark_need_reflesh = true;
			}
			else if(key.p_touch && key.touch_x >= 40 && key.touch_x <= 279 && key.touch_y >= 200 && key.touch_x <= 209 && !bmark_start_mark_request && !bmark_sending_data)
			{
				bmark_send_data = !bmark_send_data;
				bmark_need_reflesh = true;
			}
		}
	}
}
