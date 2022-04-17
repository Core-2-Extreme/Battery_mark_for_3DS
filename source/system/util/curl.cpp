#include "system/headers.hpp"
#include "curl/curl.h"

bool util_curl_init = false;
u32* util_curl_buffer = NULL;

struct Http_data
{
	int max_size = 0;
	int current_size = 0;
	int* used_size = NULL;
	u8* data = NULL;
	std::string file_name = "";
	std::string dir_path = "";
};

Result_with_string Util_curl_init(int buffer_size)
{
	Result_with_string result;
	if(util_curl_init)
		goto already_inited;

	if(buffer_size < 0)
		goto invalid_arg;

	util_curl_buffer = (u32*)__real_memalign(0x1000, buffer_size);
	if(!util_curl_buffer)
		goto out_of_memory;

	result.code = socInit(util_curl_buffer, buffer_size);
	if(result.code != 0)
	{
		result.error_description = "[Error] socInit() failed. ";
		goto nintendo_api_failed;
	}

	util_curl_init = true;
	return result;

	already_inited:
	result.code = DEF_ERR_ALREADY_INITIALIZED;
	result.string = DEF_ERR_ALREADY_INITIALIZED_STR;
	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	out_of_memory:
	result.code = DEF_ERR_OUT_OF_MEMORY;
	result.string = DEF_ERR_OUT_OF_MEMORY_STR;
	return result;

	nintendo_api_failed:
	free(util_curl_buffer);
	util_curl_buffer = NULL;
	result.string = DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR;
	return result;
}

void Util_curl_exit()
{
	if(!util_curl_init)
		return;

	util_curl_init = false;
	socExit();
	free(util_curl_buffer);
	util_curl_buffer = NULL;
}

void Util_curl_close(CURL* curl_handle)
{
	if(curl_handle)
		curl_easy_cleanup(curl_handle);
		
	curl_handle = NULL;
}

size_t Util_curl_write_callback(char* input_data, size_t size, size_t nmemb, void *user_data)
{
	int buffer_size = 0;
	int input_size = size * nmemb;
	int allocated_size = 0;
	Http_data* http_data = (Http_data*)user_data;

	if(!user_data)
		return -1;

	//Out of memory
	if((*http_data->used_size) + input_size > http_data->max_size)
		goto error;

	//Need to realloc memory
	if((*http_data->used_size) + input_size > http_data->current_size)
	{
		buffer_size = http_data->max_size > http_data->current_size + 0x40000 ? http_data->current_size  + 0x40000 : http_data->max_size;

		http_data->data = (u8*)Util_safe_linear_realloc(http_data->data, buffer_size);
		if (!http_data->data)
			goto error;
		
		allocated_size = buffer_size - http_data->current_size;
		memset(http_data->data + http_data->current_size, 0x0, allocated_size);
		http_data->current_size = buffer_size;
	}

	memcpy(http_data->data + (*http_data->used_size), input_data, input_size);
	*http_data->used_size += input_size;

	return input_size;

	error:
	Util_safe_linear_free(http_data->data);
	http_data->data = NULL;
	*http_data->used_size = 0;
	http_data->current_size = 0;
	http_data->max_size = 0;
	return -1;
}

size_t Util_curl_save_callback(char* input_data, size_t size, size_t nmemb, void *user_data)
{
	int input_size = size * nmemb;
	int input_data_offset = 0;
	Result_with_string result;
	Http_data* http_data = (Http_data*)user_data;

	if(!user_data)
		return -1;

	*http_data->used_size += input_size;
	
	//If libcurl buffer size is bigger than our buffer size, save it directly without buffering
	if(input_size > http_data->max_size)
	{
		result = Util_file_save_to_file(http_data->file_name, http_data->dir_path, (u8*)input_data, input_size, false);
		http_data->current_size = 0;
		if(result.code == 0)
			return size * nmemb;
		else
			return -1;
	}
	//If we run out of buffer, save it
	else if(http_data->current_size + input_size > http_data->max_size)
	{
		memcpy(http_data->data + http_data->current_size, input_data, http_data->max_size - http_data->current_size);
		input_data_offset = http_data->max_size - http_data->current_size;

		http_data->current_size += input_data_offset;
		input_size = input_size - input_data_offset;

		result = Util_file_save_to_file(http_data->file_name, http_data->dir_path, http_data->data, http_data->current_size, false);
		http_data->current_size = 0;
	}
	else
		result.code = 0;

	memcpy(http_data->data + http_data->current_size, input_data + input_data_offset, input_size);
	http_data->current_size += input_size;

	if(result.code == 0)
		return size * nmemb;
	else
		return -1;
}

