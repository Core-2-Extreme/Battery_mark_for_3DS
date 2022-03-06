#pragma once

//settings
#define DEF_MAIN_DIR (std::string)"/Battery_mark/"
#define DEF_UPDATE_DIR_PREFIX (std::string)"/3ds/Battery_mark_ver_"
#define DEF_UPDATE_FILE_PREFIX (std::string)"battery_mark"
#define DEF_CHECK_INTERNET_URL (std::string)"https://connectivitycheck.gstatic.com/generate_204"
#define DEF_SEND_APP_INFO_URL (std::string)"https://script.google.com/macros/s/AKfycbyn_blFyKWXCgJr6NIF8x6ETs7CHRN5FXKYEAAIrzV6jPYcCkI/exec"
#define DEF_CHECK_UPDATE_URL (std::string)"https://script.google.com/macros/s/AKfycbz8Nsobjwc9IslWkIEB_2gyc76bTyJFwod2SZJbeVuhu_HbfQiszty8/exec"
#define DEF_HTTP_USER_AGENT (std::string)"battery mark for 3ds " + DEF_CURRENT_APP_VER
#define DEF_CURRENT_APP_VER (std::string)"v2.1.0-d"
#define DEF_CURRENT_APP_VER_INT 2064
#define DEF_ENABLE_BMARK
#define DEF_ENABLE_BMR
//#define DEF_ENABLE_SUB_APP2
//#define DEF_ENABLE_SUB_APP3
//#define DEF_ENABLE_SUB_APP4
//#define DEF_ENABLE_SUB_APP5
//#define DEF_ENABLE_SUB_APP6
//#define DEF_ENABLE_SUB_APP7
#define DEF_DRAW_MAX_NUM_OF_SPRITE_SHEETS 128
#define DEF_DECODER_MAX_CACHE_PACKETS 256
#define DEF_DECODER_MAX_AUDIO_TRACKS 8
#define DEF_DECODER_MAX_VIDEO_TRACKS 2
#define DEF_DECODER_MAX_SUBTITLE_TRACKS 8
#define DEF_DECODER_MAX_RAW_IMAGE 128
#define DEF_DECODER_MAX_SUBTITLE_DATA 16
#define DEF_DECODER_MAX_SESSIONS 2
#define DEF_ENCODER_MAX_SESSIONS 2
#define DEF_EXPL_MAX_FILES 1024
#define DEF_HTTP_POST_BUFFER_SIZE 0x100000
#define DEF_LOG_BUFFER_LINES 512
#define DEF_LOG_COLOR 0xFF00D000
#define DEF_MUXER_MAX_SESSIONS 2
#define DEF_SPEAKER_MAX_BUFFERS 192
#define DEF_SWKBD_MAX_DIC_WORDS 128

//battery mark
#define DEF_BMARK_BMR_RANKING_SERVER_URL (std::string)"https://script.google.com/macros/s/AKfycbxo4iwCbxtI2ZuYQP7bKveRdHx6kPTH4rZ8Pg8nUguIq_7zcoooEUszJQ/exec"
#define DEF_BMARK_BMR_NUM_OF_HISTORY 280

#define DEF_BMARK_NUM_OF_MSG 33
//#define DEF_BMARK_ENABLE_ICON
#define DEF_BMARK_ENABLE_NAME
#define DEF_BMARK_ICON_PATH (std::string)"romfs:/"
#define DEF_BMARK_NAME (std::string)"Battery\nmark"
#define DEF_BMARK_VER (std::string)"v2.1.0-d"
#define DEF_BMARK_MAIN_STR (std::string)"Bmark/Main"
#define DEF_BMARK_INIT_STR (std::string)"Bmark/Init"
#define DEF_BMARK_EXIT_STR (std::string)"Bmark/Exit"
#define DEF_BMARK_WATCH_THREAD_STR (std::string)"Bmark/Watch thread"
#define DEF_BMARK_VIDEO_THREAD_STR (std::string)"Bmark/Video thread"
#define DEF_BMARK_COPY_THREAD_STR (std::string)"Bmark/Copy thread"
#define DEF_BMARK_UPDATE_THREAD_STR (std::string)"Bmark/Update thread"

