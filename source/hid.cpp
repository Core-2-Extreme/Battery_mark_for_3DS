#include <3ds.h>
#include <string>
#include <unistd.h>

#include "headers.hpp"

#include "menu.hpp"
#include "setting_menu.hpp"

bool hid_scan_hid_thread_run = false;
bool hid_key_A_press = false;
bool hid_key_B_press = false;
bool hid_key_X_press = false;
bool hid_key_Y_press = false;
bool hid_key_C_UP_press = false;
bool hid_key_C_RIGHT_press = false;
bool hid_key_C_DOWN_press = false;
bool hid_key_C_LEFT_press = false;
bool hid_key_D_UP_press = false;
bool hid_key_D_RIGHT_press = false;
bool hid_key_D_DOWN_press = false;
bool hid_key_D_LEFT_press = false;
bool hid_key_L_press = false;
bool hid_key_R_press = false;
bool hid_key_ZL_press = false;
bool hid_key_ZR_press = false;
bool hid_key_START_press = false;
bool hid_key_SELECT_press = false;
bool hid_key_CS_UP_press = false;
bool hid_key_CS_DOWN_press = false;
bool hid_key_CS_RIGHT_press = false;
bool hid_key_CS_LEFT_press = false;
bool hid_key_touch_press = false;
bool hid_key_A_held = false;
bool hid_key_B_held = false;
bool hid_key_X_held = false;
bool hid_key_Y_held = false;
bool hid_key_C_UP_held = false;
bool hid_key_C_DOWN_held = false;
bool hid_key_C_RIGHT_held = false;
bool hid_key_C_LEFT_held = false;
bool hid_key_D_UP_held = false;
bool hid_key_D_DOWN_held = false;
bool hid_key_D_RIGHT_held = false;
bool hid_key_D_LEFT_held = false;
bool hid_key_L_held = false;
bool hid_key_R_held = false;
bool hid_key_ZL_held = false;
bool hid_key_ZR_held = false;
bool hid_key_START_held = false;
bool hid_key_SELECT_held = false;
bool hid_key_CS_UP_held = false;
bool hid_key_CS_DOWN_held = false;
bool hid_key_CS_RIGHT_held = false;
bool hid_key_CS_LEFT_held = false;
bool hid_key_touch_held = false;
bool hid_disabled = false;
int hid_cpad_pos_x = 0;
int hid_cpad_pos_y = 0;
int hid_touch_pos_x = 0;
int hid_touch_pos_x_before = 0;
int hid_touch_pos_x_moved = 0;
int hid_touch_pos_y = 0;
int hid_touch_pos_y_before = 0;
int hid_touch_pos_y_moved = 0;
int hid_held_time = 0;
int hid_count = 0;
std::string hid_scan_hid_thread_string = "Hid/Scan hid thread";
Thread hid_scan_hid_thread;

void Hid_init(void)
{
	hid_scan_hid_thread_run = true;
	hid_scan_hid_thread = threadCreate(Hid_scan_hid_thread, (void*)(""), STACKSIZE, PRIORITY_REALTIME, -1, false);
}

void Hid_exit(void)
{
	hid_scan_hid_thread_run = false;
	threadJoin(hid_scan_hid_thread, 10000000000);
	threadFree(hid_scan_hid_thread);
}

bool Hid_query_disable_flag(void)
{
	return hid_disabled;
}

