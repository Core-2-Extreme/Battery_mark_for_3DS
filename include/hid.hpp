#pragma once
#include "types.hpp"

void Hid_init(void);

void Hid_exit(void);

bool Hid_query_disable_flag(void);

void Hid_query_key_state(Hid_info* out_key_state);

void Hid_set_disable_flag(bool flag);

void Hid_key_flag_reset(void);

void Hid_scan_hid_thread(void* arg);