#define DEF_BMARK_BATTERY_MSG 0
#define DEF_BMARK_BATTERY_TIME_MSG 1
#define DEF_BMARK_DO_NOT_CHARGE_MSG 2
#define DEF_BMARK_ABORTED_MSG 3
#define DEF_BMARK_BATTERY_TIME_MAX_MSG 4
#define DEF_BMARK_BATTERY_TIME_AVG_MSG 5
#define DEF_BMARK_BATTERY_TIME_MIN_MSG 6
#define DEF_BMARK_ELAPSED_TIME_MSG 7
#define DEF_BMARK_TOTAL_ELAPSED_TIME_MSG 8
#define DEF_BMARK_REMAINING_TIME_MSG 9
#define DEF_BMARK_START_MSG 10
#define DEF_BMARK_STOP_MSG 11
#define DEF_BMARK_BATTERY_MARK_IN_PROGRESS_MSG 12
#define DEF_BMARK_RESULT_TITLE_MSG 13
#define DEF_BMARK_CALCULATING_MSG 14
#define DEF_BMARK_NO_ENOUGH_BATTERY_MSG 15
#define DEF_BMARK_NO_ENOUGH_BATTERY_DESCRIPTION_0_MSG 16
#define DEF_BMARK_NO_ENOUGH_BATTERY_DESCRIPTION_1_MSG 17
#define DEF_BMARK_BATTERY_VOLTAGE_MSG 18
#define DEF_BMARK_SEND_DATA_MSG 19
#define DEF_BMARK_OFF_MSG 20
#define DEF_BMARK_ON_MSG 21
#define DEF_BMARK_USER_NAME_HINT_MSG 22
#define DEF_BMARK_SUBMITING_RESULT_MSG 23
#define DEF_BMARK_DO_NOT_CLOSE_MSG 24
#define DEF_BMARK_TIME_MSG 25
#define DEF_BMARK_ABORT_CONFIRMATION_MSG 26
#define DEF_BMARK_YES_MSG 27
#define DEF_BMARK_NO_MSG 28
#define DEF_BMARK_BATTERY_TEMP_MSG 29
#define DEF_BMARK_GRAPH_BATTERY_LEVEL_MSG 30
#define DEF_BMARK_GRAPH_BATTERY_TEMP_MSG 31
#define DEF_BMARK_GRAPH_BATTERY_VOLTAGE_MSG 32

//battery mark ranking
#define DEF_BMR_NUM_OF_MSG 24
#define DEF_BMR_NUM_OF_LOGS 200
//#define DEF_BMR_ENABLE_ICON
#define DEF_BMR_ENABLE_NAME
#define DEF_BMR_ICON_PATH (std::string)"romfs:/"
#define DEF_BMR_NAME (std::string)"Battery\nmark\nranking"
#define DEF_BMR_VER (std::string)"v1.1.0-d"
#define DEF_BMR_MAIN_STR (std::string)"Bmr/Main"
#define DEF_BMR_INIT_STR (std::string)"Bmr/Init"
#define DEF_BMR_EXIT_STR (std::string)"Bmr/Exit"
#define DEF_BMR_WORKER_THREAD_STR (std::string)"Bmr/Worker thread"

#define DEF_BMR_MODEL_MSG 0
#define DEF_BMR_TOTAL_MSG 1
#define DEF_BMR_AVG_MSG 2
#define DEF_BMR_MAX_MSG 3
#define DEF_BMR_MIN_MSG 4
#define DEF_BMR_USER_NAME_MSG 5
#define DEF_BMR_APP_VER_MSG 6
#define DEF_BMR_SYS_VER_MSG 7
#define DEF_BMR_DATE_MSG 8
#define DEF_BMR_MODEL_SELECTION_MSG 9
#define DEF_BMR_DL_RANKING_MSG 16
#define DEF_BMR_CONTROLS_MSG 17
#define DEF_BMR_PAGE_MSG 18
#define DEF_BMR_DL_MSG 19
#define DEF_BMR_BATTERY_LEVEL_MSG 20
#define DEF_BMR_BATTERY_TEMP_MSG 21
#define DEF_BMR_BATTERY_VOLTAGE_MSG 22
#define DEF_BMR_CLOSE_MSG 23