Result_with_string Util_curl_request(CURL* curl_handle, std::string url, CURLoption method, bool follow_redirect,
int max_redirect, u8* post_data, int post_data_size)
{
	Result_with_string result;

	//Never returns error
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());

	if(method == CURLOPT_HTTPPOST)
		result.code = curl_easy_setopt(curl_handle, CURLOPT_POST, 1);
	else
		result.code = curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);

	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	result.code = curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	result.code = curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, follow_redirect);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	result.code = curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, max_redirect);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	result.code = curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, (DEF_HTTP_USER_AGENT).c_str());
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	result.code = curl_easy_setopt(curl_handle, CURLOPT_BUFFERSIZE, 1024 * 64);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	if(method == CURLOPT_HTTPPOST)
	{
		result.code = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, (char*)post_data);
		if (result.code != CURLE_OK)
		{
			result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
			goto curl_api_failed;
		}

		result.code = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, post_data_size);
		if (result.code != CURLE_OK)
		{
			result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
			goto curl_api_failed;
		}
	}

	return result;

	curl_api_failed:
	Util_curl_close(curl_handle);
	result.code = DEF_ERR_CURL_RETURNED_NOT_SUCCESS;
	result.string = DEF_ERR_CURL_RETURNED_NOT_SUCCESS_STR;
	return result;
}

Result_with_string Util_curl_get_request(CURL* curl_handle, std::string url, bool follow_redirect, int max_redirect)
{
	return Util_curl_request(curl_handle, url, CURLOPT_HTTPGET, follow_redirect, max_redirect, NULL, 0);
}

Result_with_string Util_curl_post_request(CURL* curl_handle, std::string url, u8* post_data, int post_data_size, bool follow_redirect, int max_redirect)
{
	return Util_curl_request(curl_handle, url, CURLOPT_HTTPPOST, follow_redirect, max_redirect, post_data, post_data_size);
}

void Util_curl_get_response(CURL* curl_handle, int* status_code, std::string* new_url)
{
	char moved_url[4096];
	Result_with_string result;

	memset(moved_url, 0x0, 4096);
	*new_url = "";
	*status_code = 0;

	result.code = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &status_code);
	if(result.code != CURLE_OK)
		*status_code = 0;

	result.code = curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, moved_url);
	if(result.code != CURLE_OK)
		*new_url = "";
	else
		*new_url = moved_url;
}

Result_with_string Util_curl_download_data(CURL* curl_handle, Http_data* http_data)
{
	Result_with_string result;
	char error_data[4096];
	memset(error_data, 0, 4096);

	result.code = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, Util_curl_write_callback);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	result.code = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)http_data);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	result.code = curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error_data);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	result.code = curl_easy_perform(curl_handle);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_perform() failed. " + std::to_string(result.code) + " " + error_data + " ";
		goto curl_api_failed;
	}
	return result;

	curl_api_failed:
	Util_curl_close(curl_handle);
	Util_safe_linear_free(http_data->data);
	http_data->data = NULL;
	*http_data->used_size = 0;
	http_data->current_size = 0;
	http_data->max_size = 0;
	result.code = DEF_ERR_CURL_RETURNED_NOT_SUCCESS;
	result.string = DEF_ERR_CURL_RETURNED_NOT_SUCCESS_STR;
	return result;
}

