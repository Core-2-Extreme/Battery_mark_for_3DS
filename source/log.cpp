#include <3ds.h>
#include <string>

#include "headers.hpp"

bool log_previous_show_logs = false;
bool log_show_logs = false;
int log_current_log_num = 0;
int log_previous_y = -1;
int log_y = 0;
double log_previous_x = 0.0;
double log_x = 0.0;
double log_up_time_ms = 0.0;
double log_spend_time[512];
std::string log_logs[512];
std::string log_previous_logs[512];
TickCounter log_up_time_timer;

void Log_start_up_time_timer(void)
{
	osTickCounterStart(&log_up_time_timer);
}

double Log_query_x(void)
{
	return log_x;
}

int Log_query_y(void)
{
	return log_y;
}

bool Log_query_log_show_flag(void)
{
	return log_show_logs;
}

std::string Log_query_log(int log_num)
{
	if (log_num >= 0 && log_num <= 511)
		return log_logs[log_num];
	else
		return "";
}

bool Log_query_need_reflesh(void)
{
	bool need = false;

	for(int i = 0; i < 512; i++)
	{
		if(log_previous_logs[i] != log_logs[i])
		{
			need = true;
			break;
		}
	}

	if(need || log_previous_x != log_x || log_previous_y != log_y || log_previous_show_logs != log_show_logs)
	{
		for(int i = 0; i < 512; i++)
			log_previous_logs[i] = log_logs[i];

		log_previous_x = log_x;
		log_previous_y = log_y;
		log_previous_show_logs = log_show_logs;
		return true;
	}
	else
		return false;
}

void Log_set_x(double value)
{
	log_x = value;
}

void Log_set_y(int value)
{
	log_y = value;
}

void Log_set_log_show_flag(bool flag)
{
	log_show_logs = flag;
}

int Log_log_save(std::string type, std::string text)
{
	return Log_log_save(type, text, 1234567890, false);
}

int Log_log_save(std::string type, std::string text, Result result)
{
	return Log_log_save(type, text, result, false);
}

int Log_log_save(std::string type, std::string text, Result result, bool draw)
{
	double time_cache;
	int return_log_num;
	char app_log_cache[4096];
	osTickCounterUpdate(&log_up_time_timer);
	time_cache = osTickCounterRead(&log_up_time_timer);
	log_up_time_ms = log_up_time_ms + time_cache;
	log_spend_time[log_current_log_num] = log_up_time_ms;
	time_cache = log_up_time_ms / 1000;
	memset(app_log_cache, 0x0, 4096);

	if (result == 1234567890)
		sprintf(app_log_cache, "[%.5f][%s] %s", time_cache, type.c_str(), text.c_str());
	else
		sprintf(app_log_cache, "[%.5f][%s] %s0x%lx", time_cache, type.c_str(), text.c_str(), result);

	log_logs[log_current_log_num] = app_log_cache;
	log_current_log_num++;
	return_log_num = log_current_log_num;
	if (log_current_log_num >= 512)
		log_current_log_num = 0;

	if (log_current_log_num < 23)
		log_y = 0;
	else
		log_y = log_current_log_num - 23;

	if (draw)
		Draw_log(true);

	return (return_log_num - 1);
}

void Log_log_add(int add_log_num, std::string add_text)
{
	Log_log_add(add_log_num, add_text, 1234567890, false);
}

void Log_log_add(int add_log_num, std::string add_text, Result result)
{
	Log_log_add(add_log_num, add_text, result, false);
}

void Log_log_add(int add_log_num, std::string add_text, Result result, bool draw)
{
	double time_cache;
	char app_log_add_cache[4096];
	osTickCounterUpdate(&log_up_time_timer);
	time_cache = osTickCounterRead(&log_up_time_timer);
	log_up_time_ms = log_up_time_ms + time_cache;
	time_cache = log_up_time_ms - log_spend_time[add_log_num];
	memset(app_log_add_cache, 0x0, 4096);

	if (result != 1234567890)
		sprintf(app_log_add_cache, "%s0x%lx (%.2fms)", add_text.c_str(), result, time_cache);
	else
		sprintf(app_log_add_cache, "%s (%.2fms)", add_text.c_str(), time_cache);

	if (draw)
		Draw_log(true);

	log_logs[add_log_num] += app_log_add_cache;
}

void Log_main(void)
{
	Hid_info key;
	Hid_query_key_state(&key);

	if(log_show_logs)
	{
		if (key.h_c_up)
		{
				if (log_y - 1 > 0)
					log_y--;
		}
		if (key.h_c_down)
		{
				if (log_y + 1 < 512)
					log_y++;
		}
		if (key.h_c_left)
		{
				if (log_x + 5.0 < 0.0)
					log_x += 5.0;
				else
					log_x = 0.0;
		}
		if (key.h_c_right)
		{
				if (log_x - 5.0 > -1000.0)
					log_x -= 5.0;
				else
					log_x = -1000.0;
		}
	}
}
