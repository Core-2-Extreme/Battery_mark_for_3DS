#pragma once
#include "system/types.hpp"

bool Bmark_query_init_flag(void);

bool Bmark_query_running_flag(void);

Result_with_string Bmark_load_msg(std::string lang);

void Bmark_resume(void);

void Bmark_suspend(void);

void Bmark_hid(Hid_info key);

void Bmark_init(bool draw);

void Bmark_exit(bool draw);

void Bmark_main(void);