Result_with_string Util_curl_save_data(CURL* curl_handle, Http_data* http_data)
{
	Result_with_string result;
	char error_data[4096];
	memset(error_data, 0, 4096);

	http_data->data = (u8*)Util_safe_linear_alloc(http_data->max_size);
	if(!http_data->data)
		goto out_of_memory;

	result.code = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, Util_curl_save_callback);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	result.code = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)http_data);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	result.code = curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error_data);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_setopt() failed. " + std::to_string(result.code) + " ";
		goto curl_api_failed;
	}

	Util_file_delete_file(http_data->file_name, http_data->dir_path);

	result.code = curl_easy_perform(curl_handle);
	if (result.code != CURLE_OK)
	{
		result.error_description = "[Error] curl_easy_perform() failed. " + std::to_string(result.code) + " " + error_data + " ";
		goto curl_api_failed;
	}

	Util_file_save_to_file(http_data->file_name, http_data->dir_path, http_data->data, http_data->current_size, false);

	Util_safe_linear_free(http_data->data);
	http_data->data = NULL;
	return result;

	out_of_memory:
	Util_curl_close(curl_handle);
	*http_data->used_size = 0;
	http_data->current_size = 0;
	http_data->max_size = 0;
	result.code = DEF_ERR_OUT_OF_LINEAR_MEMORY;
	result.string = DEF_ERR_OUT_OF_LINEAR_MEMORY_STR;

	curl_api_failed:
	Util_curl_close(curl_handle);
	Util_safe_linear_free(http_data->data);
	http_data->data = NULL;
	*http_data->used_size = 0;
	http_data->current_size = 0;
	http_data->max_size = 0;
	result.code = DEF_ERR_CURL_RETURNED_NOT_SUCCESS;
	result.string = DEF_ERR_CURL_RETURNED_NOT_SUCCESS_STR;
	return result;
}

Result_with_string Util_curl_dl_data(std::string url, u8** data, int max_size, int* downloaded_size, bool follow_redirect,
int max_redirect)
{
	std::string last_url = "";
	int status_code = 0;
	return Util_curl_dl_data(url, data, max_size, downloaded_size, &status_code, follow_redirect, max_redirect, &last_url);
}

Result_with_string Util_curl_dl_data(std::string url, u8** data, int max_size, int* downloaded_size, bool follow_redirect,
int max_redirect, std::string* last_url)
{
	int status_code = 0;
	return Util_curl_dl_data(url, data, max_size, downloaded_size, &status_code, follow_redirect, max_redirect, last_url);
}

Result_with_string Util_curl_dl_data(std::string url, u8** data, int max_size, int* downloaded_size, int* status_code, bool follow_redirect,
int max_redirect)
{
	std::string last_url = "";
	return Util_curl_dl_data(url, data, max_size, downloaded_size, status_code, follow_redirect, max_redirect, &last_url);
}

