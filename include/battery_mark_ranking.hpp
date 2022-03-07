#pragma once
#include "system/types.hpp"

bool Bmr_query_init_flag(void);

bool Bmr_query_running_flag(void);

Result_with_string Bmr_load_msg(std::string lang);

void Bmr_resume(void);

void Bmr_suspend(void);

void Bmr_hid(Hid_info key);

void Bmr_init(bool draw);

void Bmr_exit(bool draw);

void Bmr_main(void);
