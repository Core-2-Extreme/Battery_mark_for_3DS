#include <string>
#include <unistd.h>
#include <3ds.h>

#include "types.hpp"
#include "error.hpp"
#include "util.hpp"
#include "log.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include "libavformat/avformat.h"
}

extern "C" void memcpy_asm(u8*, u8*, int);


AVPacket* util_audio_encoder_packet[2] = { NULL, NULL, };
AVFrame* util_audio_encoder_raw_data[2] = { NULL, NULL, };
AVCodecContext* util_audio_encoder_context[2] = { NULL, NULL, };
AVCodec* util_audio_encoder_codec[2] = { NULL, NULL, };
SwrContext* util_audio_encoder_swr_context[2] = { NULL, NULL, };

bool util_video_decoder_lock[2][2] = { { false, false, }, { false, false, } };
int util_video_decoder_buffer_num[2] = { 0, 0, };
int util_video_decoder_ready_buffer_num[2] = { 0, 0, };
int util_video_decoder_stream_num[2] = { -1, -1, };
AVPacket* util_video_decoder_packet[2] = { NULL, NULL, };
AVPacket* util_video_decoder_cache_packet[2] = { NULL, NULL, };
AVFrame* util_video_decoder_raw_data[2][2] = { { NULL, NULL, }, { NULL, NULL, } };
AVCodecContext* util_video_decoder_context[2] = { NULL, NULL, };
AVCodec* util_video_decoder_codec[2] = { NULL, NULL, };

int util_audio_decoder_stream_num[2] = { -1, -1, };
AVPacket* util_audio_decoder_packet[2] = { NULL, NULL, };
AVPacket* util_audio_decoder_cache_packet[2] = { NULL, NULL, };
AVFrame* util_audio_decoder_raw_data[2] = { NULL, NULL, };
AVCodecContext* util_audio_decoder_context[2] = { NULL, NULL, };
AVCodec* util_audio_decoder_codec[2] = { NULL, NULL, };
SwrContext* util_audio_decoder_swr_context[2] = { NULL, NULL, };
AVFormatContext* util_decoder_format_context[2] = { NULL, NULL, };