Result_with_string Util_curl_dl_data(std::string url, u8** data, int max_size, int* downloaded_size, int* status_code, bool follow_redirect,
int max_redirect, std::string* last_url)
{
	Result_with_string result;
	Http_data http_data;
	CURL* curl_handle = NULL;

	if(!util_curl_init)
		goto not_inited;
	
	if(url == "" || !data || max_size <= 0 || !downloaded_size || !status_code || (follow_redirect && max_redirect < 0) || !last_url)
		goto invalid_arg;
	
	*last_url = "";
	*downloaded_size = 0;

	for(int i = 0; i < 40; i++)
	{
		result.code = acWaitInternetConnection();
		if(result.code != 0xE0A09D2E)
			break;
		
		usleep(100000);
	}
	
	if(result.code != 0)
	{
		result.error_description = "[Error] acWaitInternetConnection() failed. ";
		goto nintendo_api_failed;
	}

	curl_handle = curl_easy_init();
	if(!curl_handle)
	{
		result.error_description = "[Error] curl_easy_init() failed. ";
		goto curl_api_failed;
	}

	http_data.max_size = max_size;
	http_data.used_size = downloaded_size;
	result = Util_curl_get_request(curl_handle, url, follow_redirect, max_redirect);
	if(result.code != 0)
		goto api_failed;
	
	result = Util_curl_download_data(curl_handle, &http_data);
	if(result.code != 0)
		goto api_failed;

	*data = (u8*)http_data.data;

	Util_curl_get_response(curl_handle, status_code, last_url);

	Util_curl_close(curl_handle);

	return result;

	not_inited:
	result.code = DEF_ERR_NOT_INITIALIZED;
	result.string = DEF_ERR_NOT_INITIALIZED_STR;
	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	nintendo_api_failed:
	result.string = DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR;
	return result;

	curl_api_failed:
	result.code = DEF_ERR_CURL_RETURNED_NOT_SUCCESS;
	result.string = DEF_ERR_CURL_RETURNED_NOT_SUCCESS_STR;
	return result;

	api_failed:
	return result;
}

Result_with_string Util_curl_save_data(std::string url, int buffer_size, int* downloaded_size, bool follow_redirect,
int max_redirect, std::string dir_path, std::string file_name)
{
	std::string last_url = "";
	int status_code = 0;
	return Util_curl_save_data(url, buffer_size, downloaded_size, &status_code, follow_redirect, max_redirect,
	&last_url, dir_path, file_name);
}

Result_with_string Util_curl_save_data(std::string url, int buffer_size, int* downloaded_size, bool follow_redirect,
int max_redirect, std::string* last_url, std::string dir_path, std::string file_name)
{
	int status_code = 0;
	return Util_curl_save_data(url, buffer_size, downloaded_size, &status_code, follow_redirect, max_redirect,
	last_url, dir_path, file_name);
}

Result_with_string Util_curl_save_data(std::string url, int buffer_size, int* downloaded_size, int* status_code, bool follow_redirect,
int max_redirect, std::string dir_path, std::string file_name)
{
	std::string last_url = "";
	return Util_curl_save_data(url, buffer_size, downloaded_size, status_code, follow_redirect, max_redirect,
	&last_url, dir_path, file_name);
}

Result_with_string Util_curl_save_data(std::string url, int buffer_size, int* downloaded_size, int* status_code, bool follow_redirect,
int max_redirect, std::string* last_url, std::string dir_path, std::string file_name)
{
	Result_with_string result;
	Http_data http_data;
	CURL* curl_handle = NULL;

	if(!util_curl_init)
		goto not_inited;
	
	if(url == "" || buffer_size <= 0 || !downloaded_size || !status_code || (follow_redirect && max_redirect < 0) || !last_url)
		goto invalid_arg;
	
	*last_url = "";
	*downloaded_size = 0;

	for(int i = 0; i < 40; i++)
	{
		result.code = acWaitInternetConnection();
		if(result.code != 0xE0A09D2E)
			break;
		
		usleep(100000);
	}
	
	if(result.code != 0)
	{
		result.error_description = "[Error] acWaitInternetConnection() failed. ";
		goto nintendo_api_failed;
	}

	curl_handle = curl_easy_init();
	if(!curl_handle)
	{
		result.error_description = "[Error] curl_easy_init() failed. ";
		goto curl_api_failed;
	}

	http_data.max_size = buffer_size;
	http_data.used_size = downloaded_size;
	http_data.dir_path = dir_path;
	http_data.file_name = file_name;
	result = Util_curl_get_request(curl_handle, url, follow_redirect, max_redirect);
	if(result.code != 0)
		goto api_failed;
	
	result = Util_curl_save_data(curl_handle, &http_data);
	if(result.code != 0)
		goto api_failed;

	Util_curl_get_response(curl_handle, status_code, last_url);

	Util_curl_close(curl_handle);

	return result;

	not_inited:
	result.code = DEF_ERR_NOT_INITIALIZED;
	result.string = DEF_ERR_NOT_INITIALIZED_STR;
	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	nintendo_api_failed:
	result.string = DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR;
	return result;

	curl_api_failed:
	result.code = DEF_ERR_CURL_RETURNED_NOT_SUCCESS;
	result.string = DEF_ERR_CURL_RETURNED_NOT_SUCCESS_STR;
	return result;

	api_failed:
	return result;
}

