#pragma once

/**
 * @brief Initialize curl api.
 * @param buffer_size Internal buffer size used by post request.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @warning Thread dangerous (untested)
*/
Result_with_string Util_curl_init(int buffer_size);

/**
 * @brief Uninitialize curl API.
 * Do nothing if curl api is not initialized.
 * @warning Thread dangerous (untested)
*/
void Util_curl_exit();

/**
 * @brief Make a http get request.
 * @param url (in) URL.
 * @param data (out) Pointer for response data, the pointer will be allocated up to max_size 
 * depends on server response.
 * @param max_size (in) Max download size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_dl_data(std::string url, u8** data, int max_size, int* downloaded_size, bool follow_redirect,
int max_redirect);

/**
 * @brief Make a HTTP get request.
 * @param url (in) URL.
 * @param data (out) Pointer for response data, the pointer will be allocated up to max_size 
 * depends on server response.
 * @param max_size (in) Max download size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param last_url (out) Last url (data contains response of last url).
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_dl_data(std::string url, u8** data, int max_size, int* downloaded_size, bool follow_redirect,
int max_redirect, std::string* last_url);

/**
 * @brief Make a HTTP get request.
 * @param url (in) URL.
 * @param data (out) Pointer for response data, the pointer will be allocated up to max_size 
 * depends on server response.
 * @param max_size (in) Max download size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param status_code (out) HTTP status code.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_dl_data(std::string url, u8** data, int max_size, int* downloaded_size, int* status_code, bool follow_redirect,
int max_redirect);

/**
 * @brief Make a HTTP get request.
 * @param url (in) URL.
 * @param data (out) Pointer for response data, the pointer will be allocated up to max_size 
 * depends on server response.
 * @param max_size (in) Max download size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param status_code (out) HTTP status code.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param last_url (out) Last url (data contains response of last url).
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_dl_data(std::string url, u8** data, int max_size, int* downloaded_size, int* status_code, bool follow_redirect,
int max_redirect, std::string* last_url);

/**
 * @brief Make a HTTP get request and save response to SD card.
 * @param url (in) URL.
 * @param buffer_size (in) Internal work buffer size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param dir_path (in) Directory path.
 * @param file_name (in) File name if file already exist, the file will be overwritten.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_save_data(std::string url, int buffer_size, int* downloaded_size, bool follow_redirect,
int max_redirect, std::string dir_path, std::string file_name);

/**
 * @brief Make a HTTP get request and save response to SD card.
 * @param url (in) URL.
 * @param buffer_size (in) Internal work buffer size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param last_url (out) Last url (response of last url will be saved).
 * @param dir_path (in) Directory path.
 * @param file_name (in) File name if file already exist, the file will be overwritten.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_save_data(std::string url, int buffer_size, int* downloaded_size, bool follow_redirect,
int max_redirect, std::string* last_url, std::string dir_path, std::string file_name);

/**
 * @brief Make a HTTP get request and save response to SD card.
 * @param url (in) URL.
 * @param buffer_size (in) Internal work buffer size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param status_code (out) HTTP status code.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param dir_path (in) Directory path.
 * @param file_name (in) File name if file already exist, the file will be overwritten.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_save_data(std::string url, int buffer_size, int* downloaded_size, int* status_code, bool follow_redirect,
int max_redirect, std::string dir_path, std::string file_name);

/**
 * @brief Make a HTTP get request and save response to SD card.
 * @param url (in) URL.
 * @param buffer_size (in) Internal work buffer size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param status_code (out) HTTP status code.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param last_url (out) Last url (response of last url will be saved).
 * @param dir_path (in) Directory path.
 * @param file_name (in) File name if file already exist, the file will be overwritten.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_save_data(std::string url, int buffer_size, int* downloaded_size, int* status_code, bool follow_redirect,
int max_redirect, std::string* last_url, std::string dir_path, std::string file_name);

/**
 * @brief Make a HTTP post request.
 * @param url (in) URL.
 * @param post_data (in) Pointer for post data.
 * @param post_size (in) Post data size.
 * @param dl_data (out) Pointer for response data, the pointer will be allocated up to max_size 
 * depends on server response.
 * @param max_dl_size (in) Max download size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_post_and_dl_data(std::string url, u8* post_data, int post_size, u8** dl_data, int max_dl_size,
int* downloaded_size, bool follow_redirect, int max_redirect);

/**
 * @brief Make a HTTP post request.
 * @param url (in) URL.
 * @param post_data (in) Pointer for post data.
 * @param post_size (in) Post data size.
 * @param dl_data (out) Pointer for response data, the pointer will be allocated up to max_size 
 * depends on server response.
 * @param max_dl_size (in) Max download size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param last_url (out) Last url (dl_data contains response of last url).
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_post_and_dl_data(std::string url, u8* post_data, int post_size, u8** dl_data, int max_dl_size,
int* downloaded_size, bool follow_redirect, int max_redirect, std::string* last_url);

/**
 * @brief Make a HTTP post request.
 * @param url (in) URL.
 * @param post_data (in) Pointer for post data.
 * @param post_size (in) Post data size.
 * @param dl_data (out) Pointer for response data, the pointer will be allocated up to max_size 
 * depends on server response.
 * @param max_dl_size (in) Max download size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param status_code (out) HTTP status code.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_post_and_dl_data(std::string url, u8* post_data, int post_size, u8** dl_data, int max_dl_size,
int* downloaded_size, int* status_code, bool follow_redirect, int max_redirect);

/**
 * @brief Make a HTTP post request.
 * @param url (in) URL.
 * @param post_data (in) Pointer for post data.
 * @param post_size (in) Post data size.
 * @param dl_data (out) Pointer for response data, the pointer will be allocated up to max_size 
 * depends on server response.
 * @param max_dl_size (in) Max download size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param status_code (out) HTTP status code.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param last_url (out) Last url (dl_data contains response of last url).
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_post_and_dl_data(std::string url, u8* post_data, int post_size, u8** dl_data, int max_dl_size,
int* downloaded_size, int* status_code, bool follow_redirect, int max_redirect, std::string* last_url);

/**
 * @brief Make a HTTP post request and save response to SD card.
 * @param url (in) URL.
 * @param post_data (in) Pointer for post data.
 * @param post_size (in) Post data size.
 * @param buffer_size (in) Internal work buffer size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param dir_path (in) Directory path.
 * @param file_name (in) File name if file already exist, the file will be overwritten.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_post_and_save_data(std::string url, u8* post_data, int post_size, int buffer_size, int* downloaded_size,
bool follow_redirect, int max_redirect, std::string dir_path, std::string file_name);

/**
 * @brief Make a HTTP post request and save response to SD card.
 * @param url (in) URL.
 * @param post_data (in) Pointer for post data.
 * @param post_size (in) Post data size.
 * @param buffer_size (in) Internal work buffer size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param last_url (out) Last url (response of last url will be saved).
 * @param dir_path (in) Directory path.
 * @param file_name (in) File name if file already exist, the file will be overwritten.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_post_and_save_data(std::string url, u8* post_data, int post_size, int buffer_size, int* downloaded_size,
bool follow_redirect, int max_redirect, std::string* last_url, std::string dir_path, std::string file_name);

/**
 * @brief Make a HTTP post request and save response to SD card.
 * @param url (in) URL.
 * @param post_data (in) Pointer for post data.
 * @param post_size (in) Post data size.
 * @param buffer_size (in) Internal work buffer size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param status_code (out) HTTP status code.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param dir_path (in) Directory path.
 * @param file_name (in) File name if file already exist, the file will be overwritten.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_post_and_save_data(std::string url, u8* post_data, int post_size, int buffer_size, int* downloaded_size,
int* status_code, bool follow_redirect, int max_redirect, std::string dir_path, std::string file_name);

/**
 * @brief Make a HTTP post request and save response to SD card.
 * @param url (in) URL.
 * @param post_data (in) Pointer for post data.
 * @param post_size (in) Post data size.
 * @param buffer_size (in) Internal work buffer size.
 * @param downloaded_size (out) Actuall downloaded size.
 * @param status_code (out) HTTP status code.
 * @param follow_redirect (in) When true, follow redirect up to max_redirect.
 * @param max_redirect (in) Max redirect.
 * @param last_url (out) Last url (response of last url will be saved).
 * @param dir_path (in) Directory path.
 * @param file_name (in) File name if file already exist, the file will be overwritten.
 * @return On success DEF_SUCCESS, 
 * on failure DEF_ERR_* or Nintendo API's error.
 * @note Thread safe
*/
Result_with_string Util_curl_post_and_save_data(std::string url, u8* post_data, int post_size, int buffer_size, int* downloaded_size,
int* status_code, bool follow_redirect, int max_redirect, std::string* last_url, std::string dir_path, std::string file_name);
