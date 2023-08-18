#ifndef VARIABLES_HPP
#define VARIABLES_HPP

#include "system/types.hpp"

extern bool var_connect_test_succes;
extern bool var_need_reflesh;
extern bool var_allow_send_app_info;
extern bool var_night_mode;
extern bool var_eco_mode;
extern bool var_debug_mode;
extern bool var_flash_mode;
extern bool var_wifi_enabled;
extern bool var_monitor_cpu_usage;
extern bool var_turn_on_top_lcd;
extern bool var_turn_on_bottom_lcd;
extern bool var_core_2_available;
extern bool var_core_3_available;
extern bool var_fake_model;
extern bool var_debug_bool[8];
extern u8 var_wifi_state;
extern u8 var_wifi_signal;
extern u8 var_battery_charge;
extern u8 var_model;
extern u8 var_screen_mode;
extern int var_hours;
extern int var_minutes;
extern int var_seconds;
extern int var_days;
extern int var_months;
extern int var_years;
extern int var_battery_level_raw;
extern int var_battery_temp;
extern int var_afk_time;
extern int var_free_ram;
extern int var_free_linear_ram;
extern int var_lcd_brightness;
extern int var_top_lcd_brightness;
extern int var_bottom_lcd_brightness;
extern int var_time_to_turn_off_lcd;
extern int var_num_of_app_start;
extern int var_system_region;
extern int var_debug_int[8];
extern double var_scroll_speed;
extern double var_battery_voltage;
extern double var_debug_double[8];
extern char var_status[128];
extern std::string var_clipboard;
extern std::string var_connected_ssid;
extern std::string var_lang;
extern std::string var_model_name[6];
extern std::string var_debug_string[8];
extern C2D_Image var_square_image[1];
extern C2D_Image var_null_image;
extern Result_with_string var_disabled_result;

#endif
