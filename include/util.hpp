#pragma once

#define UTIL_AUDIO_ENCODER_0 0
#define UTIL_AUDIO_ENCODER_1 1

#define UTIL_DECODER_0 0
#define UTIL_DECODER_1 1

extern "C" {
#include <libavcodec/avcodec.h>
}

Result_with_string Util_init_audio_encoder(AVCodecID id, int samplerate, int bitrate, int session);

Result_with_string Util_encode_audio(int size, u8* raw_data, int* encoded_size, u8* encoded_data, int session);

void Util_exit_audio_encoder(int session);

Result_with_string Util_open_file(std::string file_path, bool* has_audio, bool* has_video, int session);

Result_with_string Util_init_audio_decoder(int session);

Result_with_string Util_init_video_decoder(int low_resolution, int session);

void Util_get_audio_info(int* bitrate, int* samplerate, int* ch, std::string* format_name, double* duration, int session);

void Util_get_video_info(int* width, int* height, double* framerate, std::string* format_name, double* duration, int session);

Result_with_string Util_read_packet(AVMediaType* type, int session);

Result_with_string Util_ready_audio_packet(int session);

Result_with_string Util_ready_video_packet(int session);

Result_with_string Util_decode_audio(int* size, u8** raw_data, int session);

Result_with_string Util_decode_video(int* width, int* height, bool* key_frame, double* current_pos, int session);

void Util_get_video_image(u8* raw_data, int width, int height, int session);

Result_with_string Util_seek(u64 seek_pos, int flag, int session);//AVSEEK_FLAG_FRAME 8 AVSEEK_FLAG_ANY 4  AVSEEK_FLAG_BACKWORD 1

void Util_close_file(int session);

void Util_exit_audio_decoder(int session);

void Util_exit_video_decoder(int session);