Result_with_string Util_init_audio_encoder(AVCodecID id, int samplerate, int bitrate, int session)
{
	int ffmpeg_result = 0;
	int original_samplerate = samplerate;
	Result_with_string result;

	util_audio_encoder_codec[session] = avcodec_find_encoder(id);
	if(!util_audio_encoder_codec[session])
	{
		result.error_description = "avcodec_find_encoder() failed";
		goto fail;
	}

	util_audio_encoder_context[session] = avcodec_alloc_context3(util_audio_encoder_codec[session]);
	if(!util_audio_encoder_codec[session])
	{
		result.error_description = "avcodec_alloc_context3() failed";
		goto fail;
	}

	for(int i = 0; i < 30; i++)//select supported samplerate
	{
		if(!util_audio_encoder_codec[session]->supported_samplerates[i])
			break;
		else
		{
			if(util_audio_encoder_codec[session]->supported_samplerates[i] - samplerate <= 0)
			{
				samplerate += util_audio_encoder_codec[session]->supported_samplerates[i] - samplerate;
				break;
			}
		}
	}
	
	if(id == AV_CODEC_ID_MP2)
		util_audio_encoder_context[session]->sample_fmt = AV_SAMPLE_FMT_S16;
	else
		util_audio_encoder_context[session]->sample_fmt = AV_SAMPLE_FMT_FLT;
	
	util_audio_encoder_context[session]->bit_rate = bitrate;
	util_audio_encoder_context[session]->sample_rate = samplerate;
	util_audio_encoder_context[session]->channel_layout = AV_CH_LAYOUT_MONO;
	util_audio_encoder_context[session]->channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_MONO);
	//context->profile = FF_PROFILE_AAC_MAIN;
	util_audio_encoder_context[session]->codec_type = AVMEDIA_TYPE_AUDIO;

	ffmpeg_result = avcodec_open2(util_audio_encoder_context[session], util_audio_encoder_codec[session], NULL);
	if(ffmpeg_result != 0)
	{
		result.error_description = "avcodec_open2() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}
	
	util_audio_encoder_packet[session] = av_packet_alloc();
	util_audio_encoder_raw_data[session] = av_frame_alloc();
	if(!util_audio_encoder_raw_data[session] || !util_audio_encoder_packet)
	{
		result.error_description = "av_packet_alloc() / av_frame_alloc() failed";
		goto fail;
	}
	
	util_audio_encoder_raw_data[session]->nb_samples = util_audio_encoder_context[session]->frame_size;
	util_audio_encoder_raw_data[session]->format = util_audio_encoder_context[session]->sample_fmt;
	util_audio_encoder_raw_data[session]->channel_layout = util_audio_encoder_context[session]->channel_layout;

	ffmpeg_result = av_frame_get_buffer(util_audio_encoder_raw_data[session], 0);
	if(ffmpeg_result != 0)
	{
		result.error_description = "av_frame_get_buffer() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	av_frame_make_writable(util_audio_encoder_raw_data[session]);

	util_audio_encoder_swr_context[session] = swr_alloc();
	swr_alloc_set_opts(util_audio_encoder_swr_context[session], av_get_default_channel_layout(util_audio_encoder_context[session]->channels), util_audio_encoder_context[session]->sample_fmt, util_audio_encoder_context[session]->sample_rate,
	av_get_default_channel_layout(util_audio_encoder_context[session]->channels), AV_SAMPLE_FMT_S16, original_samplerate, 0, NULL);
	if(!util_audio_encoder_swr_context[session])
	{
		result.error_description = "swr_alloc_set_opts() failed";
		goto fail;
	}

	ffmpeg_result = swr_init(util_audio_encoder_swr_context[session]);
	if(ffmpeg_result != 0)
	{
		result.error_description = "swr_init() failed";
		goto fail;
	}

	return result;

	fail:
	Util_exit_audio_encoder(session);
	result.code = FFMPEG_RETURNED_NOT_SUCCESS;
	result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
	return result;
}

Result_with_string Util_encode_audio(int size, u8* raw_data, int* encoded_size, u8* encoded_data, int session)
{
	int encode_offset = 0;
	int encoded_offset = 0;
	int ffmpeg_result = 0;
	int one_frame_size = av_samples_get_buffer_size(NULL, util_audio_encoder_context[session]->channels, util_audio_encoder_context[session]->frame_size, util_audio_encoder_context[session]->sample_fmt, 0);
	int out_samples = 0;
	u8* swr_in_cache[1] = { NULL, };
	u8* swr_out_cache[1] = { NULL, };
	Result_with_string result;

	*encoded_size = 0;
	swr_in_cache[0] = (u8*)malloc(size);
	swr_out_cache[0] = (u8*)malloc(size * av_get_bytes_per_sample(util_audio_encoder_context[session]->sample_fmt));
	if(swr_in_cache[0] == NULL || swr_out_cache[0] == NULL)
		goto fail_;

	memcpy(swr_in_cache[0] , raw_data, size);
	out_samples = swr_convert(util_audio_encoder_swr_context[session], (uint8_t**)swr_out_cache, size, (const uint8_t**)swr_in_cache, size);
	free(swr_in_cache[0]);
	swr_in_cache[0] = NULL;
	out_samples *= av_get_bytes_per_sample(util_audio_encoder_context[session]->sample_fmt) / 2;
	
	for(int i = 0; i < 100000; i++)
	{
		util_audio_encoder_raw_data[session]->data[0] = swr_out_cache[0]  + encode_offset;

		ffmpeg_result = avcodec_send_frame(util_audio_encoder_context[session], util_audio_encoder_raw_data[session]);
		if(ffmpeg_result != 0)
		{
			result.error_description = "avcodec_send_frame() failed " + std::to_string(ffmpeg_result);
			goto fail;
		}

		ffmpeg_result = avcodec_receive_packet(util_audio_encoder_context[session], util_audio_encoder_packet[session]);
		if(ffmpeg_result == 0)
		{
			memcpy(encoded_data + encoded_offset, util_audio_encoder_packet[session]->data, util_audio_encoder_packet[session]->size);
			encoded_offset += util_audio_encoder_packet[session]->size;
			av_packet_unref(util_audio_encoder_packet[session]);
		}

		out_samples -= one_frame_size;
		encode_offset += one_frame_size;
		if(one_frame_size*2 > out_samples)
			break;
	}
	*encoded_size = encoded_offset;
	free(swr_out_cache[0]);
	swr_out_cache[0] = NULL;

	return result;

	fail:

	result.code = FFMPEG_RETURNED_NOT_SUCCESS;
	result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
	return result;

	fail_:

	free(swr_in_cache[0]);
	free(swr_out_cache[0]);
	swr_in_cache[0] = NULL;
	swr_out_cache[0] = NULL;
	result.code = OUT_OF_MEMORY;
	result.string = Err_query_template_summary(OUT_OF_MEMORY);
	result.error_description = Err_query_template_detail(OUT_OF_MEMORY);
	return result;
}

void Util_exit_audio_encoder(int session)
{
	avcodec_free_context(&util_audio_encoder_context[session]);
	av_packet_free(&util_audio_encoder_packet[session]);
	av_frame_free(&util_audio_encoder_raw_data[session]);
	swr_free(&util_audio_encoder_swr_context[session]);	
}

Result_with_string Util_open_file(std::string file_path, bool* has_audio, bool* has_video, int session)
{
	int ffmpeg_result = 0;
	Result_with_string result;
	*has_audio = false;
	*has_video = false;

	util_decoder_format_context[session] = avformat_alloc_context();
	ffmpeg_result = avformat_open_input(&util_decoder_format_context[session], file_path.c_str(), NULL, NULL);
	if(ffmpeg_result != 0)
	{
		result.error_description = "avformat_open_input() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	ffmpeg_result = avformat_find_stream_info(util_decoder_format_context[session], NULL);
	if(util_decoder_format_context[session] == NULL)
	{
		result.error_description = "avformat_find_stream_info() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	util_audio_decoder_stream_num[session] = -1;
	for(int i = 0; i < (int)util_decoder_format_context[session]->nb_streams; i++)
	{
		if(util_decoder_format_context[session]->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			*has_audio = true;
			util_audio_decoder_stream_num[session] = i;
		}
		else if(util_decoder_format_context[session]->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			*has_video = true;
			util_video_decoder_stream_num[session] = i;
		}
	}

	if(util_audio_decoder_stream_num[session] == -1 && util_video_decoder_stream_num[session] == -1)
	{
		result.error_description = "No audio and video data";
		goto fail;
	}
	return result;

	fail:

	Util_exit_audio_decoder(session);
	result.code = FFMPEG_RETURNED_NOT_SUCCESS;
	result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
	return result;
}

Result_with_string Util_init_audio_decoder(int session)
{
	int ffmpeg_result = 0;
	Result_with_string result;

	util_audio_decoder_codec[session] = avcodec_find_decoder(util_decoder_format_context[session]->streams[util_audio_decoder_stream_num[session]]->codecpar->codec_id);
	if(!util_audio_decoder_codec[session])
	{
		result.error_description = "avcodec_find_decoder() failed";
		goto fail;
	}

	util_audio_decoder_context[session] = avcodec_alloc_context3(util_audio_decoder_codec[session]);
	if(!util_audio_decoder_context[session])
	{
		result.error_description = "avcodec_alloc_context3() failed";
		goto fail;
	}

	ffmpeg_result = avcodec_parameters_to_context(util_audio_decoder_context[session], util_decoder_format_context[session]->streams[util_audio_decoder_stream_num[session]]->codecpar);
	if(ffmpeg_result != 0)
	{
		result.error_description = "avcodec_parameters_to_context() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	ffmpeg_result = avcodec_open2(util_audio_decoder_context[session], util_audio_decoder_codec[session], NULL);
	if(ffmpeg_result != 0)
	{
		result.error_description = "avcodec_open2() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	util_audio_decoder_swr_context[session] = swr_alloc();
	swr_alloc_set_opts(util_audio_decoder_swr_context[session], av_get_default_channel_layout(util_audio_decoder_context[session]->channels), AV_SAMPLE_FMT_S16, util_audio_decoder_context[session]->sample_rate,
		av_get_default_channel_layout(util_audio_decoder_context[session]->channels), util_audio_decoder_context[session]->sample_fmt, util_audio_decoder_context[session]->sample_rate, 0, NULL);
	if(!util_audio_decoder_swr_context[session])
	{
		result.error_description = "swr_alloc_set_opts() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	ffmpeg_result = swr_init(util_audio_decoder_swr_context[session]);
	if(ffmpeg_result != 0)
	{
		result.error_description = "swr_init() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	return result;

	fail:

	Util_exit_audio_decoder(session);
	result.code = FFMPEG_RETURNED_NOT_SUCCESS;
	result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
	return result;
}

Result_with_string Util_init_video_decoder(int low_resolution, int session)
{
	int ffmpeg_result = 0;
	Result_with_string result;

	util_video_decoder_codec[session] = avcodec_find_decoder(util_decoder_format_context[session]->streams[util_video_decoder_stream_num[session]]->codecpar->codec_id);
	if(!util_video_decoder_codec[session])
	{
		result.error_description = "avcodec_find_decoder() failed";
		goto fail;
	}

	util_video_decoder_context[session] = avcodec_alloc_context3(util_video_decoder_codec[session]);
	if(!util_video_decoder_context[session])
	{
		result.error_description = "avcodec_alloc_context3() failed";
		goto fail;
	}

	ffmpeg_result = avcodec_parameters_to_context(util_video_decoder_context[session], util_decoder_format_context[session]->streams[util_video_decoder_stream_num[session]]->codecpar);
	if(ffmpeg_result != 0)
	{
		result.error_description = "avcodec_parameters_to_context() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	util_video_decoder_context[session]->lowres = low_resolution;
	ffmpeg_result = avcodec_open2(util_video_decoder_context[session], util_video_decoder_codec[session], NULL);
	if(ffmpeg_result != 0)
	{
		result.error_description = "avcodec_open2() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	return result;

	fail:

	Util_exit_video_decoder(session);
	result.code = FFMPEG_RETURNED_NOT_SUCCESS;
	result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
	return result;
}

void Util_get_audio_info(int* bitrate, int* samplerate, int* ch, std::string* format_name, double* duration, int session)
{
	*bitrate = util_audio_decoder_context[session]->bit_rate;
	*samplerate = util_audio_decoder_context[session]->sample_rate;
	*ch = util_audio_decoder_context[session]->channels;
	*format_name = util_audio_decoder_codec[session]->name;
	*duration = (double)util_decoder_format_context[session]->duration / AV_TIME_BASE;
}

void Util_get_video_info(int* width, int* height, double* framerate, std::string* format_name, double* duration, int session)
{
	*width = util_video_decoder_context[session]->width;
	*height = util_video_decoder_context[session]->height;
	*framerate = (double)util_decoder_format_context[session]->streams[util_video_decoder_stream_num[session]]->avg_frame_rate.num / util_decoder_format_context[session]->streams[util_video_decoder_stream_num[session]]->avg_frame_rate.den;
	*format_name = util_video_decoder_codec[session]->name;
	*duration = (double)util_decoder_format_context[session]->duration / AV_TIME_BASE;
}

Result_with_string Util_read_packet(AVMediaType* type, int session)
{
	Result_with_string result;
	int ffmpeg_result = 0;
	AVPacket* cache_packet = NULL;
	*type = AVMEDIA_TYPE_UNKNOWN;

	cache_packet = av_packet_alloc();
	if(!cache_packet)
	{
		result.error_description = "av_packet_alloc() failed";
		goto fail;
	}

	ffmpeg_result = av_read_frame(util_decoder_format_context[session], cache_packet);
	if(ffmpeg_result != 0)
	{
		result.error_description = "av_read_frame() failed";
		goto fail;
	}

	if(cache_packet->stream_index == util_audio_decoder_stream_num[session])//audio packet
	{
		util_audio_decoder_cache_packet[session] = av_packet_alloc();
		if(!util_audio_decoder_cache_packet[session])
		{
			result.error_description = "av_packet_alloc() failed" + std::to_string(ffmpeg_result);
			goto fail;
		}

		av_packet_unref(util_audio_decoder_cache_packet[session]);
		ffmpeg_result = av_packet_ref(util_audio_decoder_cache_packet[session], cache_packet);
		if(ffmpeg_result != 0)
		{
			result.error_description = "av_packet_ref() failed" + std::to_string(ffmpeg_result);
			goto fail;
		}

		*type = AVMEDIA_TYPE_AUDIO;
	}
	else if(cache_packet->stream_index == util_video_decoder_stream_num[session])//video packet
	{
		util_video_decoder_cache_packet[session] = av_packet_alloc();
		if(!util_video_decoder_cache_packet[session])
		{
			result.error_description = "av_packet_alloc() failed";
			goto fail;
		}

		av_packet_unref(util_video_decoder_cache_packet[session]);
		ffmpeg_result = av_packet_ref(util_video_decoder_cache_packet[session], cache_packet);
		if(ffmpeg_result != 0)
		{
			result.error_description = "av_packet_ref() failed" + std::to_string(ffmpeg_result);
			goto fail;
		}

		*type = AVMEDIA_TYPE_VIDEO;
	}

	av_packet_free(&cache_packet);
	return result;

	fail:

	av_packet_free(&util_audio_decoder_cache_packet[session]);
	av_packet_free(&util_video_decoder_cache_packet[session]);
	av_packet_free(&cache_packet);
	result.code = FFMPEG_RETURNED_NOT_SUCCESS;
	result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
	return result;
}

Result_with_string Util_ready_audio_packet(int session)
{
	Result_with_string result;
	int ffmpeg_result = 0;

	av_packet_free(&util_audio_decoder_packet[session]);
	util_audio_decoder_packet[session] = av_packet_alloc();
	if(!util_audio_decoder_packet[session])
	{
		result.error_description = "av_packet_alloc() failed";
		goto fail;
	}

	av_packet_unref(util_audio_decoder_packet[session]);
	ffmpeg_result = av_packet_ref(util_audio_decoder_packet[session], util_audio_decoder_cache_packet[session]);
	if(ffmpeg_result != 0)
	{
		result.error_description = "av_packet_ref() failed" + std::to_string(ffmpeg_result);
		goto fail;
	}

	av_packet_free(&util_audio_decoder_cache_packet[session]);
	return result;

	fail:

	av_packet_free(&util_audio_decoder_packet[session]);
	result.code = FFMPEG_RETURNED_NOT_SUCCESS;
	result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
	return result;
}

Result_with_string Util_ready_video_packet(int session)
{
	Result_with_string result;
	int ffmpeg_result = 0;

	av_packet_free(&util_video_decoder_packet[session]);
	util_video_decoder_packet[session] = av_packet_alloc();
	if(!util_video_decoder_packet[session])
	{
		result.error_description = "av_packet_alloc() failed";
		goto fail;
	}

	av_packet_unref(util_video_decoder_packet[session]);
	ffmpeg_result = av_packet_ref(util_video_decoder_packet[session], util_video_decoder_cache_packet[session]);
	if(ffmpeg_result != 0)
	{
		result.error_description = "av_packet_ref() failed" + std::to_string(ffmpeg_result);
		goto fail;
	}

	av_packet_free(&util_video_decoder_cache_packet[session]);
	return result;

	fail:

	av_packet_free(&util_video_decoder_packet[session]);
	result.code = FFMPEG_RETURNED_NOT_SUCCESS;
	result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
	return result;
}

Result_with_string Util_decode_audio(int* size, u8** raw_data, int session)
{
	int ffmpeg_result = 0;
	Result_with_string result;
	*size = 0;

	util_audio_decoder_raw_data[session] = av_frame_alloc();
	if(!util_audio_decoder_raw_data[session])
	{
		result.error_description = "av_frame_alloc() failed";
		goto fail;
	}
	
	ffmpeg_result = avcodec_send_packet(util_audio_decoder_context[session], util_audio_decoder_packet[session]);
	if(ffmpeg_result == 0)
	{
		ffmpeg_result = avcodec_receive_frame(util_audio_decoder_context[session], util_audio_decoder_raw_data[session]);
		if(ffmpeg_result == 0)
		{
			*raw_data = (u8*)malloc(util_audio_decoder_raw_data[session]->nb_samples * 2 * util_audio_decoder_context[session]->channels);
			*size = swr_convert(util_audio_decoder_swr_context[session], raw_data, util_audio_decoder_raw_data[session]->nb_samples, (const uint8_t**)util_audio_decoder_raw_data[session]->data, util_audio_decoder_raw_data[session]->nb_samples);
			*size *= 2;
		}
		else
		{
			result.error_description = "avcodec_receive_frame() failed " + std::to_string(ffmpeg_result);
			goto fail;
		}
	}
	else
	{
		result.error_description = "avcodec_send_packet() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	av_packet_free(&util_audio_decoder_packet[session]);
	av_frame_free(&util_audio_decoder_raw_data[session]);
	return result;

	fail:

	av_packet_free(&util_audio_decoder_packet[session]);
	av_frame_free(&util_audio_decoder_raw_data[session]);
	result.code = FFMPEG_RETURNED_NOT_SUCCESS;
	result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
	return result;
}

Result_with_string Util_decode_video(int* width, int* height, bool* key_frame, double* current_pos, int session)
{
	int ffmpeg_result = 0;
	int count = 0;
	double framerate = (double)util_decoder_format_context[session]->streams[util_video_decoder_stream_num[session]]->avg_frame_rate.num / util_decoder_format_context[session]->streams[util_video_decoder_stream_num[session]]->avg_frame_rate.den;
	double current_frame = (double)util_video_decoder_packet[session]->dts / util_video_decoder_packet[session]->duration;
	Result_with_string result;
	*width = 0;
	*height = 0;
	*current_pos = 0;
	if(framerate != 0.0)
		*current_pos = current_frame * (1000 / framerate);//calc frame pos
	//Log_log_save("", std::to_string(framerate) + " " + std::to_string(current_frame), 1234567890, false);

	if(util_video_decoder_packet[session]->flags == 1)
		*key_frame = true;
	else
		*key_frame = false;
	
	util_video_decoder_raw_data[session][util_video_decoder_buffer_num[session]] = av_frame_alloc();
	if(!util_video_decoder_raw_data[session][util_video_decoder_buffer_num[session]])
	{
		result.error_description = "av_frame_alloc() failed";
		goto fail;
	}

	ffmpeg_result = avcodec_send_packet(util_video_decoder_context[session], util_video_decoder_packet[session]);
	if(ffmpeg_result == 0)
	{
		ffmpeg_result = avcodec_receive_frame(util_video_decoder_context[session], util_video_decoder_raw_data[session][util_video_decoder_buffer_num[session]]);
		if(ffmpeg_result == 0)
		{
			*width = util_video_decoder_raw_data[session][util_video_decoder_buffer_num[session]]->width;
			*height = util_video_decoder_raw_data[session][util_video_decoder_buffer_num[session]]->height;
		}
		else
		{
			result.error_description = "avcodec_receive_frame() failed " + std::to_string(ffmpeg_result);
			goto fail;
		}
	}
	else
	{
		result.error_description = "avcodec_send_packet() failed " + std::to_string(ffmpeg_result);
		goto fail;
	}

	if(util_video_decoder_buffer_num[session] == 0)
		util_video_decoder_buffer_num[session] = 1;
	else
		util_video_decoder_buffer_num[session] = 0;
	
	while(util_video_decoder_lock[session][util_video_decoder_buffer_num[session]])
	{
		count++;
		if(count > 40000)//time out 4000ms
			break;
	
		usleep(100);
	}

	if(util_video_decoder_buffer_num[session] == 0)
		util_video_decoder_ready_buffer_num[session] = 1;
	else
		util_video_decoder_ready_buffer_num[session] = 0;

	av_packet_free(&util_video_decoder_packet[session]);
	av_frame_free(&util_video_decoder_raw_data[session][util_video_decoder_buffer_num[session]]);
	return result;

	fail:

	av_packet_free(&util_video_decoder_packet[session]);
	av_frame_free(&util_video_decoder_raw_data[session][util_video_decoder_buffer_num[session]]);
	result.code = FFMPEG_RETURNED_NOT_SUCCESS;
	result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
	return result;
}

void Util_get_video_image(u8* raw_data, int width, int height, int session)
{
	int cpy_size[2] = { 0, 0, };
	int buffer_num = util_video_decoder_ready_buffer_num[session];
	util_video_decoder_lock[session][buffer_num] = true;//lock

	cpy_size[0] = (width * height);
	cpy_size[1] = cpy_size[0] / 4;
	cpy_size[0] -= cpy_size[0] % 32;
	cpy_size[1] -= cpy_size[1] % 32;
	memcpy_asm(raw_data, util_video_decoder_raw_data[session][buffer_num]->data[0], cpy_size[0]);
	memcpy_asm(raw_data + (width * height), util_video_decoder_raw_data[session][buffer_num]->data[1], cpy_size[1]);
	memcpy_asm(raw_data + (width * height) + (width * height / 4), util_video_decoder_raw_data[session][buffer_num]->data[2], cpy_size[1]);

	util_video_decoder_lock[session][buffer_num] = false;//unlock
}

Result_with_string Util_seek(u64 seek_pos, int flag, int session)
{
	int ffmpeg_result;
	Result_with_string result;

	ffmpeg_result = avformat_seek_file(util_decoder_format_context[session], -1, seek_pos, seek_pos, seek_pos, flag);//AVSEEK_FLAG_FRAME 8 AVSEEK_FLAG_ANY 4  AVSEEK_FLAG_BACKWORD 1
	if(ffmpeg_result < 0)
	{
		result.code = FFMPEG_RETURNED_NOT_SUCCESS;
		result.string = Err_query_template_summary(FFMPEG_RETURNED_NOT_SUCCESS);
		result.error_description = "avformat_seek_file() failed " + std::to_string(ffmpeg_result);
	}

	return result;
}

void Util_close_file(int session)
{
	avformat_close_input(&util_decoder_format_context[session]);
}

void Util_exit_audio_decoder(int session)
{
	avcodec_free_context(&util_audio_decoder_context[session]);
	av_packet_free(&util_audio_decoder_packet[session]);
	av_packet_free(&util_audio_decoder_cache_packet[session]);
	av_frame_free(&util_audio_decoder_raw_data[session]);
	swr_free(&util_audio_decoder_swr_context[session]);
}

void Util_exit_video_decoder(int session)
{
	avcodec_free_context(&util_video_decoder_context[session]);
	av_packet_free(&util_video_decoder_packet[session]);
	av_packet_free(&util_video_decoder_cache_packet[session]);
	av_frame_free(&util_video_decoder_raw_data[session][0]);
	av_frame_free(&util_video_decoder_raw_data[session][1]);
}