void Hid_query_key_state(Hid_info* out_key_state)
{
	out_key_state->p_a = hid_key_A_press;
	out_key_state->p_b = hid_key_B_press;
	out_key_state->p_x = hid_key_X_press;
	out_key_state->p_y = hid_key_Y_press;
	out_key_state->p_c_up = hid_key_C_UP_press;
	out_key_state->p_c_down = hid_key_C_DOWN_press;
	out_key_state->p_c_left = hid_key_C_LEFT_press;
	out_key_state->p_c_right = hid_key_C_RIGHT_press;
	out_key_state->p_d_up = hid_key_D_UP_press;
	out_key_state->p_d_down = hid_key_D_DOWN_press;
	out_key_state->p_d_left = hid_key_D_LEFT_press;
	out_key_state->p_d_right = hid_key_D_RIGHT_press;
	out_key_state->p_l = hid_key_L_press;
	out_key_state->p_r = hid_key_R_press;
	out_key_state->p_zl = hid_key_ZL_press;
	out_key_state->p_zr = hid_key_ZR_press;
	out_key_state->p_start = hid_key_START_press;
	out_key_state->p_select = hid_key_SELECT_press;
	out_key_state->p_cs_up = hid_key_CS_UP_press;
	out_key_state->p_cs_down = hid_key_CS_DOWN_press;
	out_key_state->p_cs_left = hid_key_CS_LEFT_press;
	out_key_state->p_cs_right = hid_key_CS_RIGHT_press;
	out_key_state->p_touch = hid_key_touch_press;
	out_key_state->h_a = hid_key_A_held;
	out_key_state->h_b = hid_key_B_held;
	out_key_state->h_x = hid_key_X_held;
	out_key_state->h_y = hid_key_Y_held;
	out_key_state->h_c_up = hid_key_C_UP_held;
	out_key_state->h_c_down = hid_key_C_DOWN_held;
	out_key_state->h_c_left = hid_key_C_LEFT_held;
	out_key_state->h_c_right = hid_key_C_RIGHT_held;
	out_key_state->h_d_up = hid_key_D_UP_held;
	out_key_state->h_d_down = hid_key_D_DOWN_held;
	out_key_state->h_d_left = hid_key_D_LEFT_held;
	out_key_state->h_d_right = hid_key_D_RIGHT_held;
	out_key_state->h_l = hid_key_L_held;
	out_key_state->h_r = hid_key_R_held;
	out_key_state->h_zl = hid_key_ZL_held;
	out_key_state->h_zr = hid_key_ZR_held;
	out_key_state->h_start = hid_key_START_held;
	out_key_state->h_select = hid_key_SELECT_held;
	out_key_state->h_cs_up = hid_key_CS_UP_held;
	out_key_state->h_cs_down = hid_key_CS_DOWN_held;
	out_key_state->h_cs_left = hid_key_CS_LEFT_held;
	out_key_state->h_cs_right = hid_key_CS_RIGHT_held;
	out_key_state->h_touch = hid_key_touch_held;
	out_key_state->cpad_x = hid_cpad_pos_x;
	out_key_state->cpad_y = hid_cpad_pos_y;
	out_key_state->touch_x = hid_touch_pos_x;
	out_key_state->touch_y = hid_touch_pos_y;
	out_key_state->touch_x_move = hid_touch_pos_x_moved;
	out_key_state->touch_y_move = hid_touch_pos_y_moved;
	out_key_state->held_time = hid_held_time;
	out_key_state->count = hid_count;
}

void Hid_key_flag_reset(void)
{
	hid_key_A_press = false;
	hid_key_B_press = false;
	hid_key_X_press = false;
	hid_key_Y_press = false;
	hid_key_C_UP_press = false;
	hid_key_C_DOWN_press = false;
	hid_key_C_RIGHT_press = false;
	hid_key_C_LEFT_press = false;
	hid_key_D_UP_press = false;
	hid_key_D_DOWN_press = false;
	hid_key_D_RIGHT_press = false;
	hid_key_D_LEFT_press = false;
	hid_key_L_press = false;
	hid_key_R_press = false;
	hid_key_ZL_press = false;
	hid_key_ZR_press = false;
	hid_key_START_press = false;
	hid_key_SELECT_press = false;
	hid_key_CS_UP_press = false;
	hid_key_CS_DOWN_press = false;
	hid_key_CS_RIGHT_press = false;
	hid_key_CS_LEFT_press = false;
	hid_key_touch_press = false;
	hid_key_A_held = false;
	hid_key_B_held = false;
	hid_key_X_held = false;
	hid_key_Y_held = false;
	hid_key_C_UP_held = false;
	hid_key_C_DOWN_held = false;
	hid_key_C_RIGHT_held = false;
	hid_key_C_LEFT_held = false;
	hid_key_D_UP_held = false;
	hid_key_D_DOWN_held = false;
	hid_key_D_RIGHT_held = false;
	hid_key_D_LEFT_held = false;
	hid_key_L_held = false;
	hid_key_R_held = false;
	hid_key_ZL_held = false;
	hid_key_ZR_held = false;
	hid_key_START_held = false;
	hid_key_SELECT_held = false;
	hid_key_CS_UP_held = false;
	hid_key_CS_DOWN_held = false;
	hid_key_CS_RIGHT_held = false;
	hid_key_CS_LEFT_held = false;
	hid_key_touch_held = false;
	hid_touch_pos_x = 0;
	hid_touch_pos_y = 0;
	hid_count = 0;
}

