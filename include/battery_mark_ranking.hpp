#pragma once

#define BMR_NUM_OF_MSG 17

#define BMR_NUM_OF_LOGS 1000

bool Bmr_query_init_flag(void);

bool Bmr_query_running_flag(void);

Result_with_string Bmr_load_msg(std::string lang);

void Bmr_resume(void);

void Bmr_suspend(void);

void Bmr_init(void);

void Bmr_exit(void);

void Bmr_main(void);
