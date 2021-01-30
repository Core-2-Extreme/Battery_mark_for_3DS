#include <3ds.h>
#include <algorithm>
#include <unistd.h>
#include <string>
#include "citro2d.h"

#include "headers.hpp"

#include "menu.hpp"
#include "setting_menu.hpp"

std::string main_init_string = "Main/Init";
std::string main_exit_string = "Main/Exit";
std::string main_svc_name_list[10] = { "fs", "ac", "apt", "mcuHwc", "ptmu", "httpc", "romfs", "cfgu", "am", "news", };
/**/

void Init(void)
{
	int log_num;
	std::string texture_name_list[5] = { "wifi_signal", "battery_level", "battery_charge", "square", "ui", };
	Result_with_string result;

	Log_start_up_time_timer();
	Log_log_save(main_init_string , "Initializing...", 1234567890, false);
	Log_log_save(main_init_string, Menu_query_ver(), 1234567890, false);

	osSetSpeedupEnable(true);
	gfxInitDefault();
//	gfxSet3D(true);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);

	Draw_init();
	Draw_frame_ready();
	Draw_screen_ready_to_draw(0, true, 0, 1.0, 1.0, 1.0);
	Draw_screen_ready_to_draw(1, true, 0, 1.0, 1.0, 1.0);
	Draw_apply_draw();
	Draw_progress("[Main] Initializing service...");

	for (int i = 0; i < 10; i++)
	{
		log_num = Log_log_save(main_init_string, main_svc_name_list[i] + "Init()...", 1234567890, FORCE_DEBUG);

		if(i == 0)
			result.code = fsInit();
		else if (i == 1)
			result.code = acInit();
		else if (i == 2)
			result.code = aptInit();
		else if (i == 3)
			result.code = mcuHwcInit();
		else if (i == 4)
			result.code = ptmuInit();
		else if (i == 5)
			result.code = httpcInit(0x100000);
		else if (i == 6)
			result.code = romfsInit();
		else if (i == 7)
			result.code = cfguInit();
		else if (i == 8)
			result.code = amInit();
		else if(i == 9)
			result.code = newsInit();

		if (result.code == 0)
			Log_log_add(log_num, Err_query_template_summary(0), result.code, FORCE_DEBUG);
		else
			Log_log_add(log_num, Err_query_template_summary(-1024), result.code, FORCE_DEBUG);
	}

	log_num = Log_log_save(main_init_string, "Draw_load_kanji_samples()...", 1234567890, FORCE_DEBUG);
	Draw_load_kanji_samples();
	Log_log_add(log_num, result.string, result.code, FORCE_DEBUG);

	log_num = Log_log_save(main_init_string, "APT_SetAppCpuTimeLimit()...", 1234567890, FORCE_DEBUG);
	result.code = APT_SetAppCpuTimeLimit(5);
	if (result.code == 0)
		Log_log_add(log_num, Err_query_template_summary(0), result.code, FORCE_DEBUG);
	else
		Log_log_add(log_num, Err_query_template_summary(-1024), result.code, FORCE_DEBUG);

	aptSetSleepAllowed(true);

	Draw_progress("[Main] Starting threads...");
	Hid_init();
	Expl_init();
	Exfont_init();
	Sem_init();
	Sem_suspend();
	Sem_set_operation_flag(SEM_RELOAD_MSG_REQUEST, true);
	Sem_set_operation_flag(SEM_CHANGE_WIFI_STATE_REQUEST, true);
	for (int i = 0; i < EXFONT_NUM_OF_FONT_NAME; i++)
		Sem_set_load_external_font_request(i, true);

	for(int i = 0; i < 4; i++)
		Sem_set_load_system_font_request(i, true);

	Sem_set_operation_flag(SEM_LOAD_EXTERNAL_FONT_REQUEST, true);
	for(int i = 0; i < 300; i++)
	{
		if(Sem_query_loaded_external_font_flag(0))
			break;

		usleep(10000);
	}
	Sem_set_operation_flag(SEM_LOAD_SYSTEM_FONT_REQUEST, true);

	Draw_progress("[Main] Loading textures...");

	for (int i = 0; i < 5; i++)
	{
		log_num = Log_log_save(main_init_string, "Draw_load_texture()...", 1234567890, FORCE_DEBUG);

		if(i == 0)
			result = Draw_load_texture("romfs:/gfx/" + texture_name_list[i] + ".t3x", i, Wifi_icon_image, 0, 9);
		else if(i == 1)
			result = Draw_load_texture("romfs:/gfx/" + texture_name_list[i] + ".t3x", i, Battery_level_icon_image, 0, 21);
		else if (i == 2)
			result = Draw_load_texture("romfs:/gfx/" + texture_name_list[i] + ".t3x", i, Battery_charge_icon_image, 0, 1);
		else if (i == 3)
			result = Draw_load_texture("romfs:/gfx/" + texture_name_list[i] + ".t3x", i, Square_image, 0, 1);
		else if (i == 4)
			result = Draw_load_texture("romfs:/gfx/" + texture_name_list[i] + ".t3x", i, ui_image, 0, 4);

		Log_log_add(log_num, result.string, result.code, FORCE_DEBUG);
	}

	svcSetThreadPriority(CUR_THREAD_HANDLE, PRIORITY_HIGH - 1);
	Log_log_save(main_init_string , "Initialized.", 1234567890, false);
}

int main()
{
	Init();
	Menu_init();

	// Main loop
	while (aptMainLoop())
	{
		Hid_set_disable_flag(false);
		Draw_set_do_not_draw_flag(false);

		if (Menu_query_must_exit_flag())
			break;

		Menu_main();

		Hid_set_disable_flag(true);
		Draw_set_do_not_draw_flag(true);
	}

	Menu_exit();
	Hid_exit();
	Expl_exit();

	for (int i = 0; i < 10; i++)
	{
		Log_log_save(main_exit_string, main_svc_name_list[i] + "Exit()...", 1234567890, FORCE_DEBUG);

		if (i == 0)
			fsExit();
		else if (i == 1)
			acExit();
		else if (i == 2)
			aptExit();
		else if (i == 3)
			mcuHwcExit();
		else if (i == 4)
			ptmuExit();
		else if (i == 5)
			httpcExit();
		else if (i == 6)
			romfsExit();
		else if (i == 7)
			cfguExit();
		else if (i == 8)
			amExit();
		else if(i == 9)
			newsExit();
	}

	Draw_exit();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}
