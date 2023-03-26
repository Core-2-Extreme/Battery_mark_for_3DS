#pragma once
#include <string>

//These definitions are based on (stolen from) library\ffmpeg\include\libavutil\pixfmt.h see it for more information.
enum Pixel_format
{
    PIXEL_FORMAT_INVALID            = -1,
    //YUV*
    PIXEL_FORMAT_YUV410P            = 0,
    PIXEL_FORMAT_YUV411P            = 1,
    PIXEL_FORMAT_YUV420P            = 2,
    PIXEL_FORMAT_YUV420P9BE         = 3,
    PIXEL_FORMAT_YUV420P9LE         = 4,
    PIXEL_FORMAT_YUV420P10BE        = 5,
    PIXEL_FORMAT_YUV420P10LE        = 6,
    PIXEL_FORMAT_YUV420P12BE        = 7,
    PIXEL_FORMAT_YUV420P12LE        = 8,
    PIXEL_FORMAT_YUV420P14BE        = 9,
    PIXEL_FORMAT_YUV420P14LE        = 10,
    PIXEL_FORMAT_YUV420P16BE        = 11,
    PIXEL_FORMAT_YUV420P16LE        = 12,
    PIXEL_FORMAT_YUV422P            = 13,
    PIXEL_FORMAT_YUV422P9BE         = 14,
    PIXEL_FORMAT_YUV422P9LE         = 15,
    PIXEL_FORMAT_YUV422P10BE        = 16,
    PIXEL_FORMAT_YUV422P10LE        = 17,
    PIXEL_FORMAT_YUV422P12BE        = 18,
    PIXEL_FORMAT_YUV422P12LE        = 19,
    PIXEL_FORMAT_YUV422P14BE        = 20,
    PIXEL_FORMAT_YUV422P14LE        = 21,
    PIXEL_FORMAT_YUV422P16BE        = 22,
    PIXEL_FORMAT_YUV422P16LE        = 23,
    PIXEL_FORMAT_YUV440P            = 24,
    PIXEL_FORMAT_YUV440P10BE        = 25,
    PIXEL_FORMAT_YUV440P10LE        = 26,
    PIXEL_FORMAT_YUV440P12BE        = 27,
    PIXEL_FORMAT_YUV440P12LE        = 28,
    PIXEL_FORMAT_YUV444P            = 29,
    PIXEL_FORMAT_YUV444P9BE         = 30,
    PIXEL_FORMAT_YUV444P9LE         = 31,
    PIXEL_FORMAT_YUV444P10BE        = 32,
    PIXEL_FORMAT_YUV444P10LE        = 33,
    PIXEL_FORMAT_YUV444P12BE        = 34,
    PIXEL_FORMAT_YUV444P12LE        = 35,
    PIXEL_FORMAT_YUV444P14BE        = 36,
    PIXEL_FORMAT_YUV444P14LE        = 37,
    PIXEL_FORMAT_YUV444P16BE        = 38,
    PIXEL_FORMAT_YUV444P16LE        = 39,
    //YUVJ*
    PIXEL_FORMAT_YUVJ411P           = 40,
    PIXEL_FORMAT_YUVJ420P           = 41,
    PIXEL_FORMAT_YUVJ422P           = 42,
    PIXEL_FORMAT_YUVJ440P           = 43,
    PIXEL_FORMAT_YUVJ444P           = 44,
    //YUVA*
    PIXEL_FORMAT_YUVA420P           = 45,
    PIXEL_FORMAT_YUVA420P9BE        = 46,
    PIXEL_FORMAT_YUVA420P9LE        = 47,
    PIXEL_FORMAT_YUVA420P10BE       = 48,
    PIXEL_FORMAT_YUVA420P10LE       = 49,
    PIXEL_FORMAT_YUVA420P16BE       = 50,
    PIXEL_FORMAT_YUVA420P16LE       = 51,
    PIXEL_FORMAT_YUVA422P           = 52,
    PIXEL_FORMAT_YUVA422P9BE        = 53,
    PIXEL_FORMAT_YUVA422P9LE        = 54,
    PIXEL_FORMAT_YUVA422P10BE       = 55,
    PIXEL_FORMAT_YUVA422P10LE       = 56,
    PIXEL_FORMAT_YUVA422P16BE       = 57,
    PIXEL_FORMAT_YUVA422P16LE       = 58,
    PIXEL_FORMAT_YUVA444P           = 59,
    PIXEL_FORMAT_YUVA444P9BE        = 60,
    PIXEL_FORMAT_YUVA444P9LE        = 61,
    PIXEL_FORMAT_YUVA444P10BE       = 62,
    PIXEL_FORMAT_YUVA444P10LE       = 63,
    PIXEL_FORMAT_YUVA444P16BE       = 64,
    PIXEL_FORMAT_YUVA444P16LE       = 65,
    //UYVY*
    PIXEL_FORMAT_UYVY422            = 66,
    //YUYV*
    PIXEL_FORMAT_YUYV422            = 67,
    //YVYU*
    PIXEL_FORMAT_YVYU422            = 68,
    //UYYVYY*
    PIXEL_FORMAT_UYYVYY411          = 69,
    //RGB* (exclude RGBA)
    PIXEL_FORMAT_RGB121             = 70,
    PIXEL_FORMAT_RGB121_BYTE        = 71,
    PIXEL_FORMAT_RGB332             = 72,
    PIXEL_FORMAT_RGB444BE           = 73,
    PIXEL_FORMAT_RGB444LE           = 74,
    PIXEL_FORMAT_RGB555BE           = 75,
    PIXEL_FORMAT_RGB555LE           = 76,
    PIXEL_FORMAT_RGB565BE           = 77,
    PIXEL_FORMAT_RGB565LE           = 78,
    PIXEL_FORMAT_RGB888             = 79,
    PIXEL_FORMAT_RGB161616BE        = 80,
    PIXEL_FORMAT_RGB161616LE        = 81,
    //BGR* (exclude BGRA)
    PIXEL_FORMAT_BGR121             = 82,
    PIXEL_FORMAT_BGR121_BYTE        = 83,
    PIXEL_FORMAT_BGR332             = 84,
    PIXEL_FORMAT_BGR444BE           = 85,
    PIXEL_FORMAT_BGR444LE           = 86,
    PIXEL_FORMAT_BGR555BE           = 87,
    PIXEL_FORMAT_BGR555LE           = 88,
    PIXEL_FORMAT_BGR565BE           = 89,
    PIXEL_FORMAT_BGR565LE           = 90,
    PIXEL_FORMAT_BGR888             = 91,
    PIXEL_FORMAT_BGR161616BE        = 92,
    PIXEL_FORMAT_BGR161616LE        = 93,
    //GBR* (exclude GRBA)
    PIXEL_FORMAT_GBR888P            = 94,
    PIXEL_FORMAT_GBR999PBE          = 95,
    PIXEL_FORMAT_GBR999PLE          = 96,
    PIXEL_FORMAT_GBR101010PBE       = 97,
    PIXEL_FORMAT_GBR101010PLE       = 98,
    PIXEL_FORMAT_GBR121212PBE       = 99,
    PIXEL_FORMAT_GBR121212PLE       = 100,
    PIXEL_FORMAT_GBR141414PBE       = 101,
    PIXEL_FORMAT_GBR141414PLE       = 102,
    PIXEL_FORMAT_GBR161616PBE       = 103,
    PIXEL_FORMAT_GBR161616PLE       = 104,
    //ARGB*
    PIXEL_FORMAT_ARGB8888           = 105,
    //ABGR*
    PIXEL_FORMAT_ABGR8888           = 106,
    //RGBA*
    PIXEL_FORMAT_RGBA8888           = 107,
    PIXEL_FORMAT_RGBA16161616BE     = 108,
    PIXEL_FORMAT_RGBA16161616LE     = 109,
    //BGRA*
    PIXEL_FORMAT_BGRA8888           = 110,
    PIXEL_FORMAT_BGRA16161616BE     = 111,
    PIXEL_FORMAT_BGRA16161616LE     = 112,
    //GBRA*
    PIXEL_FORMAT_GBRA8888P          = 113,
    PIXEL_FORMAT_GBRA10101010PBE    = 114,
    PIXEL_FORMAT_GBRA10101010PLE    = 115,
    PIXEL_FORMAT_GBRA12121212PBE    = 116,
    PIXEL_FORMAT_GBRA12121212PLE    = 117,
    PIXEL_FORMAT_GBRA16161616PBE    = 118,
    PIXEL_FORMAT_GBRA16161616PLE    = 119,
    //GRAY*
    PIXEL_FORMAT_GRAY8              = 120,
    PIXEL_FORMAT_GRAY10BE           = 121,
    PIXEL_FORMAT_GRAY10LE           = 122,
    PIXEL_FORMAT_GRAY12BE           = 123,
    PIXEL_FORMAT_GRAY12LE           = 124,
    PIXEL_FORMAT_GRAY16BE           = 125,
    PIXEL_FORMAT_GRAY16LE           = 126,
    //GRAYALPHA*
    PIXEL_FORMAT_GRAYALPHA88        = 127,
    PIXEL_FORMAT_GRAYALPHA1616BE    = 128,
    PIXEL_FORMAT_GRAYALPHA1616LE    = 129,