void Hid_set_disable_flag(bool flag)
{
	hid_disabled = flag;
}

void Hid_scan_hid_thread(void* arg)
{
	Log_log_save(hid_scan_hid_thread_string, "Thread started.", 1234567890, false);

	u32 kDown;
	u32 kHeld;
	touchPosition touch_pos;
	circlePosition circle_pos;
	Result_with_string result;

	while (hid_scan_hid_thread_run)
	{
		hidScanInput();
		hidTouchRead(&touch_pos);
		hidCircleRead(&circle_pos);
		kHeld = hidKeysHeld();
		kDown = hidKeysDown();

		if (kDown & KEY_A)
			hid_key_A_press = true;
		if (kDown & KEY_B)
			hid_key_B_press = true;
		if (kDown & KEY_X)
			hid_key_X_press = true;
		if (kDown & KEY_Y)
			hid_key_Y_press = true;
		if (kDown & KEY_DUP)
			hid_key_D_UP_press = true;
		if (kDown & KEY_DDOWN)
			hid_key_D_DOWN_press = true;
		if (kDown & KEY_DRIGHT)
			hid_key_D_RIGHT_press = true;
		if (kDown & KEY_DLEFT)
			hid_key_D_LEFT_press = true;
		if (kDown & KEY_CPAD_UP)
			hid_key_C_UP_press = true;
		if (kDown & KEY_CPAD_DOWN)
			hid_key_C_DOWN_press = true;
		if (kDown & KEY_CPAD_RIGHT)
			hid_key_C_RIGHT_press = true;
		if (kDown & KEY_CPAD_LEFT)
			hid_key_C_LEFT_press = true;
		if (kDown & KEY_SELECT)
			hid_key_SELECT_press = true;
		if (kDown & KEY_START)
			hid_key_START_press = true;
		if (kDown & KEY_L)
			hid_key_L_press = true;
		if (kDown & KEY_R)
			hid_key_R_press = true;
		if (kDown & KEY_ZL)
			hid_key_ZL_press = true;
		if (kDown & KEY_ZR)
			hid_key_ZR_press = true;
		if (kHeld & KEY_DRIGHT)
			hid_key_D_RIGHT_held = true;
		if (kHeld & KEY_DLEFT)
			hid_key_D_LEFT_held = true;
		if (kHeld & KEY_DDOWN)
			hid_key_D_DOWN_held = true;
		if (kHeld & KEY_DUP)
			hid_key_D_UP_held = true;
		if (kHeld & KEY_A)
			hid_key_A_held = true;
		if (kHeld & KEY_B)
			hid_key_B_held = true;
		if (kHeld & KEY_Y)
			hid_key_Y_held = true;
		if (kHeld & KEY_X)
			hid_key_X_held = true;
		if (kHeld & KEY_CPAD_UP)
			hid_key_C_UP_held = true;
		if (kHeld & KEY_CPAD_DOWN)
			hid_key_C_DOWN_held = true;
		if (kHeld & KEY_CPAD_RIGHT)
			hid_key_C_RIGHT_held = true;
		if (kHeld & KEY_CPAD_LEFT)
			hid_key_C_LEFT_held = true;
		if (kHeld & KEY_L)
			hid_key_L_held = true;
		if (kHeld & KEY_R)
			hid_key_R_held = true;
		if (kDown & KEY_ZL)
			hid_key_ZL_held = true;
		if (kDown & KEY_ZR)
			hid_key_ZR_held = true;
		if (kDown & KEY_TOUCH || kHeld & KEY_TOUCH)
		{
			if (kDown & KEY_TOUCH)
			{
				hid_key_touch_press = true;
				hid_touch_pos_x_before = touch_pos.px;
				hid_touch_pos_y_before = touch_pos.py;
				hid_touch_pos_x = touch_pos.px;
				hid_touch_pos_y = touch_pos.py;
			}
			if (kHeld & KEY_TOUCH)
			{
				hid_key_touch_held = true;
				hid_touch_pos_x = touch_pos.px;
				hid_touch_pos_y = touch_pos.py;
				hid_touch_pos_x_moved = hid_touch_pos_x_before - hid_touch_pos_x;
				hid_touch_pos_y_moved = hid_touch_pos_y_before - hid_touch_pos_y;
				hid_touch_pos_x_before = touch_pos.px;
				hid_touch_pos_y_before = touch_pos.py;
			}
		}
		else
		{
			hid_touch_pos_x_moved = 0;
			hid_touch_pos_y_moved = 0;
			hid_touch_pos_x_before = 0;
			hid_touch_pos_y_before = 0;
		}

		hid_cpad_pos_x = circle_pos.dx;
		hid_cpad_pos_y = circle_pos.dy;

		if (hid_key_X_held && hid_key_Y_press)
			Sem_set_settings(SEM_SYSTEM_SETTING_MENU_SHOW, !Sem_query_settings(SEM_SYSTEM_SETTING_MENU_SHOW));

		if (Sem_query_settings(SEM_SYSTEM_SETTING_MENU_SHOW))
		{
			if (hid_key_touch_held)
			{
				if (hid_touch_pos_x > 0 && hid_touch_pos_x < 300 && hid_touch_pos_y > 0 && hid_touch_pos_y < 30)
				{
					Sem_set_settings_i(SEM_LCD_BRIGHTNESS, ((hid_touch_pos_x + 20) / 2));
					Menu_set_operation_flag(MENU_CHANGE_BRIGHTNESS_REQUEST, true);
				}
			}
			if (hid_key_touch_press)
			{
				if (hid_touch_pos_x > 300 && hid_touch_pos_x < 320 && hid_touch_pos_y > 0 && hid_touch_pos_y < 30)
						Sem_set_operation_flag(SEM_CHANGE_WIFI_STATE_REQUEST, true);
			}
		}

		if (hid_key_A_press || hid_key_B_press || hid_key_X_press || hid_key_Y_press || hid_key_D_RIGHT_press
			|| hid_key_D_LEFT_press || hid_key_ZL_press || hid_key_ZR_press || hid_key_START_press
			|| hid_key_SELECT_press || hid_key_touch_press || hid_key_A_held || hid_key_B_held
			|| hid_key_X_held || hid_key_Y_held || hid_key_D_DOWN_held || hid_key_D_RIGHT_held
			|| hid_key_D_LEFT_held || hid_key_C_UP_held || hid_key_C_DOWN_held || hid_key_C_RIGHT_held
			|| hid_key_C_LEFT_held || hid_key_D_UP_held || hid_key_touch_held)
		{
			if (Menu_query_afk_time() > Sem_query_settings_i(SEM_TIME_TO_TURN_OFF_LCD) * 2)
				Menu_set_operation_flag(MENU_CHANGE_BRIGHTNESS_REQUEST, true);

			Menu_reset_afk_time();
		}
		if (hid_key_D_UP_held || hid_key_D_DOWN_held || hid_key_D_RIGHT_held || hid_key_D_LEFT_held
			|| hid_key_C_UP_held || hid_key_C_DOWN_held || hid_key_C_RIGHT_held || hid_key_C_LEFT_held
			|| hid_key_CS_UP_held || hid_key_CS_DOWN_held || hid_key_CS_RIGHT_held || hid_key_CS_LEFT_held
			|| hid_key_touch_held)
			hid_held_time++;
		else
			hid_held_time = 0;

		hid_count++;
		if(hid_disabled)
			Hid_key_flag_reset();

		gspWaitForVBlank();
	}
	Log_log_save(hid_scan_hid_thread_string, "Thread exit", 1234567890, false);
	threadExit(0);
}
