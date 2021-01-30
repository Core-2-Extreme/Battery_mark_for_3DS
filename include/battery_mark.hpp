#pragma once

#define BMARK_NUM_OF_MSG 29

bool Bmark_query_init_flag(void);

bool Bmark_query_running_flag(void);

Result_with_string Bmark_load_msg(std::string lang);

void Bmark_resume(void);

void Bmark_suspend(void);

void Bmark_init(void);

void Bmark_exit(void);

void Bmark_main(void);
