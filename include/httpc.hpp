#pragma once
#include "types.hpp"

int Httpc_query_dl_progress(int port);

int Httpc_query_dled_size(int port);

int Httpc_query_post_and_dl_progress(int port);

void Httpc_reset_dl_progress(int port);

void Httpc_reset_post_and_dl_progress(int port);

Result_with_string Httpc_dl_data(std::string url, u8* data_buffer, int buffer_size, u32* downloaded_data_size, u32* status_code, bool follow_redirect, std::string* last_url, bool do_not_dl, int max_redirect, int port);

Result_with_string Httpc_dl_data(std::string url, u8* data_buffer, int buffer_size, u32* downloaded_data_size, u32* status_code, bool follow_redirect, std::string* last_url, bool do_not_dl, int max_redirect, int port, std::string dir_path, std::string file_path);

Result_with_string Httpc_post_and_dl_data(std::string url, char* post_data_buffer, int post_buffer_size, u8* dl_data_buffer, int dl_buffer_size, u32* downloaded_data_size, u32* status_code, bool follow_redirect, int port);