//sample app 2
#define DEF_SAPP2_NUM_OF_MSG 1
//#define DEF_SAPP2_ENABLE_ICON
#define DEF_SAPP2_ENABLE_NAME
#define DEF_SAPP2_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP2_NAME (std::string)"sample 2"
#define DEF_SAPP2_VER (std::string)"v0.0.1"
#define DEF_SAPP2_MAIN_STR (std::string)"Sapp2/Main"
#define DEF_SAPP2_INIT_STR (std::string)"Sapp2/Init"
#define DEF_SAPP2_EXIT_STR (std::string)"Sapp2/Exit"
#define DEF_SAPP2_WORKER_THREAD_STR (std::string)"Sapp2/Worker thread"

//sample app 3
#define DEF_SAPP3_NUM_OF_MSG 1
//#define DEF_SAPP3_ENABLE_ICON
#define DEF_SAPP3_ENABLE_NAME
#define DEF_SAPP3_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP3_NAME (std::string)"sample 3"
#define DEF_SAPP3_VER (std::string)"v0.0.1"
#define DEF_SAPP3_MAIN_STR (std::string)"Sapp3/Main"
#define DEF_SAPP3_INIT_STR (std::string)"Sapp3/Init"
#define DEF_SAPP3_EXIT_STR (std::string)"Sapp3/Exit"
#define DEF_SAPP3_WORKER_THREAD_STR (std::string)"Sapp3/Worker thread"

//sample app 4
#define DEF_SAPP4_NUM_OF_MSG 1
//#define DEF_SAPP4_ENABLE_ICON
#define DEF_SAPP4_ENABLE_NAME
#define DEF_SAPP4_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP4_NAME (std::string)"sample 4"
#define DEF_SAPP4_VER (std::string)"v0.0.1"
#define DEF_SAPP4_MAIN_STR (std::string)"Sapp4/Main"
#define DEF_SAPP4_INIT_STR (std::string)"Sapp4/Init"
#define DEF_SAPP4_EXIT_STR (std::string)"Sapp4/Exit"
#define DEF_SAPP4_WORKER_THREAD_STR (std::string)"Sapp4/Worker thread"

//sample app 5
#define DEF_SAPP5_NUM_OF_MSG 1
//#define DEF_SAPP5_ENABLE_ICON
#define DEF_SAPP5_ENABLE_NAME
#define DEF_SAPP5_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP5_NAME (std::string)"sample 5"
#define DEF_SAPP5_VER (std::string)"v0.0.1"
#define DEF_SAPP5_MAIN_STR (std::string)"Sapp5/Main"
#define DEF_SAPP5_INIT_STR (std::string)"Sapp5/Init"
#define DEF_SAPP5_EXIT_STR (std::string)"Sapp5/Exit"
#define DEF_SAPP5_WORKER_THREAD_STR (std::string)"Sapp5/Worker thread"

//sample app 6
#define DEF_SAPP6_NUM_OF_MSG 1
//#define DEF_SAPP6_ENABLE_ICON
#define DEF_SAPP6_ENABLE_NAME
#define DEF_SAPP6_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP6_NAME (std::string)"sample 6"
#define DEF_SAPP6_VER (std::string)"v0.0.1"
#define DEF_SAPP6_MAIN_STR (std::string)"Sapp6/Main"
#define DEF_SAPP6_INIT_STR (std::string)"Sapp6/Init"
#define DEF_SAPP6_EXIT_STR (std::string)"Sapp6/Exit"
#define DEF_SAPP6_WORKER_THREAD_STR (std::string)"Sapp6/Worker thread"

//sample app 7
#define DEF_SAPP7_NUM_OF_MSG 1
//#define DEF_SAPP7_ENABLE_ICON
#define DEF_SAPP7_ENABLE_NAME
#define DEF_SAPP7_ICON_PATH (std::string)"romfs:/"
#define DEF_SAPP7_NAME (std::string)"sample 7"
#define DEF_SAPP7_VER (std::string)"v0.0.1"
#define DEF_SAPP7_MAIN_STR (std::string)"Sapp7/Main"
#define DEF_SAPP7_INIT_STR (std::string)"Sapp7/Init"
#define DEF_SAPP7_EXIT_STR (std::string)"Sapp7/Exit"
#define DEF_SAPP7_WORKER_THREAD_STR (std::string)"Sapp7/Worker thread"