    PIXEL_FORMAT_MAX,
};

//These definitions are based on (stolen from) library\ffmpeg\include\libavutil\smaplefmt.h see it for more information.
enum Sample_format
{
    SAMPLE_FORMAT_INVALID   = -1,
    //Integer
    SAMPLE_FORMAT_U8        = 0,
    SAMPLE_FORMAT_U8P       = 1,
    SAMPLE_FORMAT_S16       = 2,
    SAMPLE_FORMAT_S16P      = 3,
    SAMPLE_FORMAT_S32       = 4,
    SAMPLE_FORMAT_S32P      = 5,
    SAMPLE_FORMAT_S64       = 6,
    SAMPLE_FORMAT_S64P      = 7,
    //Float
    SAMPLE_FORMAT_FLOAT32   = 8,
    SAMPLE_FORMAT_FLOAT32P  = 9,
    //Double
    SAMPLE_FORMAT_DOUBLE64  = 10,
    SAMPLE_FORMAT_DOUBLE64P = 11,

    SAMPLE_FORMAT_MAX,
};

struct Result_with_string
{
	std::string string = "[Success] ";
	std::string error_description = "";
	uint code = DEF_SUCCESS;
};

struct Watch_bool
{
	bool* address = NULL;
	bool previous_value = false;
};

