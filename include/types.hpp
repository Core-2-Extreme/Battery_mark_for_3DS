#pragma once

#define STACKSIZE (64 * 1024)
#define INACTIVE_THREAD_SLEEP_TIME 100000
#define ACTIVE_THREAD_SLEEP_TIME 50000
 //0x18~0x3F
#define PRIORITY_IDLE 0x36
#define PRIORITY_LOW 0x25
#define PRIORITY_NORMAL 0x24
#define PRIORITY_HIGH 0x23
#define PRIORITY_REALTIME 0x18
#define FORCE_DEBUG false

#define MENU_HTTP_PORT0 0
#define SEM_HTTP_PORT0 1
#define BMR_HTTP_PORT0 2
#define MENU_HTTP_POST_PORT0 0
#define BMARK_HTTP_POST_PORT0 1

struct Result_with_string
{
	std::string string = "[Success] ";
	std::string error_description = "";
	Result code = 0;
};

struct Hid_info
{
	bool p_a = false;
	bool p_b = false;
	bool p_x = false;
	bool p_y = false;
	bool p_c_up = false;
	bool p_c_down = false;
	bool p_c_left = false;
	bool p_c_right = false;
	bool p_d_up = false;
	bool p_d_down = false;
	bool p_d_left = false;
	bool p_d_right = false;
	bool p_l = false;
	bool p_r = false;
	bool p_zl = false;
	bool p_zr = false;
	bool p_start = false;
	bool p_select = false;
	bool p_cs_up = false;
	bool p_cs_down = false;
	bool p_cs_left = false;
	bool p_cs_right = false;
	bool p_touch = false;
	bool h_a = false;
	bool h_b = false;
	bool h_x = false;
	bool h_y = false;
	bool h_c_up = false;
	bool h_c_down = false;
	bool h_c_left = false;
	bool h_c_right = false;
	bool h_d_up = false;
	bool h_d_down = false;
	bool h_d_left = false;
	bool h_d_right = false;
	bool h_l = false;
	bool h_r = false;
	bool h_zl = false;
	bool h_zr = false;
	bool h_start = false;
	bool h_select = false;
	bool h_cs_up = false;
	bool h_cs_down = false;
	bool h_cs_left = false;
	bool h_cs_right = false;
	bool h_touch = false;
	int cpad_x = 0;
	int cpad_y = 0;
	int touch_x = 0;
	int touch_y = 0;
	int touch_x_move = 0;
	int touch_y_move = 0;
	int held_time = 0;
	int count = 0;
};
