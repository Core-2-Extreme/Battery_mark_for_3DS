#pragma once

void Log_start_up_time_timer(void);

double Log_query_x(void);

int Log_query_y(void);

bool Log_query_log_show_flag(void);

std::string Log_query_log(int log_num);

bool Log_query_need_reflesh(void);

void Log_set_x(double value);

void Log_set_y(int value);

void Log_set_log_show_flag(bool flag);

int Log_log_save(std::string type, std::string text);

int Log_log_save(std::string type, std::string text, Result result);

int Log_log_save(std::string type, std::string text, Result result, bool draw);

void Log_log_add(int add_log_num, std::string add_text);

void Log_log_add(int add_log_num, std::string add_text, Result result);

void Log_log_add(int add_log_num, std::string add_text, Result result, bool draw);

void Log_main(void);