Result_with_string Util_curl_post_and_dl_data(std::string url, u8* post_data, int post_size, u8** dl_data, int max_dl_size,
int* downloaded_size, bool follow_redirect, int max_redirect)
{
	std::string last_url = "";
	int status_code = 0;
	return Util_curl_post_and_dl_data(url, post_data, post_size, dl_data, max_dl_size, downloaded_size,
	&status_code, follow_redirect, max_redirect, &last_url);
}

Result_with_string Util_curl_post_and_dl_data(std::string url, u8* post_data, int post_size, u8** dl_data, int max_dl_size,
int* downloaded_size, bool follow_redirect, int max_redirect, std::string* last_url)
{
	int status_code = 0;
	return Util_curl_post_and_dl_data(url, post_data, post_size, dl_data, max_dl_size, downloaded_size,
	&status_code, follow_redirect, max_redirect, last_url);
}

Result_with_string Util_curl_post_and_dl_data(std::string url, u8* post_data, int post_size, u8** dl_data, int max_dl_size,
int* downloaded_size, int* status_code, bool follow_redirect, int max_redirect)
{
	std::string last_url = "";
	return Util_curl_post_and_dl_data(url, post_data, post_size, dl_data, max_dl_size, downloaded_size,
	status_code, follow_redirect, max_redirect, &last_url);
}

Result_with_string Util_curl_post_and_dl_data(std::string url, u8* post_data, int post_size, u8** dl_data, int max_dl_size,
int* downloaded_size, int* status_code, bool follow_redirect, int max_redirect, std::string* last_url)
{
	Result_with_string result;
	Http_data http_data;
	CURL* curl_handle = NULL;

	if(!util_curl_init)
		goto not_inited;
	
	if(url == "" || !post_data || post_size <= 0 || !dl_data || max_dl_size <= 0 || !downloaded_size || !status_code 
	|| (follow_redirect && max_redirect < 0) || !last_url)
		goto invalid_arg;
	
	*last_url = "";
	*downloaded_size = 0;

	for(int i = 0; i < 40; i++)
	{
		result.code = acWaitInternetConnection();
		if(result.code != 0xE0A09D2E)
			break;
		
		usleep(100000);
	}
	
	if(result.code != 0)
	{
		result.error_description = "[Error] acWaitInternetConnection() failed. ";
		goto nintendo_api_failed;
	}

	curl_handle = curl_easy_init();
	if(!curl_handle)
	{
		result.error_description = "[Error] curl_easy_init() failed. ";
		goto curl_api_failed;
	}

	http_data.max_size = max_dl_size;
	http_data.used_size = downloaded_size;
	result = Util_curl_post_request(curl_handle, url, post_data, post_size, follow_redirect, max_redirect);
	if(result.code != 0)
		goto api_failed;
	
	result = Util_curl_download_data(curl_handle, &http_data);
	if(result.code != 0)
		goto api_failed;

	*dl_data = (u8*)http_data.data;

	Util_curl_get_response(curl_handle, status_code, last_url);

	Util_curl_close(curl_handle);

	return result;

	not_inited:
	result.code = DEF_ERR_NOT_INITIALIZED;
	result.string = DEF_ERR_NOT_INITIALIZED_STR;
	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	nintendo_api_failed:
	result.string = DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR;
	return result;

	curl_api_failed:
	result.code = DEF_ERR_CURL_RETURNED_NOT_SUCCESS;
	result.string = DEF_ERR_CURL_RETURNED_NOT_SUCCESS_STR;
	return result;

	api_failed:
	return result;
}