struct Watch_int
{
	int* address = NULL;
	int previous_value = 0;
};

struct Watch_double
{
	double* address = NULL;
	double previous_value = 0;
};

struct Watch_string
{
	std::string* address = NULL;
	std::string previous_value = "";
};

struct Audio_info
{
	int bitrate = 0;				//Audio bitrate in Bps.
	int sample_rate = 0;		    //Audio smaple rate in Hz.
	int ch = 0;                 	//Number of audio channels.
	std::string format_name = "";	//Audio codec name.
	double duration = 0;			//Audio track duration in seconds.
	std::string track_lang = "";    //Track languages
	Sample_format sample_format = SAMPLE_FORMAT_INVALID;    //Audio sample format.
};

struct Video_info
{
	int width = 0;					//Video width.
	int height = 0;					//Video height.
	double framerate = 0;			//Video framerate.
	std::string format_name = "";	//Video codec name.
	double duration = 0;			//Video track duration in seconds.
	int thread_type = DEF_DECODER_THREAD_TYPE_NONE;	    //Threading method (DEF_DECODER_THREAD_TYPE_*).
	int sar_width = 1;	            //Sample aspect ratio for width.
	int sar_height = 1;             //Sample aspect ratio for height.
	Pixel_format pixel_format = PIXEL_FORMAT_INVALID;   //Video pixel format.
};