Result_with_string Util_curl_post_and_save_data(std::string url, u8* post_data, int post_size, int buffer_size, int* downloaded_size,
bool follow_redirect, int max_redirect, std::string dir_path, std::string file_name)
{
	std::string last_url = "";
	int status_code = 0;
	return Util_curl_post_and_save_data(url, post_data, post_size, buffer_size, downloaded_size,
	&status_code, follow_redirect, max_redirect, &last_url, dir_path, file_name);
}

Result_with_string Util_curl_post_and_save_data(std::string url, u8* post_data, int post_size, int buffer_size, int* downloaded_size,
bool follow_redirect, int max_redirect, std::string* last_url, std::string dir_path, std::string file_name)
{
	int status_code = 0;
	return Util_curl_post_and_save_data(url, post_data, post_size, buffer_size, downloaded_size,
	&status_code, follow_redirect, max_redirect, last_url, dir_path, file_name);
}

Result_with_string Util_curl_post_and_save_data(std::string url, u8* post_data, int post_size, int buffer_size, int* downloaded_size,
int* status_code, bool follow_redirect, int max_redirect, std::string dir_path, std::string file_name)
{
	std::string last_url = "";
	return Util_curl_post_and_save_data(url, post_data, post_size, buffer_size, downloaded_size,
	status_code, follow_redirect, max_redirect, &last_url, dir_path, file_name);
}

Result_with_string Util_curl_post_and_save_data(std::string url, u8* post_data, int post_size, int buffer_size, int* downloaded_size,
int* status_code, bool follow_redirect, int max_redirect, std::string* last_url, std::string dir_path, std::string file_name)
{
	Result_with_string result;
	Http_data http_data;
	CURL* curl_handle = NULL;

	if(!util_curl_init)
		goto not_inited;
	
	if(url == "" || !post_data || post_size <= 0 || buffer_size <= 0 || !downloaded_size || !status_code 
	|| (follow_redirect && max_redirect < 0) || !last_url)
		goto invalid_arg;
	
	*last_url = "";
	*downloaded_size = 0;

	for(int i = 0; i < 40; i++)
	{
		result.code = acWaitInternetConnection();
		if(result.code != 0xE0A09D2E)
			break;
		
		usleep(100000);
	}
	
	if(result.code != 0)
	{
		result.error_description = "[Error] acWaitInternetConnection() failed. ";
		goto nintendo_api_failed;
	}

	curl_handle = curl_easy_init();
	if(!curl_handle)
	{
		result.error_description = "[Error] curl_easy_init() failed. ";
		goto curl_api_failed;
	}

	http_data.max_size = buffer_size;
	http_data.used_size = downloaded_size;
	http_data.dir_path = dir_path;
	http_data.file_name = file_name;
	result = Util_curl_post_request(curl_handle, url, post_data, post_size, follow_redirect, max_redirect);
	if(result.code != 0)
		goto api_failed;
	
	result = Util_curl_save_data(curl_handle, &http_data);
	if(result.code != 0)
		goto api_failed;

	Util_curl_get_response(curl_handle, status_code, last_url);

	Util_curl_close(curl_handle);

	return result;

	not_inited:
	result.code = DEF_ERR_NOT_INITIALIZED;
	result.string = DEF_ERR_NOT_INITIALIZED_STR;
	return result;

	invalid_arg:
	result.code = DEF_ERR_INVALID_ARG;
	result.string = DEF_ERR_INVALID_ARG_STR;
	return result;

	nintendo_api_failed:
	result.string = DEF_ERR_NINTENDO_RETURNED_NOT_SUCCESS_STR;
	return result;

	curl_api_failed:
	result.code = DEF_ERR_CURL_RETURNED_NOT_SUCCESS;
	result.string = DEF_ERR_CURL_RETURNED_NOT_SUCCESS_STR;
	return result;

	api_failed:
	return result;
}