struct Color_converter_parameters
{
	u8* source = NULL;		//(in)  Source raw image data, user must allocate the buffer.
	u8* converted = NULL;	//(out) Converted raw image data, this buffer will be allocated inside of function.
	int in_width = 0;		//(in)  Source image width.
	int in_height = 0;		//(in)  Source image height.
	int out_width = 0;		//(in)  Converted image width.
	int out_height = 0;     //(in)  Converted image height.
	Pixel_format in_color_format = PIXEL_FORMAT_INVALID;    //(in) Source image pixel format.
	Pixel_format out_color_format = PIXEL_FORMAT_INVALID;	//(in) Converted image pixel format.
};

struct Audio_converter_parameters
{
	u8* source = NULL;			//(in)  Source raw audio data, user must allocate the buffer.
	u8* converted = NULL;		//(out) Converted raw audio data, this buffer will be allocated inside of function.
	int in_samples = 0;			//(in)  Number of source audio samples per channel.
	int in_ch = 0;				//(in)  Source audio ch.
	int in_sample_rate = 0;		//(in)  Source audio saple rate in Hz.
	int out_samples = 0;		//(out) Number of converted audio samples per channel.
	int out_ch = 0;				//(in)  Converted audio ch.
	int out_sample_rate = 0;	//(in)  Converted audio saple rate in Hz.
	Sample_format in_sample_format = SAMPLE_FORMAT_INVALID;     //(in) Source audio sample format.
	Sample_format out_sample_format = SAMPLE_FORMAT_INVALID;    //(in) Converted audio sample format.
};

struct Subtitle_info
{
	std::string format_name = "";
	std::string track_lang = "";
};

struct Subtitle_data
{
	std::string text = "";
	double start_time = 0;
	double end_time = 0;
};

struct Image_data
{
	C2D_Image c2d = { .tex = NULL, };
	Tex3DS_SubTexture* subtex = NULL;
	bool selected = false;
	double x = -1;
	double y = -1;
	double x_size = -1;
	double y_size = -1;
};

struct Hid_info
{
	bool p_a = false;
	bool p_b = false;
	bool p_x = false;
	bool p_y = false;
	bool p_c_up = false;
	bool p_c_down = false;
	bool p_c_left = false;
	bool p_c_right = false;
	bool p_d_up = false;
	bool p_d_down = false;
	bool p_d_left = false;
	bool p_d_right = false;
	bool p_l = false;
	bool p_r = false;
	bool p_zl = false;
	bool p_zr = false;
	bool p_start = false;
	bool p_select = false;
	bool p_cs_up = false;
	bool p_cs_down = false;
	bool p_cs_left = false;
	bool p_cs_right = false;
	bool p_touch = false;
	bool p_any = false;
	bool h_a = false;
	bool h_b = false;
	bool h_x = false;
	bool h_y = false;
	bool h_c_up = false;
	bool h_c_down = false;
	bool h_c_left = false;
	bool h_c_right = false;
	bool h_d_up = false;
	bool h_d_down = false;
	bool h_d_left = false;
	bool h_d_right = false;
	bool h_l = false;
	bool h_r = false;
	bool h_zl = false;
	bool h_zr = false;
	bool h_start = false;
	bool h_select = false;
	bool h_cs_up = false;
	bool h_cs_down = false;
	bool h_cs_left = false;
	bool h_cs_right = false;
	bool h_touch = false;
	bool h_any = false;
	bool r_a = false;
	bool r_b = false;
	bool r_x = false;
	bool r_y = false;
	bool r_c_up = false;
	bool r_c_down = false;
	bool r_c_left = false;
	bool r_c_right = false;
	bool r_d_up = false;
	bool r_d_down = false;
	bool r_d_left = false;
	bool r_d_right = false;
	bool r_l = false;
	bool r_r = false;
	bool r_zl = false;
	bool r_zr = false;
	bool r_start = false;
	bool r_select = false;
	bool r_cs_up = false;
	bool r_cs_down = false;
	bool r_cs_left = false;
	bool r_cs_right = false;
	bool r_touch = false;
	bool r_any = false;
	int cpad_x = 0;
	int cpad_y = 0;
	int touch_x = 0;
	int touch_y = 0;
	int touch_x_move = 0;
	int touch_y_move = 0;
	int held_time = 0;
	u64 ts = 0;
};
