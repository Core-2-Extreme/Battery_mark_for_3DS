#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define Thread_max 4
#define STACKSIZE (8 * 1024)

volatile unsigned long long int count[Thread_max];
volatile bool cpu_load = false;
volatile bool cpu_load_started[4] = { false };
volatile bool timer_run = false;
volatile bool timer_flag = false;
volatile int thread_num = 2;

char ver[] = "v1.1.1 ";
char* battery_mark_mode[] = { "\x1b[36m   Full check   \x1b[30m","\x1b[36m  Medium check  \x1b[30m","\x1b[36m  Short check   \x1b[30m","\x1b[36mVery short check\x1b[30m","\x1b[36m  Custom check  \x1b[30m" };
char* battery_mark_run_msg[] = { "\x1b[32mRunning     \x1b[30m","\x1b[32mRunning.    \x1b[30m","\x1b[32mRunning..   \x1b[30m","\x1b[32mRunning...  \x1b[30m","\x1b[32mRunning.... \x1b[30m","\x1b[32mRunning.....\x1b[30m", };
double battery_mark_battery_voltage_start = 0;
double battery_mark_battery_voltage_end = 0;
int battery_mark_battery_level_start = -1;
int battery_mark_custom_percent = 1;
int battery_mark_mode_percent[] = { 85, 30, 10, 5, 1};
int battery_mark_count = 0;
int battery_mark_battery_before = -1;
int battery_mark_time_count = -1;
int battery_mark_time_count_seconds = -1;
int battery_mark_time_count_minutes = -1;
int battery_mark_total_time = -1;
int battery_mark_total_calculate_time = -1;
int battery_mark_total_time_seconds = -1;
int battery_mark_total_time_minutes = -1;
int battery_mark_total_time_hours = -1;
int battery_mark_left_time = -1;
int battery_mark_left_calculate_time = -1;
int battery_mark_left_time_seconds = -1;
int battery_mark_left_time_minutes = -1;
int battery_mark_left_time_hours = -1;
int battery_mark_min_time = 100000;
int battery_mark_min_time_seconds = -1;
int battery_mark_min_time_minutes = -1;
int battery_mark_max_time = -1;
int battery_mark_max_time_seconds = -1;
int battery_mark_max_time_minutes = -1;
int battery_mark_avg_calculate_time = -1;
int battery_mark_avg_time = -1;
int battery_mark_avg_time_seconds = -1;
int battery_mark_avg_time_minutes = -1;
int battery_mark_less_count = -1;
int battery_mark_start_time_hours = -1;
int battery_mark_start_time_minutes = -1;
int battery_mark_start_time_seconds = -1;
int battery_mark_battery_level_end = -1;
int battery_mark_end_time_hours = -1;
int battery_mark_end_time_minutes = -1;
int battery_mark_end_time_seconds = -1;
int battery_mark_select_num = 0;
bool battery_mark_run = false;
bool battery_mark_running = false;
bool battery_mark_finish = false;
bool battery_mark_abort = false;
bool battery_mark_full = false;
bool battery_mark_medium = false;
bool battery_mark_short = false;
bool battery_mark_very_short = false;
bool battery_mark_custom = false;

u8 battery_level = -1;
u8 check_battery_level = -1;
int battery_0 = 0;
int battery_1 = 0;
u8 battery_voltage = -1;
int battery_v_0 = 0;
int battery_v_1 = 0;
int battery_v_2 = 0;
int battery_v_3 = 0;
u8 battery_charge = -1;
int battery_c_0 = 0;
int battery_c_1 = 0;
int battery_c_2 = 0;
u32 cpu_limit = -1;
int main_sleep = 50000;
int thread_sleep = 50000;
int hours = -1;
int hour_0 = 9;
int hour_1 = 9;
int minutes = -1;
int minute_0 = 9;
int minute_1 = 9;
int seconds = -1;
int second_0 = 9;
int second_1 = 9;
int line = 15;
int all_loops = 0;
int loops[Thread_max];
int news_cool_time = 0;

double battery_voltage_calculate = 255;
bool debug = false;
bool update_thread_run = false;
bool input_wait = false;
bool app_exit = false;
bool timer_exit = false;
bool ptmu_init_flag = true;
bool mcu_init_flag = true;
bool apt_init_flag = true;
bool news_init_flag = true;
bool ptmu_init_succes = false;
bool mcu_init_succes = false;
bool apt_init_succes = false;
bool news_init_succes = false;

u16 msg_ = 0x0a;//new line
u16 msg_space = 0x20;//space
u16 msg_percent = 0x25; //%
u16 msg_0 = 0x5b; // [
u16 msg_1 = 0x5d; // ]
u16 msg_2 = 0x3a; // :
u16 msg_3 = 0x3d; // =
u16 msg_4 = 0x2e; // .
u16 msg_5 = 0x2d; // -
u16 msg_6 = 0x3e; // >

u16 _0 = 0x30;
u16 _1 = 0x31;
u16 _2 = 0x32;
u16 _3 = 0x33;
u16 _4 = 0x34;
u16 _5 = 0x35;
u16 _6 = 0x36;
u16 _7 = 0x37;
u16 _8 = 0x38;
u16 _9 = 0x39;

u16 a = 0x61;
u16 b = 0x62;
u16 c = 0x63;
u16 d = 0x64;
u16 e = 0x65;
u16 f = 0x66;
u16 g = 0x67;
u16 h = 0x68;
u16 i = 0x69;
u16 j = 0x6a;
u16 k = 0x6b;
u16 l = 0x6c;
u16 m = 0x6d;
u16 n = 0x6e;
u16 o = 0x6f;
u16 p = 0x70;
u16 q = 0x71;
u16 r = 0x72;
u16 s = 0x73;
u16 t = 0x74;
u16 u = 0x75;
u16 v = 0x76;
u16 w = 0x77;
u16 x = 0x78;
u16 y = 0x79;
u16 z = 0x7a;

u16 A = 0x41;
u16 B = 0x42;
u16 C = 0x43;
u16 D = 0x44;
u16 E = 0x45;
u16 F = 0x46;
u16 G = 0x47;
u16 H = 0x48;
u16 I = 0x49;
u16 J = 0x4a;
u16 K = 0x4b;
u16 L = 0x4c;
u16 M = 0x4d;
u16 N = 0x4e;
u16 O = 0x4f;
u16 P = 0x50;
u16 Q = 0x51;
u16 R = 0x52;
u16 S = 0x53;
u16 T = 0x54;
u16 U = 0x55;
u16 V = 0x56;
u16 W = 0x57;
u16 X = 0x58;
u16 Y = 0x59;
u16 Z = 0x5a;

Thread threads[Thread_max], timer_thread, update_thread;

PrintConsole Screen_top, Screen_bottom;

void Timer_thread(void *arg)
{		
	while (timer_run)
	{
		usleep(1000000);
		if (battery_mark_running)
		{
			battery_mark_time_count = battery_mark_time_count + 1;
			battery_mark_time_count_seconds = battery_mark_time_count_seconds + 1;
			battery_mark_total_time_seconds = battery_mark_total_time_seconds + 1;
			if (battery_mark_time_count_seconds >= 60)
			{
				battery_mark_time_count_seconds = 0;
				battery_mark_time_count_minutes = battery_mark_time_count_minutes + 1;
			}

			if (battery_mark_total_time_seconds >= 60)
			{
				battery_mark_total_time_seconds = 0;
				battery_mark_total_time_minutes = battery_mark_total_time_minutes + 1;
			}
			else if(battery_mark_total_time_minutes >= 60)
			{
				battery_mark_total_time_minutes = 0;
				battery_mark_total_time_hours = battery_mark_total_time_hours + 1;
			}

			if (battery_mark_left_time_seconds >= 1)
			{
				battery_mark_left_time_seconds = battery_mark_left_time_seconds - 1;
			}
			else
			{
				if (battery_mark_left_time_minutes >= 1)
				{
					battery_mark_left_time_seconds = 59;
					battery_mark_left_time_minutes = battery_mark_left_time_minutes - 1;
				}
				else
				{
					if (battery_mark_left_time_hours >= 1)
					{
						battery_mark_left_time_minutes = 59;
						battery_mark_left_time_hours = battery_mark_left_time_hours - 1;
					}
				}
			}
		}

		if (news_cool_time >= 1)
		{
			news_cool_time = news_cool_time - 1;
		}
		timer_flag = true;
	}
	
	if(debug)
	{
		consoleSelect(&Screen_bottom);
		printf("[%02d:%02d:%02d] \x1b[45mTimer_thread(); ended\x1b[40m\n", hours, minutes, seconds);
	}
	timer_exit = true;
}

void CPU_test(void *arg)
{
	cpu_load_started[(int)arg] = true;
	while (cpu_load)
	{
		count[(int)arg]++;
		usleep(0);
	}
}

void CPUload_start(void)
{
	if(debug)
	{
		consoleSelect(&Screen_bottom);
		printf("[%02d:%02d:%02d] \x1b[45mCPU_load_start(); load\x1b[40m\n", hours, minutes, seconds);
	}
	
	int i;
	s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	consoleSelect(&Screen_bottom);
	printf("[%02d:%02d:%02d] CPU load Starting...\n", hours, minutes, seconds);
	usleep(250000);

	for (i = 0; i < thread_num; i++)
	{

		threads[i] = threadCreate(CPU_test, (void*)(i), STACKSIZE, prio + 1, i, false);

		if (debug)
		{
			printf("[%02d:%02d:%02d] \x1b[45mCreated thread %d \x1b[32mOK\x1b[37m\n\x1b[40m", hours, minutes, seconds, i);
		}
	}
	
	if(debug)
	{
		consoleSelect(&Screen_bottom);
		printf("[%02d:%02d:%02d] \x1b[45mCPU_load_start(); ended\x1b[40m\n", hours, minutes, seconds);
	}
}

void CPUload_start_wait(void)
{
	if (debug)
	{
		consoleSelect(&Screen_bottom);
		printf("[%02d:%02d:%02d] \x1b[45mCPU_load_start_wait(); load\x1b[40m\n", hours, minutes, seconds);
	}

	consoleSelect(&Screen_bottom);
	printf("[%02d:%02d:%02d] waiting... \n", hours, minutes, seconds);
	usleep(1000000);

	for (int j = 0; j < thread_num; j++)
	{
		if (cpu_load_started[j])
		{
			printf("[%02d:%02d:%02d] CPU load Started Thread %d \x1b[32mOK\x1b[37m\n", hours, minutes, seconds, j);
		}
		else
		{
			printf("[%02d:%02d:%02d] CPU load Started Thread %d \x1b[31mNG\x1b[37m\n", hours, minutes, seconds, j);
		}
	}

	if (debug)
	{
		consoleSelect(&Screen_bottom);
		printf("[%02d:%02d:%02d] \x1b[45mCPU_load_start_wait(); ended\x1b[40m\n", hours, minutes, seconds);
	}
}

void CPUload_stop(void)
{	
	if(debug)
	{
		consoleSelect(&Screen_bottom);
		printf("[%02d:%02d:%02d] \x1b[45mCPU_load_stop(); load\x1b[40m\n", hours, minutes, seconds);
	}
	
	consoleSelect(&Screen_bottom);
	printf("[%02d:%02d:%02d] CPUload Stoping...\n", hours, minutes, seconds);
	cpu_load = false;
	int i;
	for (i = 0; i < thread_num; i++)
	{
		threadJoin(threads[i], 3000000);
		threadFree(threads[i]);
	}

	printf("[%02d:%02d:%02d] CPUload Stoped \x1b[32mOK\x1b[37m\n", hours, minutes, seconds);
	cpu_load_started[0] = false;
	cpu_load_started[1] = false;
	cpu_load_started[2] = false;
	cpu_load_started[3] = false;
	
	if(debug)
	{
		consoleSelect(&Screen_bottom);
		printf("[%02d:%02d:%02d] \x1b[45mCPU_load_stop(); ended\x1b[40m\n", hours, minutes, seconds);
	}
}

void Get_system_info(void)
{		
	PTMU_GetBatteryChargeState(&battery_charge);//battery charge
	MCUHWC_GetBatteryLevel(&battery_level);//battery level(%)
	MCUHWC_GetBatteryVoltage(&battery_voltage);//battery voltage

	battery_voltage_calculate = battery_voltage;
	battery_voltage_calculate = 5 * battery_voltage_calculate / 256;//Voltage calculation https://www.3dbrew.org/wiki/MCUHWC:GetBatteryVoltage

	//Get time
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);
	hours = timeStruct->tm_hour;
	minutes = timeStruct->tm_min;
	seconds = timeStruct->tm_sec;
}

void Data_update(void)
{
	if (timer_flag)
	{
		all_loops = 0;
		for (int i = 0; i < Thread_max; i++)
		{
			loops[i] = count[i];
			count[i] = 0;
		}
		timer_flag = false;
	}

	consoleSelect(&Screen_top);
	printf("\x1b[1;0H-------------------------------------------------");
	printf("\x1b[3;0H%s", ver);
	printf("\x1b[4;0HBattery Level = %d%% ", battery_level);
	printf("\x1b[5;0HBattery Voltage = %.2fv", battery_voltage_calculate);

	if (battery_charge == 1)
	{
		printf("\x1b[6;0HBattery Charge = \x1b[32mYes \x1b[37m");
	}
	else
	{
		printf("\x1b[6;0HBattery Charge = \x1b[31mNo \x1b[37m");
	}

	printf("\x1b[8;0HPress A to CPULoad START/STOP. (Threads %d)", thread_num);
	printf("\x1b[9;0HPress B to change debug mode.");
	printf("\x1b[10;0HPress Y to change CPU limit.");
	if (news_cool_time == 0)
	{
		printf("\x1b[11;0HPress X to save the current battery state.       ");
	}
	else
	{
		printf("\x1b[11;0HPress X to save the current battery state. (%02ds)", news_cool_time);
	}
	printf("\x1b[12;0HPress L & R to battery mark.");
	printf("\x1b[13;0HPress Start to exit.");

	printf("\x1b[14;0H*BL = Battery Level *BC = Battery Charge");
	printf("\x1b[15;0H-------------------------------------------------");
	printf("\x1b[16;0H[%02d:%02d:%02d]", hours, minutes, seconds);
}

void Data_update_bottom(void)
{
	if (check_battery_level != battery_level)
	{
		consoleSelect(&Screen_bottom);
		if (battery_charge == 1)
		{
			printf("[%02d:%02d:%02d] BL=%d%% %.2fv BC=\x1b[1m\x1b[32mYes\x1b[0m\x1b[37m\n", hours, minutes, seconds, battery_level, battery_voltage_calculate);
		}
		else
		{
			printf("[%02d:%02d:%02d] BL=%d%% %.2fv BC=\x1b[1m\x1b[31mNo\x1b[0m\x1b[37m\n", hours, minutes, seconds, battery_level, battery_voltage_calculate);
		}
	}

	check_battery_level = battery_level;
}

void Debug(void)
{
	line = 18;
	consoleSelect(&Screen_top);
	if (timer_flag)
	{
		printf("\x1b[17;0H\x1b[45m\rtimer flag \x1b[32mtrue \x1b[37m\x1b[40m");
		for (int i = 0; i < Thread_max; i++)
		{
			printf("\x1b[%d;0H\x1b[45m\rthread %d : %d Loop/s\x1b[40m        ", line + i, i, loops[i]);
			all_loops = all_loops + loops[i];
		}
		printf("\x1b[22;0H\x1b[45m\rall thread : %d Loop/s\x1b[40m        ", all_loops);
	}
	else
	{
		printf("\x1b[17;0H\x1b[45m\rtimer flag \x1b[31mfalse\x1b[37m\x1b[40m");
	}
}

void Update_thread(void *arg)
{
	while (update_thread_run)
	{
		Get_system_info();
		usleep(500000);
	}
}

void Debug_mode_change_dialog(void)
{
	input_wait = true;
	consoleSelect(&Screen_top);

	//message
	printf("\x1b[10;12H--------------------------");
	printf("\x1b[11;12H|     Do you want to     |");
	printf("\x1b[12;12H|   enable debug mode?   |");
	printf("\x1b[13;12H|                        |");
	printf("\x1b[14;12H|                        |");
	printf("\x1b[15;12H|                        |");
	printf("\x1b[16;12H|                        |");
	printf("\x1b[17;12H|       \x1b[32m<A>enable\x1b[37m        |");
	printf("\x1b[18;12H|       \x1b[31m<B>disable\x1b[37m       |");
	printf("\x1b[19;12H|  Waiting for input...  |");
	printf("\x1b[20;12H--------------------------");

	while (input_wait)
	{
		hidScanInput();
		u32 key = hidKeysDown();
		if (key & KEY_A)
		{
			if (!debug)
			{
				consoleSelect(&Screen_bottom);
				printf("[%02d:%02d:%02d] Debug enabled. \n", hours, minutes, seconds);
			}
			debug = true;
			input_wait = false;
		}
		else if (key & KEY_B)
		{
			if (debug)
			{
				consoleSelect(&Screen_bottom);
				printf("[%02d:%02d:%02d] Debug disabled. \n", hours, minutes, seconds);
			}
			debug = false;
			input_wait = false;
		}
		usleep(main_sleep);
	}

	consoleSelect(&Screen_top);
	consoleClear();
}

void Debug_mode_setting_dialog(void)
{
	char* select[] = { ">"," "," "," "," " };
	char* flag[] = { "32mEnable ","32mEnable ","32mEnable ","32mEnable " };
	int select_num = 0;
	input_wait = true;
	consoleSelect(&Screen_top);

	while (input_wait)
	{

		//message
		printf("\x1b[10;12H--------------------------");
		printf("\x1b[11;12H|    Advanced setting    |");
		printf("\x1b[12;12H|                        |");
		printf("\x1b[13;12H| %s Continue             |", select[0]);
		printf("\x1b[14;12H| %s PTMU svc Init \x1b[%s\x1b[37m|", select[1], flag[0]);
		printf("\x1b[15;12H| %s MCU svc Init \x1b[%s\x1b[37m |", select[2], flag[1]);
		printf("\x1b[16;12H| %s APT svc Init \x1b[%s\x1b[37m |", select[3], flag[2]);
		printf("\x1b[17;12H| %s NEWS svc Init \x1b[%s\x1b[37m|", select[4], flag[3]);
		printf("\x1b[18;12H|    \x1b[32m<A>\x1b[37mChange/Confirm   |");
		printf("\x1b[19;12H|    \x1b[32m<Up>\x1b[37m,\x1b[32m<Down>\x1b[37m move    |");
		printf("\x1b[20;12H--------------------------");

		usleep(main_sleep);
		hidScanInput();
		u32 key = hidKeysDown();
		if (key & KEY_A)
		{
			if (select_num == 0)
			{
				input_wait = false;
			}
			else if (select_num == 1)
			{
				if (ptmu_init_flag)
				{
					flag[0] = "31mDisable";
					ptmu_init_flag = false;
				}
				else
				{
					flag[0] = "32mEnable ";
					ptmu_init_flag = true;
				}
			}
			else if (select_num == 2)
			{
				if (mcu_init_flag)
				{
					flag[1] = "31mDisable";
					mcu_init_flag = false;
				}
				else
				{
					flag[1] = "32mEnable ";
					mcu_init_flag = true;
				}
			}
			else if (select_num == 3)
			{
				if (apt_init_flag)
				{
					flag[2] = "31mDisable";
					apt_init_flag = false;
				}
				else
				{
					flag[2] = "32mEnable ";
					apt_init_flag = true;
				}
			}
			else if (select_num == 4)
			{
				if (news_init_flag)
				{
					flag[3] = "31mDisable";
					news_init_flag = false;
				}
				else
				{
					flag[3] = "32mEnable ";
					news_init_flag = true;
				}
			}
		}
		else if (key & KEY_DOWN)
		{
			select_num = select_num + 1;
			if (select_num > 4)
			{
				select_num = 4;
			}
		}
		else if (key & KEY_UP)
		{
			select_num = select_num - 1;
			if (select_num < 0)
			{
				select_num = 0;
			}
		}

		if (select_num == 0)
		{
			select[0] = ">";
			select[1] = " ";
			select[2] = " ";
			select[3] = " ";
			select[4] = " ";
		}
		else if(select_num == 1)
		{
			select[0] = " ";
			select[1] = ">";
			select[2] = " ";
			select[3] = " ";
			select[4] = " ";
		}
		else if (select_num == 2)
		{
			select[0] = " ";
			select[1] = " ";
			select[2] = ">";
			select[3] = " ";
			select[4] = " ";
		}
		else if (select_num == 3)
		{
			select[0] = " ";
			select[1] = " ";
			select[2] = " ";
			select[3] = ">";
			select[4] = " ";
		}
		else if (select_num == 4)
		{
			select[0] = " ";
			select[1] = " ";
			select[2] = " ";
			select[3] = " ";
			select[4] = ">";
		}
	}

	consoleSelect(&Screen_top);
	consoleClear();
}

void Exit_check_dialog(void)
{
	input_wait = true;
	consoleSelect(&Screen_top);

	//message
	printf("\x1b[10;12H--------------------------");
	printf("\x1b[11;12H|     Do you want to     |");
	printf("\x1b[12;12H|        exit app?       |");
	printf("\x1b[13;12H|                        |");
	printf("\x1b[14;12H|                        |");
	printf("\x1b[15;12H|                        |");
	printf("\x1b[16;12H|                        |");
	printf("\x1b[17;12H|         \x1b[32m<A>Yes\x1b[37m         |");
	printf("\x1b[18;12H|         \x1b[31m<B>No\x1b[37m          |");
	printf("\x1b[19;12H|  Waiting for input...  |");
	printf("\x1b[20;12H--------------------------");

	while (input_wait)
	{
		hidScanInput();
		u32 key = hidKeysDown();
		if (key & KEY_A)
		{
			app_exit = true;
			input_wait = false;
		}
		else if (key & KEY_B)
		{
			input_wait = false;
		}
		usleep(main_sleep);
	}

	consoleSelect(&Screen_top);
	consoleClear();
}

void Change_cpu_limit(void)
{
	input_wait = true;
	u32 new_cpu_limit = 80;
	consoleSelect(&Screen_top);

	while (input_wait)
	{

		//message
		printf("\x1b[10;12H--------------------------");
		printf("\x1b[11;12H|    Set new cpu limit   |");
		printf("\x1b[12;12H|     (System Core)      |");
		printf("\x1b[13;12H|                        |");
		printf("\x1b[14;12H|     CPU limit %02ld %%     |", new_cpu_limit);
		printf("\x1b[15;12H|                        |");
		printf("\x1b[16;12H|                        |");
		printf("\x1b[17;12H|      \x1b[32m<Up>,<Down>\x1b[37m       |");
		printf("\x1b[18;12H| \x1b[32m<LEFT>,<RIGHT>\x1b[37m change  |");
		printf("\x1b[19;12H|       \x1b[32m<A>\x1b[37mConfirm       |");
		printf("\x1b[20;12H--------------------------");

		usleep(main_sleep);
		hidScanInput();
		u32 key = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		if (key & KEY_A)
		{
			input_wait = false;
			Result set_cpu_limit_result = APT_SetAppCpuTimeLimit(new_cpu_limit);

			if (debug)
			{
				consoleSelect(&Screen_bottom);
				if (set_cpu_limit_result == 0)
				{
					printf("[%02d:%02d:%02d] \x1b[45mSet cpu limit \x1b[1m\x1b[32mSucces!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[32m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, set_cpu_limit_result);
				}
				else
				{
					printf("[%02d:%02d:%02d] \x1b[45mSet cpu limit \x1b[1m\x1b[33mFailed!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[33m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, set_cpu_limit_result);
				}
				APT_GetAppCpuTimeLimit(&cpu_limit);
				printf("[%02d:%02d:%02d] \x1b[45mcpu limit value = \x1b[32m\x1b[1m%ld%%\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, cpu_limit);
			}
		}
		else if (kHeld & KEY_UP)
		{
			usleep(50000);
			new_cpu_limit = new_cpu_limit + 1;
			if (new_cpu_limit > 80)
			{
				new_cpu_limit = 80;
			}
		}
		else if (kHeld & KEY_DOWN)
		{
			usleep(50000);
			new_cpu_limit = new_cpu_limit - 1;
			if (new_cpu_limit < 5)
			{
				new_cpu_limit = 5;
			}
		}
		else if (key & KEY_LEFT)
		{
			new_cpu_limit = 5;
		}
		else if (key & KEY_RIGHT)
		{
			new_cpu_limit = 80;
		}
	}
	consoleSelect(&Screen_top);
	consoleClear();
}

void Check_time(void)
{
	int hour_loop = 0;
	int minute_loop = 0;
	int second_loop = 0;

	//hour
	for (int ih = 0; ih < 3; ih++)
	{
		for (int ih_2 = 0; ih_2 < 10; ih_2++)
		{
			if (hours == hour_loop)
			{
				hour_0 = ih;
				hour_1 = ih_2;
				ih = 100; ih_2 = 100;
				break;
			}
			hour_loop = hour_loop + 1;
		}
	}

	//minute
	for (int im = 0; im < 6; im++)
	{
		for (int im_2 = 0; im_2 < 10; im_2++)
		{
			if (minutes == minute_loop)
			{
				minute_0 = im;
				minute_1 = im_2;
				im = 100; im_2 = 100;
				break;
			}
			minute_loop = minute_loop + 1;
		}
	}

	//second
	for (int is = 0; is < 6; is++)
	{
		for (int is_2 = 0; is_2 < 10; is_2++)
		{
			if (seconds == second_loop)
			{
				second_0 = is;
				second_1 = is_2;
				is = 100; is_2 = 100;
				break;
			}
			second_loop = second_loop + 1;
		}
	}
}

void Check_battery(void)
{
	int loop_battery_level = 0;
	int loop_battery_voltage = 0;
	double battery_voltage_calculate_news = battery_voltage_calculate * 100;
	int battery_voltage = battery_voltage_calculate_news;

	if (battery_level == 100)
	{
		battery_0 = 9;
		battery_1 = 9;
	}
	else
	{
		for (int i_bl = 0; i_bl < 10; i_bl++)
		{
			for (int i_bl_2 = 0; i_bl_2 < 10; i_bl_2++)
			{
				if (battery_level == loop_battery_level)
				{
					battery_0 = i_bl;
					battery_1 = i_bl_2;
					i_bl = 100; i_bl_2 = 100;
					break;
				}
				loop_battery_level = loop_battery_level + 1;
			}
		}
	}

	for (int i_vl = 0; i_vl < 5; i_vl++)
	{
		for (int i_vl_2 = 0; i_vl_2 < 10; i_vl_2++)
		{
			for (int i_vl_3 = 0; i_vl_3 < 10; i_vl_3++)
			{
				if (battery_voltage == loop_battery_voltage)
				{
					battery_v_0 = i_vl;
					battery_v_1 = i_vl_2;
					battery_v_2 = i_vl_3;
					i_vl = 100; i_vl_2 = 100; i_vl_3 = 100;
					break;
				}
				loop_battery_voltage = loop_battery_voltage + 1;
			}
		}
	}

	if (battery_charge == 1)
	{
		battery_c_0 = 3;
		battery_c_1 = 4;
		battery_c_2 = 5;
	}
	else
	{
		battery_c_0 = 1;
		battery_c_1 = 2;
		battery_c_2 = 0;
	}
}

void Battery_mark_result(void)
{
	int start_hour_0 = 9;
	int start_hour_1 = 9;
	int start_minute_0 = 9;
	int start_minute_1 = 9;
	int start_second_0 = 9;
	int start_second_1 = 9;
	int start_battery_0 = 9;
	int start_battery_1 = 9;
	int start_battery_v_0 = 9;
	int start_battery_v_1 = 9;
	int start_battery_v_2 = 9;
	double start_battery_voltage_calculate_news = battery_mark_battery_voltage_start * 100;
	int start_battery_voltage = start_battery_voltage_calculate_news;

	int end_hour_0 = 9;
	int end_hour_1 = 9;
	int end_minute_0 = 9;
	int end_minute_1 = 9;
	int end_second_0 = 9;
	int end_second_1 = 9;
	int end_battery_0 = 9;
	int end_battery_1 = 9;
	int end_battery_v_0 = 9;
	int end_battery_v_1 = 9;
	int end_battery_v_2 = 9;
	double end_battery_voltage_calculate_news = battery_mark_battery_voltage_end * 100;
	int end_battery_voltage = end_battery_voltage_calculate_news;

	int decrease_min_minute_0 = 9;
	int decrease_min_minute_1 = 9;
	int decrease_min_second_0 = 9;
	int decrease_min_second_1 = 9;

	int decrease_max_minute_0 = 9;
	int decrease_max_minute_1 = 9;
	int decrease_max_second_0 = 9;
	int decrease_max_second_1 = 9;

	int decrease_avg_minute_0 = 9;
	int decrease_avg_minute_1 = 9;
	int decrease_avg_second_0 = 9;
	int decrease_avg_second_1 = 9;

	int total_time_hour_0 = 9;
	int total_time_hour_1 = 9;
	int total_time_minute_0 = 9;
	int total_time_minute_1 = 9;
	int total_time_second_0 = 9;
	int total_time_second_1 = 9;

	int hour_loop = 0;
	int minute_loop = 0;
	int second_loop = 0;
	int loop_battery_level = 0;
	int loop_battery_voltage = 0;

	int mode_msg[] = { 26,26,26,26,26,26,26,26,26,26 };

	if (news_init_succes)
	{
		//start hour
		for (int ih = 0; ih < 3; ih++)
		{
			for (int ih_2 = 0; ih_2 < 10; ih_2++)
			{
				if (battery_mark_start_time_hours == hour_loop)
				{
					start_hour_0 = ih;
					start_hour_1 = ih_2;
					ih = 100; ih_2 = 100;
					break;
				}
				hour_loop = hour_loop + 1;
			}
		}

		//start minute
		for (int im = 0; im < 6; im++)
		{
			for (int im_2 = 0; im_2 < 10; im_2++)
			{
				if (battery_mark_start_time_minutes == minute_loop)
				{
					start_minute_0 = im;
					start_minute_1 = im_2;
					im = 100; im_2 = 100;
					break;
				}
				minute_loop = minute_loop + 1;
			}
		}

		//start second
		for (int is = 0; is < 6; is++)
		{
			for (int is_2 = 0; is_2 < 10; is_2++)
			{
				if (battery_mark_start_time_seconds == second_loop)
				{
					start_second_0 = is;
					start_second_1 = is_2;
					is = 100; is_2 = 100;
					break;
				}
				second_loop = second_loop + 1;
			}
		}

		//start battery level
		if (battery_mark_battery_level_start == 100)
		{
			start_battery_0 = 9;
			start_battery_1 = 9;
		}
		else
		{
			for (int i_bl = 0; i_bl < 10; i_bl++)
			{
				for (int i_bl_2 = 0; i_bl_2 < 10; i_bl_2++)
				{
					if (battery_mark_battery_level_start == loop_battery_level)
					{
						start_battery_0 = i_bl;
						start_battery_1 = i_bl_2;
						i_bl = 100; i_bl_2 = 100;
						break;
					}
					loop_battery_level = loop_battery_level + 1;
				}
			}
		}

		//start battery voltage
		for (int i_vl = 0; i_vl < 5; i_vl++)
		{
			for (int i_vl_2 = 0; i_vl_2 < 10; i_vl_2++)
			{
				for (int i_vl_3 = 0; i_vl_3 < 10; i_vl_3++)
				{
					if (start_battery_voltage == loop_battery_voltage)
					{
						start_battery_v_0 = i_vl;
						start_battery_v_1 = i_vl_2;
						start_battery_v_2 = i_vl_3;
						i_vl = 100; i_vl_2 = 100; i_vl_3 = 100;
						break;
					}
					loop_battery_voltage = loop_battery_voltage + 1;
				}
			}
		}

		hour_loop = 0;
		minute_loop = 0;
		second_loop = 0;
		loop_battery_level = 0;
		loop_battery_voltage = 0;

		//end hour
		for (int ih = 0; ih < 3; ih++)
		{
			for (int ih_2 = 0; ih_2 < 10; ih_2++)
			{
				if (battery_mark_end_time_hours == hour_loop)
				{
					end_hour_0 = ih;
					end_hour_1 = ih_2;
					ih = 100; ih_2 = 100;
					break;
				}
				hour_loop = hour_loop + 1;
			}
		}

		//end minute
		for (int im = 0; im < 6; im++)
		{
			for (int im_2 = 0; im_2 < 10; im_2++)
			{
				if (battery_mark_end_time_minutes == minute_loop)
				{
					end_minute_0 = im;
					end_minute_1 = im_2;
					im = 100; im_2 = 100;
					break;
				}
				minute_loop = minute_loop + 1;
			}
		}

		//end second
		for (int is = 0; is < 6; is++)
		{
			for (int is_2 = 0; is_2 < 10; is_2++)
			{
				if (battery_mark_end_time_seconds == second_loop)
				{
					end_second_0 = is;
					end_second_1 = is_2;
					is = 100; is_2 = 100;
					break;
				}
				second_loop = second_loop + 1;
			}
		}

		//end battery level
		if (battery_mark_battery_level_end == 100)
		{
			end_battery_0 = 9;
			end_battery_1 = 9;
		}
		else
		{
			for (int i_bl = 0; i_bl < 10; i_bl++)
			{
				for (int i_bl_2 = 0; i_bl_2 < 10; i_bl_2++)
				{
					if (battery_mark_battery_level_end == loop_battery_level)
					{
						end_battery_0 = i_bl;
						end_battery_1 = i_bl_2;
						i_bl = 100; i_bl_2 = 100;
						break;
					}
					loop_battery_level = loop_battery_level + 1;
				}
			}
		}

		//end battery voltage
		for (int i_vl = 0; i_vl < 5; i_vl++)
		{
			for (int i_vl_2 = 0; i_vl_2 < 10; i_vl_2++)
			{
				for (int i_vl_3 = 0; i_vl_3 < 10; i_vl_3++)
				{
					if (end_battery_voltage == loop_battery_voltage)
					{
						end_battery_v_0 = i_vl;
						end_battery_v_1 = i_vl_2;
						end_battery_v_2 = i_vl_3;
						i_vl = 100; i_vl_2 = 100; i_vl_3 = 100;
						break;
					}
					loop_battery_voltage = loop_battery_voltage + 1;
				}
			}
		}

		minute_loop = 0;
		second_loop = 0;

		//battery decrease min minute
		for (int im = 0; im < 6; im++)
		{
			for (int im_2 = 0; im_2 < 10; im_2++)
			{
				if (battery_mark_min_time_minutes == minute_loop)
				{
					decrease_min_minute_0 = im;
					decrease_min_minute_1 = im_2;
					im = 100; im_2 = 100;
					break;
				}
				minute_loop = minute_loop + 1;
			}
		}

		//battery decrease min second
		for (int is = 0; is < 6; is++)
		{
			for (int is_2 = 0; is_2 < 10; is_2++)
			{
				if (battery_mark_min_time_seconds == second_loop)
				{
					decrease_min_second_0 = is;
					decrease_min_second_1 = is_2;
					is = 100; is_2 = 100;
					break;
				}
				second_loop = second_loop + 1;
			}
		}

		minute_loop = 0;
		second_loop = 0;

		//battery decrease max minute
		for (int im = 0; im < 6; im++)
		{
			for (int im_2 = 0; im_2 < 10; im_2++)
			{
				if (battery_mark_max_time_minutes == minute_loop)
				{
					decrease_max_minute_0 = im;
					decrease_max_minute_1 = im_2;
					im = 100; im_2 = 100;
					break;
				}
				minute_loop = minute_loop + 1;
			}
		}

		//battery decrease max second
		for (int is = 0; is < 6; is++)
		{
			for (int is_2 = 0; is_2 < 10; is_2++)
			{
				if (battery_mark_max_time_seconds == second_loop)
				{
					decrease_max_second_0 = is;
					decrease_max_second_1 = is_2;
					is = 100; is_2 = 100;
					break;
				}
				second_loop = second_loop + 1;
			}
		}

		minute_loop = 0;
		second_loop = 0;

		//battery decrease avg minute
		for (int im = 0; im < 6; im++)
		{
			for (int im_2 = 0; im_2 < 10; im_2++)
			{
				if (battery_mark_avg_time_minutes == minute_loop)
				{
					decrease_avg_minute_0 = im;
					decrease_avg_minute_1 = im_2;
					im = 100; im_2 = 100;
					break;
				}
				minute_loop = minute_loop + 1;
			}
		}

		//battery decrease avg second
		for (int is = 0; is < 6; is++)
		{
			for (int is_2 = 0; is_2 < 10; is_2++)
			{
				if (battery_mark_avg_time_seconds == second_loop)
				{
					decrease_avg_second_0 = is;
					decrease_avg_second_1 = is_2;
					is = 100; is_2 = 100;
					break;
				}
				second_loop = second_loop + 1;
			}
		}


		hour_loop = 0;
		minute_loop = 0;
		second_loop = 0;

		//total hour
		for (int ih = 0; ih < 3; ih++)
		{
			for (int ih_2 = 0; ih_2 < 10; ih_2++)
			{
				if (battery_mark_total_time_hours == hour_loop)
				{
					total_time_hour_0 = ih;
					total_time_hour_1 = ih_2;
					ih = 100; ih_2 = 100;
					break;
				}
				hour_loop = hour_loop + 1;
			}
		}

		//end minute
		for (int im = 0; im < 6; im++)
		{
			for (int im_2 = 0; im_2 < 10; im_2++)
			{
				if (battery_mark_total_time_minutes == minute_loop)
				{
					total_time_minute_0 = im;
					total_time_minute_1 = im_2;
					im = 100; im_2 = 100;
					break;
				}
				minute_loop = minute_loop + 1;
			}
		}

		//end second
		for (int is = 0; is < 6; is++)
		{
			for (int is_2 = 0; is_2 < 10; is_2++)
			{
				if (battery_mark_total_time_seconds == second_loop)
				{
					total_time_second_0 = is;
					total_time_second_1 = is_2;
					is = 100; is_2 = 100;
					break;
				}
				second_loop = second_loop + 1;
			}
		}


		//mode
		if (battery_mark_full)
		{
			mode_msg[0] = 5;
			mode_msg[1] = 20;
			mode_msg[2] = 11;
			mode_msg[3] = 11;
		}
		else if (battery_mark_short)
		{
			mode_msg[0] = 18;
			mode_msg[1] = 7;
			mode_msg[2] = 14;
			mode_msg[3] = 17;
			mode_msg[4] = 19;
		}
		else if (battery_mark_medium)
		{
			mode_msg[0] = 12;
			mode_msg[1] = 4;
			mode_msg[2] = 3;
			mode_msg[3] = 8;
			mode_msg[4] = 20;
			mode_msg[5] = 12;
		}
		else if (battery_mark_very_short)
		{
			mode_msg[0] = 21;
			mode_msg[1] = 4;
			mode_msg[2] = 17;
			mode_msg[3] = 24;
			mode_msg[4] = 26;
			mode_msg[5] = 18;
			mode_msg[6] = 7;
			mode_msg[7] = 14;
			mode_msg[8] = 17;
			mode_msg[9] = 19;
		}
		else if (battery_mark_custom)
		{
			mode_msg[0] = 2;
			mode_msg[1] = 20;
			mode_msg[2] = 18;
			mode_msg[3] = 19;
			mode_msg[4] = 14;
			mode_msg[5] = 12;
		}

		u16 title[] = { B,a,t,t,e,r,y,msg_space,m,a,r,k,msg_space,r,e,s,u,l,t };
		int title_length = sizeof title / sizeof title[0];
		u32 titlelength = title_length - 1;

		u16 message_num[] = { _0,_1,_2,_3,_4,_5,_6,_7,_8,_9,msg_space };
		u16 message_abc[] = { a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,msg_space };

		u16 message[] = { B,a,t,t,e,r,y,msg_space,m,a,r,k,msg_space,v,_1,msg_4,_0,msg_4,_1,msg_space,r,e,s,u,l,t,msg_,msg_
			,B,a,t,t,e,r,y,msg_space,m,a,r,k,msg_space,m,o,d,e,msg_space,msg_3,msg_space,message_abc[mode_msg[0]],message_abc[mode_msg[1]],message_abc[mode_msg[2]],message_abc[mode_msg[3]],message_abc[mode_msg[4]]
			,message_abc[mode_msg[5]],message_abc[mode_msg[6]],message_abc[mode_msg[7]],message_abc[mode_msg[8]],message_abc[mode_msg[9]],msg_
			,B,a,t,t,e,r,y,msg_space,m,a,r,k,msg_space,s,t,a,r,t,msg_
			,msg_0,message_num[start_hour_0],message_num[start_hour_1],msg_2,message_num[start_minute_0],message_num[start_minute_1],msg_2,message_num[start_second_0],message_num[start_second_1],msg_1
			,msg_space,message_num[start_battery_0],message_num[start_battery_1],msg_percent,msg_space,message_num[start_battery_v_0],msg_4,message_num[start_battery_v_1],message_num[start_battery_v_2],v,msg_
			,B,a,t,t,e,r,y,msg_space,m,a,r,k,msg_space,e,n,d,msg_
			,msg_0,message_num[end_hour_0],message_num[end_hour_1],msg_2,message_num[end_minute_0],message_num[end_minute_1],msg_2,message_num[end_second_0],message_num[end_second_1],msg_1
			,msg_space,message_num[end_battery_0],message_num[end_battery_1],msg_percent,msg_space,message_num[end_battery_v_0],msg_4,message_num[end_battery_v_1],message_num[end_battery_v_2],v,msg_,msg_
			,T,e,s,t,e,d,msg_space,b,a,t,t,e,r,y,msg_space,l,e,v,e,l,msg_space,msg_3,msg_space,message_num[start_battery_0],message_num[start_battery_1],msg_percent,msg_space,msg_5,msg_6,msg_space,message_num[end_battery_0],message_num[end_battery_1],msg_percent,msg_
			,B,a,t,t,e,r,y,msg_space,d,e,c,r,e,a,s,e,msg_space,_1,msg_percent,msg_space,m,i,n,msg_space,msg_0,message_num[decrease_min_minute_0],message_num[decrease_min_minute_1],msg_2,message_num[decrease_min_second_0],message_num[decrease_min_second_1],msg_1,msg_
			,B,a,t,t,e,r,y,msg_space,d,e,c,r,e,a,s,e,msg_space,_1,msg_percent,msg_space,m,a,x,msg_space,msg_0,message_num[decrease_max_minute_0],message_num[decrease_max_minute_1],msg_2,message_num[decrease_max_second_0],message_num[decrease_max_second_1],msg_1,msg_
			,B,a,t,t,e,r,y,msg_space,d,e,c,r,e,a,s,e,msg_space,_1,msg_percent,msg_space,a,v,g,msg_space,msg_0,message_num[decrease_avg_minute_0],message_num[decrease_avg_minute_1],msg_2,message_num[decrease_avg_second_0],message_num[decrease_avg_second_1],msg_1,msg_
			,T,o,t,a,l,msg_space,e,l,a,p,s,e,d,msg_space,t,i,m,e,msg_space,msg_0,message_num[total_time_hour_0],message_num[total_time_hour_1],msg_2,message_num[total_time_minute_0],message_num[total_time_minute_1],msg_2,message_num[total_time_second_0],message_num[total_time_second_1],msg_1
			, 0x0 };
		u32 messageLength = 1000;

		NEWS_AddNotification(title, titlelength, message, messageLength, NULL, 0, false);
	}
}

void Battery_mark_init(void)
{
	void Mode_flag_false(void)
	{
		battery_mark_full = false;
		battery_mark_short = false;
		battery_mark_medium = false;
		battery_mark_very_short = false;
		battery_mark_custom = false;
	}

	bool battery_mark_init_succes = true;

	battery_mark_start_time_hours = -1;
	battery_mark_start_time_minutes = -1;
	battery_mark_start_time_seconds = -1;
	battery_mark_battery_level_start = -1;

	consoleSelect(&Screen_top);
	printf("\x1b[47m\x1b[30m");

	printf("\x1b[4;4H-------------------------------------------");
	printf("\x1b[5;4H|             \x1b[36mBattery mark\x1b[30m                |");
	printf("\x1b[6;4H|         Checking service Init           |");
	printf("\x1b[7;4H|  (\x1b[31mRequired\x1b[30m) ptmu service Init ...       |");
	printf("\x1b[8;4H|  (\x1b[31mRequired\x1b[30m) mcu service Init  ...       |");
	printf("\x1b[9;4H|  (\x1b[31mRequired\x1b[30m) apt service Init  ...       |");
	printf("\x1b[10;4H|  (\x1b[34mOptional\x1b[30m) news service Init ...       |");
	printf("\x1b[11;4H|  Set screen light level 5 ...           |");
	printf("\x1b[12;4H|  Set system core limit 30%% ...          |");
	printf("\x1b[13;4H|  Set system core limit 70%% ...          |");
	printf("\x1b[14;4H|  Start cpu load Thread 0 ...            |");
	printf("\x1b[15;4H|  Start cpu load Thread 1 ...            |");
	printf("\x1b[16;4H|                                         |");
	printf("\x1b[17;4H|  State : Initilizing [         ]        |");
	printf("\x1b[18;4H|       Choose battery mark mode          |");
	printf("\x1b[19;4H|                                         |");
	printf("\x1b[20;4H|                                         |");
	printf("\x1b[21;4H|                                         |");
	printf("\x1b[22;4H|                                         |");
	printf("\x1b[23;4H|                                         |");
	printf("\x1b[24;4H|\x1b[32m<Left><Right>\x1b[30mSelect,\x1b[32m<A>\x1b[30mContinu,\x1b[32m<B>\x1b[30mGo back|");
	printf("\x1b[25;4H-------------------------------------------");

	usleep(1000000);
	if (ptmu_init_succes)
	{
		printf("\x1b[7;4H|  (\x1b[31mRequired\x1b[30m) ptmu service Init ... \x1b[32mGood\x1b[30m  |");
		printf("\x1b[17;28H\x1b[32mo\x1b[30m");
	}
	else
	{
		printf("\x1b[7;4H|  (\x1b[31mRequired\x1b[30m) ptmu service Init ... \x1b[31mBad\x1b[30m   |");
		printf("\x1b[17;28H\x1b[31mx\x1b[30m");
		battery_mark_init_succes = false;
	}

	usleep(main_sleep);
	if (mcu_init_succes)
	{
		printf("\x1b[8;4H|  (\x1b[31mRequired\x1b[30m) mcu service Init  ... \x1b[32mGood\x1b[30m  |");
		printf("\x1b[17;29H\x1b[32mo\x1b[30m");
	}
	else
	{
		printf("\x1b[8;4H|  (\x1b[31mRequired\x1b[30m) mcu service Init  ... \x1b[31mBad\x1b[30m   |");
		printf("\x1b[17;29H\x1b[31mx\x1b[30m");
		battery_mark_init_succes = false;
	}

	usleep(main_sleep);
	if (apt_init_succes)
	{
		printf("\x1b[9;4H|  (\x1b[31mRequired\x1b[30m) apt service Init  ... \x1b[32mGood\x1b[30m  |");
		printf("\x1b[17;30H\x1b[32mo\x1b[30m");
	}
	else
	{
		printf("\x1b[9;4H|  (\x1b[31mRequired\x1b[30m) apt service Init  ... \x1b[31mBad\x1b[30m   |");
		printf("\x1b[17;30H\x1b[31mx\x1b[30m");
		battery_mark_init_succes = false;
	}

	usleep(main_sleep);
	if (news_init_succes)
	{
		printf("\x1b[10;4H|  (\x1b[34mOptional\x1b[30m) news service Init ... \x1b[32mGood\x1b[30m  |");
		printf("\x1b[17;31H\x1b[32mo\x1b[30m");
	}
	else
	{
		printf("\x1b[10;4H|  (\x1b[34mOptional\x1b[30m) news service Init ... \x1b[31mBad\x1b[30m   |");
		printf("\x1b[17;31H\x1b[31mx\x1b[30m");
	}

	usleep(main_sleep);
	gspLcdInit();
	Result set_light_level = GSPLCD_SetBrightness(GSPLCD_SCREEN_BOTH, 5);
	if (set_light_level == 0)
	{
		printf("\x1b[11;4H|  Set screen light level 5 ... \x1b[32mGood\x1b[30m      |");
		printf("\x1b[17;32H\x1b[32mo\x1b[30m");
	}
	else
	{
		printf("\x1b[11;4H|  Set screen light level 5 ... \x1b[31mBad\x1b[30m       |");
		printf("\x1b[17;32H\x1b[31mx\x1b[30m");
	}
	gspLcdExit();

	usleep(main_sleep);
	if (apt_init_succes)
	{
		Result set_cpu_limit_30_result = APT_SetAppCpuTimeLimit(30);
		if (set_cpu_limit_30_result == 0)
		{
			printf("\x1b[12;4H|  Set system core limit 30%% ... \x1b[32mGood\x1b[30m     |");
			printf("\x1b[17;33H\x1b[32mo\x1b[30m");
		}
		else
		{
			printf("\x1b[12;4H|  Set system core limit 30%% ... \x1b[31mBad\x1b[30m      |");
			printf("\x1b[17;33H\x1b[31mx\x1b[30m");
		}
	}
	else
	{
		printf("\x1b[12;4H|  Set system core limit 30%% ... \x1b[31mSkipped\x1b[30m  |");
		printf("\x1b[17;33H\x1b[31mx\x1b[30m");
	}

	usleep(main_sleep);
	if (apt_init_succes)
	{
		Result set_cpu_limit_70_result = APT_SetAppCpuTimeLimit(70);
		if (set_cpu_limit_70_result == 0)
		{
			printf("\x1b[13;4H|  Set system core limit 70%% ... \x1b[32mGood\x1b[30m     |");
			printf("\x1b[17;34H\x1b[32mo\x1b[30m");
		}
		else
		{
			printf("\x1b[13;4H|  Set system core limit 70%% ... \x1b[31mBad\x1b[30m      |");
			printf("\x1b[17;34H\x1b[31mx\x1b[30m");
		}
	}
	else
	{
		printf("\x1b[13;4H|  Set system core limit 70%% ... \x1b[31mSkipped\x1b[30m  |");
		printf("\x1b[17;34H\x1b[31mx\x1b[30m");
	}

	thread_num = 2;
	cpu_load = true;
	CPUload_start();
	CPUload_start_wait();

	usleep(main_sleep);
	consoleSelect(&Screen_top);
	if (cpu_load_started[0])
	{
		printf("\x1b[14;4H|  Start cpu load Thread 0 ... \x1b[32mGood\x1b[30m       |");
		printf("\x1b[17;35H\x1b[32mo\x1b[30m");
	}
	else
	{
		printf("\x1b[14;4H|  Start cpu load Thread 0 ... \x1b[31mBad\x1b[30m        |");
		printf("\x1b[17;35H\x1b[31mx\x1b[30m");
	}

	usleep(main_sleep);
	if (cpu_load_started[1])
	{
		printf("\x1b[15;4H|  Start cpu load Thread 1 ... \x1b[32mGood\x1b[30m       |");
		printf("\x1b[17;36H\x1b[32mo\x1b[30m");
	}
	else
	{
		printf("\x1b[15;4H|  Start cpu load Thread 1 ... \x1b[31mBad\x1b[30m        |");
		printf("\x1b[17;36H\x1b[31mx\x1b[30m");
	}

	usleep(main_sleep);
	printf("\x1b[17;39H\x1b[32mDone\x1b[30m");

	while (true)
	{
		hidScanInput();
		u32 key = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		Data_update_bottom();

		consoleSelect(&Screen_top);
		printf("\x1b[19;4H|            %s             |", battery_mark_mode[battery_mark_select_num]);

		if (battery_mark_init_succes)
		{
			if (battery_mark_select_num == 0)//full
			{
				printf("\x1b[20;4H|    This mode will using 85%% battery     |");
				printf("\x1b[21;4H|    Your current battery level = %02d%%     |", battery_level);
				if (battery_level > 86)
				{
					printf("\x1b[22;4H|          \x1b[32mYou can this test now\x1b[30m          |");
					printf("\x1b[23;4H|      \x1b[32mPress A to start battery mark\x1b[30m      |");
					Mode_flag_false();
					battery_mark_full = true;
				}
				else
				{
					printf("\x1b[22;4H|        \x1b[31mYou can't this test now\x1b[30m          |");
					printf("\x1b[23;4H|   \x1b[31mPlease charge battery and try again\x1b[30m   |");
					Mode_flag_false();
				}
			}
			else if (battery_mark_select_num == 1)//medium
			{
				printf("\x1b[20;4H|    This mode will using 30%% battery     |");
				printf("\x1b[21;4H|    Your current battery level = %02d%%     |", battery_level);
				if (battery_level > 31)
				{
					printf("\x1b[22;4H|          \x1b[32mYou can this test now\x1b[30m          |");
					printf("\x1b[23;4H|      \x1b[32mPress A to start battery mark\x1b[30m      |");
					Mode_flag_false();
					battery_mark_medium = true;
				}
				else
				{
					printf("\x1b[22;4H|        \x1b[31mYou can't this test now\x1b[30m          |");
					printf("\x1b[23;4H|   \x1b[31mPlease charge battery and try again\x1b[30m   |");
					Mode_flag_false();
				}
			}
			else if (battery_mark_select_num == 2)//short
			{
				printf("\x1b[20;4H|    This mode will using 10%% battery     |");
				printf("\x1b[21;4H|    Your current battery level = %02d%%     |", battery_level);
				if (battery_level > 11)
				{
					printf("\x1b[22;4H|          \x1b[32mYou can this test now\x1b[30m          |");
					printf("\x1b[23;4H|      \x1b[32mPress A to start battery mark\x1b[30m      |");
					Mode_flag_false();
					battery_mark_short = true;
				}
				else
				{
					printf("\x1b[22;4H|        \x1b[31mYou can't this test now\x1b[30m          |");
					printf("\x1b[23;4H|   \x1b[31mPlease charge battery and try again\x1b[30m   |");
					Mode_flag_false();
				}
			}
			else if (battery_mark_select_num == 3)//very short
			{
				printf("\x1b[20;4H|    This mode will using 05%% battery     |");
				printf("\x1b[21;4H|    Your current battery level = %02d%%     |", battery_level);
				if (battery_level > 6)
				{
					printf("\x1b[22;4H|          \x1b[32mYou can this test now\x1b[30m          |");
					printf("\x1b[23;4H|      \x1b[32mPress A to start battery mark\x1b[30m      |");
					Mode_flag_false();
					battery_mark_very_short = true;
				}
				else
				{
					printf("\x1b[22;4H|        \x1b[31mYou can't this test now\x1b[30m          |");
					printf("\x1b[23;4H|   \x1b[31mPlease charge battery and try again\x1b[30m   |");
					Mode_flag_false();
				}
			}
			else if (battery_mark_select_num == 4)//custom
			{
				printf("\x1b[20;4H| using %02d%% battery (\x1b[32m<UP>,<DOWN>\x1b[30m change%%) |", battery_mark_custom_percent);
				printf("\x1b[21;4H|    Your current battery level = %02d%%     |", battery_level);
				if (battery_level > battery_mark_custom_percent + 1)
				{
					printf("\x1b[22;4H|          \x1b[32mYou can this test now\x1b[30m          |");
					printf("\x1b[23;4H|      \x1b[32mPress A to start battery mark\x1b[30m      |");
					Mode_flag_false();
					battery_mark_custom = true;
				}
				else
				{
					printf("\x1b[22;4H|        \x1b[31mYou can't this test now\x1b[30m          |");
					printf("\x1b[23;4H|   \x1b[31mPlease charge battery and try again\x1b[30m   |");
					Mode_flag_false();
				}
			}
		}
		else
		{
			printf("\x1b[20;4H|     \x1b[31mYou must Init Required service\x1b[30m      |");
			printf("\x1b[22;4H|         \x1b[31mYou can't this test now\x1b[30m         |");
			Mode_flag_false();
		}

		usleep(main_sleep);

		if (key & KEY_A)
		{
			if (battery_mark_full || battery_mark_medium || battery_mark_short || battery_mark_very_short || battery_mark_custom)
			{
				battery_mark_run = true;
				battery_mark_battery_level_start = battery_level - 1;
				battery_mark_battery_level_end = battery_mark_battery_level_start - battery_mark_mode_percent[battery_mark_select_num];
				break;
			}
		}
		else if (key & KEY_B)
		{
			battery_mark_run = false;
			printf("\x1b[40m\x1b[37m");
			CPUload_stop();
			consoleSelect(&Screen_top);
			consoleClear();
			break;
		}
		else if (key & KEY_LEFT)
		{
			battery_mark_select_num = battery_mark_select_num - 1;
			if (battery_mark_select_num < 0)
			{
				battery_mark_select_num = 0;
			}
		}
		else if (key & KEY_RIGHT)
		{
			battery_mark_select_num = battery_mark_select_num + 1;
			if (battery_mark_select_num > 4)
			{
				battery_mark_select_num = 4;
			}
		}

		if (battery_mark_select_num == 4)
		{
			if (kHeld & KEY_UP)
			{
				usleep(100000);
				battery_mark_custom_percent = battery_mark_custom_percent + 1;
				if (battery_mark_custom_percent > 95)
				{
					battery_mark_custom_percent = 95;
				}
				battery_mark_mode_percent[4] = battery_mark_custom_percent;
			}
			if (kHeld & KEY_DOWN)
			{
				usleep(100000);
				battery_mark_custom_percent = battery_mark_custom_percent - 1;
				if (battery_mark_custom_percent < 1)
				{
					battery_mark_custom_percent = 1;
				}
				battery_mark_mode_percent[4] = battery_mark_custom_percent;
			}
		}
	}
}

void Battery_mark(void)
{
	while (true)
	{
		gspLcdInit();
		GSPLCD_SetBrightness(GSPLCD_SCREEN_BOTH, 5);
		gspLcdExit();

		usleep(main_sleep);
		Data_update_bottom();

		consoleSelect(&Screen_top);
		printf("\x1b[47m\x1b[30m");

		if (input_wait)
		{

			printf("\x1b[11;10H-------------------------------");
			printf("\x1b[12;10H|    Do you want to close     |");
			printf("\x1b[13;10H|        battery mark?        |");
			printf("\x1b[14;10H|                             |");
			printf("\x1b[15;10H|         \x1b[31m!!Warning!!\x1b[30m         |");
			printf("\x1b[16;10H|  \x1b[31mProgress will be lost if\x1b[30m   |");
			printf("\x1b[17;10H|  \x1b[31myou close during the test\x1b[30m  |");
			printf("\x1b[18;10H|            \x1b[32m<A>Yes\x1b[30m           |");
			printf("\x1b[19;10H|            \x1b[31m<B>No\x1b[30m            |");
			printf("\x1b[20;10H|    Waiting for input...     |");
			printf("\x1b[21;10H-------------------------------");
		}
		else
		{
			printf("\x1b[11;4H|                                         |");
			printf("\x1b[12;4H| Battery decrease 1%% \x1b[31mmin\x1b[30m :    [%02d:%02d]    |", battery_mark_min_time_minutes, battery_mark_min_time_seconds);
			printf("\x1b[13;4H| Battery decrease 1%% \x1b[32mmax\x1b[30m :    [%02d:%02d]    |", battery_mark_max_time_minutes, battery_mark_max_time_seconds);
			printf("\x1b[14;4H| Battery decrease 1%% avg :    [%02d:%02d]    |", battery_mark_avg_time_minutes, battery_mark_avg_time_seconds);
			printf("\x1b[15;4H| Elapsed time            :    [%02d:%02d]    |", battery_mark_time_count_minutes, battery_mark_time_count_seconds);
			printf("\x1b[16;4H| Total elapsed time      : [%02d:%02d:%02d]    |", battery_mark_total_time_hours, battery_mark_total_time_minutes, battery_mark_total_time_seconds);
			printf("\x1b[17;4H| Time left (estimated)   : [%02d:%02d:%02d]    |", battery_mark_left_time_hours, battery_mark_left_time_minutes, battery_mark_left_time_seconds);
			printf("\x1b[18;4H|                                         |");
			printf("\x1b[19;4H|               \x1b[31m!!Warning!!\x1b[30m               |");
			printf("\x1b[20;4H| \x1b[31mDon't charge battery while battery mark\x1b[30m |");
			printf("\x1b[21;4H| \x1b[31mBattery mark will be abort if you do it\x1b[30m |");
			printf("\x1b[22;4H|                                         |");
			printf("\x1b[23;4H|                                         |");
			printf("\x1b[24;4H|               \x1b[32m<B>\x1b[30mGo back                |");
			printf("\x1b[25;4H-------------------------------------------");
		}

		printf("\x1b[4;4H-------------------------------------------");
		printf("\x1b[5;4H|              \x1b[36mBattery mark\x1b[30m               |");
		printf("\x1b[6;4H| Current time,battery[%02d:%02d:%02d] %02d%% %.2fv|", hours, minutes, seconds, battery_level, battery_voltage_calculate);
		printf("\x1b[7;4H| Battery mark start  [%02d:%02d:%02d] %02d%% %.2fv|", battery_mark_start_time_hours, battery_mark_start_time_minutes, battery_mark_start_time_seconds, battery_mark_battery_level_start, battery_mark_battery_voltage_start);
		printf("\x1b[8;4H| Battery mark end    [%02d:%02d:%02d] %02d%% %.2fv|", battery_mark_end_time_hours, battery_mark_end_time_minutes, battery_mark_end_time_seconds, battery_mark_battery_level_end, battery_mark_battery_voltage_end);
		printf("\x1b[9;4H| Battery mark mode : %s    |", battery_mark_mode[battery_mark_select_num]);

		if (battery_charge == 1)
		{
			battery_mark_running = false;
			battery_mark_abort = true;
		}

		if (battery_mark_running)
		{

			if (battery_mark_battery_before != battery_level)
			{
				battery_mark_less_count = battery_mark_less_count + 1;
				battery_mark_battery_before = battery_level;

				//min time update
				if (battery_mark_time_count < battery_mark_min_time)
				{
					battery_mark_min_time_seconds = battery_mark_time_count_seconds;
					battery_mark_min_time_minutes = battery_mark_time_count_minutes;
					battery_mark_min_time = battery_mark_time_count;
				}

				//max time update
				if (battery_mark_time_count > battery_mark_max_time)
				{
					battery_mark_max_time_seconds = battery_mark_time_count_seconds;
					battery_mark_max_time_minutes = battery_mark_time_count_minutes;
					battery_mark_max_time = battery_mark_time_count;
				}

				//avg time update
				battery_mark_avg_time_minutes = 0;
				battery_mark_avg_time = (battery_mark_avg_time + battery_mark_time_count);
				battery_mark_avg_calculate_time = battery_mark_avg_time / battery_mark_less_count;
				battery_mark_left_calculate_time = battery_mark_avg_calculate_time;
				while (true)
				{
					if (battery_mark_avg_calculate_time >= 60)
					{
						battery_mark_avg_calculate_time = battery_mark_avg_calculate_time - 60;
						battery_mark_avg_time_minutes = battery_mark_avg_time_minutes + 1;
					}
					else
					{
						battery_mark_avg_time_seconds = battery_mark_avg_calculate_time;
						break;
					}
				}

				//left time update
				battery_mark_left_time_minutes = 0;
				battery_mark_left_time_hours = 0;
				battery_mark_left_calculate_time = battery_mark_left_calculate_time * (battery_mark_mode_percent[battery_mark_select_num] - battery_mark_less_count);
				while (true)
				{
					if (battery_mark_left_calculate_time >= 3600)
					{
						battery_mark_left_calculate_time = battery_mark_left_calculate_time - 3600;
						battery_mark_left_time_hours = battery_mark_left_time_hours + 1;
					}
					else if (battery_mark_left_calculate_time >= 60)
					{
						battery_mark_left_calculate_time = battery_mark_left_calculate_time - 60;
						battery_mark_left_time_minutes = battery_mark_left_time_minutes + 1;
					}
					else
					{
						battery_mark_left_time_seconds = battery_mark_left_calculate_time;
						break;
					}
				}

				battery_mark_time_count = 0;
				battery_mark_time_count_minutes = 0;
				battery_mark_time_count_seconds = 0;
			}

			if (battery_mark_battery_level_start - battery_mark_mode_percent[battery_mark_select_num] == battery_level)
			{
				battery_mark_end_time_hours = hours;
				battery_mark_end_time_minutes = minutes;
				battery_mark_end_time_seconds = seconds;
				battery_mark_battery_voltage_end = battery_voltage_calculate;
				battery_mark_running = false;
				battery_mark_finish = true;
				Battery_mark_result();
				CPUload_stop();
				consoleSelect(&Screen_top);
			}

			printf("\x1b[10;4H| State : %s                    |", battery_mark_run_msg[battery_mark_count]);
			battery_mark_count = battery_mark_count + 1;
			if (battery_mark_count == 6)
			{
				battery_mark_count = 0;
			}

			if (battery_charge == 1)
			{
				battery_mark_running = false;
				battery_mark_abort = true;
			}
		}
		else if(battery_mark_abort)
		{
			printf("\x1b[10;4H| State : \x1b[31mAborted : don't charge battery\x1b[30m  |");
		}
		else if (battery_mark_finish)
		{
			printf("\x1b[10;4H| State : \x1b[32mFinished\x1b[30m                        |");
		}
		else
		{
			printf("\x1b[10;4H| State : Wait for decreasing to %02d%%      |", battery_mark_battery_level_start);
			if (battery_mark_battery_level_start == battery_level)
			{
				battery_mark_start_time_hours = hours;
				battery_mark_start_time_minutes = minutes;
				battery_mark_start_time_seconds = seconds;
				battery_mark_battery_before = battery_level;
				battery_mark_battery_voltage_start = battery_voltage_calculate;
				battery_mark_time_count = 0;
				battery_mark_time_count_minutes = 0;
				battery_mark_time_count_seconds = 0;
				battery_mark_less_count = 0;
				battery_mark_avg_calculate_time = 0;
				battery_mark_avg_time = 0;
				battery_mark_avg_time_seconds = -1;
				battery_mark_avg_time_minutes = -1;
				battery_mark_total_time = 0;
				battery_mark_total_calculate_time = 0;
				battery_mark_total_time_seconds = 0;
				battery_mark_total_time_minutes = 0;
				battery_mark_total_time_hours = 0;
				battery_mark_running = true;
				main_sleep = 250000;
			}
		}

		hidScanInput();
		u32 key = hidKeysDown();
		if (input_wait)
		{
			if (key & KEY_A)
			{
				input_wait = false;
				battery_mark_abort = false;
				battery_mark_running = false;
				battery_mark_run = false;
				battery_mark_finish = false;
				main_sleep = 50000;
				printf("\x1b[40m\x1b[37m");
				CPUload_stop();
				consoleSelect(&Screen_top);
				consoleClear();
				break;
			}
			else if (key & KEY_B)
			{
				input_wait = false;
			}
		}
		else
		{
			if (key & KEY_B)
			{
				input_wait = true;
			}
		}
	}
}

int main()
{
	osSetSpeedupEnable(true);
	gfxInitDefault();
	//gfxSet3D(true); //Uncomment if using stereoscopic 3D
	consoleInit(GFX_TOP, &Screen_top); //Change this line to consoleInit(GFX_BOTTOM, NULL) if using the bottom screen.
	consoleInit(GFX_BOTTOM, &Screen_bottom);

	Debug_mode_change_dialog();

	if (debug)
	{
		Debug_mode_setting_dialog();
	}

	consoleSelect(&Screen_bottom);

	if (ptmu_init_flag)
	{
		Result ptmu_result = ptmuInit();
		if (debug)
		{
			if (ptmu_result == 0)
			{
				ptmu_init_succes = true;
				printf("[%02d:%02d:%02d] \x1b[45mPTMU Init \x1b[1m\x1b[32mSucces!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[32m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, ptmu_result);
			}
			else
			{
				printf("[%02d:%02d:%02d] \x1b[45mPTMU Init \x1b[1m\x1b[33mFailed!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[33m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, ptmu_result);
			}
		}
		else
		{
			if (ptmu_result == 0)
			{
				ptmu_init_succes = true;
			}
		}
	}

	if (mcu_init_flag)
	{
		Result mcu_result = mcuHwcInit();
		if (debug)
		{
			if (mcu_result == 0)
			{
				mcu_init_succes = true;
				printf("[%02d:%02d:%02d] \x1b[45mMCU Init \x1b[1m\x1b[32mSucces!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[32m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, mcu_result);
			}
			else
			{
				printf("[%02d:%02d:%02d] \x1b[45mMCU Init \x1b[1m\x1b[33mFailed!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[33m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, mcu_result);
			}
		}
		else
		{
			if (mcu_result == 0)
			{
				mcu_init_succes = true;
			}
		}
	}

	if (apt_init_flag)
	{
		Result apt_result = aptInit();
		Result set_cpu_limit_result = APT_SetAppCpuTimeLimit(30);
		aptSetSleepAllowed(false);
		if (debug)
		{
			if (apt_result == 0)
			{
				apt_init_succes = true;
				printf("[%02d:%02d:%02d] \x1b[45mAPT Init \x1b[1m\x1b[32mSucces!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[32m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, apt_result);
			}
			else
			{
				printf("[%02d:%02d:%02d] \x1b[45mAPT Init \x1b[1m\x1b[33mFailed!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[33m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, apt_result);
			}
			if (set_cpu_limit_result == 0)
			{
				printf("[%02d:%02d:%02d] \x1b[45mSet cpu limit \x1b[1m\x1b[32mSucces!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[32m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, set_cpu_limit_result);
			}
			else
			{
				printf("[%02d:%02d:%02d] \x1b[45mSet cpu limit \x1b[1m\x1b[33mFailed!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[33m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, set_cpu_limit_result);
			}
		}
		else
		{
			if (apt_result == 0)
			{
				apt_init_succes = true;
			}
		}
	}

	if (news_init_flag)
	{
		Result news_result = newsInit();
		if (debug)
		{
			if (news_result == 0)
			{
				news_init_succes = true;
				printf("[%02d:%02d:%02d] \x1b[45mNEWS Init \x1b[1m\x1b[32mSucces!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[32m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, news_result);
			}
			else
			{
				printf("[%02d:%02d:%02d] \x1b[45mNEWS Init \x1b[1m\x1b[33mFailed!\x1b[0m\x1b[45m\x1b[37m code \x1b[1m\x1b[33m%ld\x1b[0m\x1b[37m\x1b[40m\n", hours, minutes, seconds, news_result);
			}
		}
		else
		{
			if (news_result == 0)
			{
				news_init_succes = true;
			}
		}
	}

	update_thread_run = true;
	update_thread = threadCreate(Update_thread, (void*)(""), STACKSIZE, 0x24, -1, false);

	timer_run = true;
	timer_thread = threadCreate(Timer_thread, (void*)(""), STACKSIZE, 0x24, -1, false);

	// Main loop
	while (aptMainLoop())
	{
		//Your code goes here

		if (battery_mark_run)
		{
			Battery_mark();
		}

		gspWaitForVBlank();
		hidScanInput();

		if (debug)
		{
			Debug();
		}

		Data_update();
		Data_update_bottom();

		usleep(main_sleep);
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		if (kDown & KEY_START)
		{
			Exit_check_dialog();
			if (app_exit)
			{
				Data_update();
				consoleSelect(&Screen_top);
				printf("\x1b[24;0HExiting...");

				if (cpu_load)
				{
					CPUload_stop();
				}
				timer_run = false;

				while (!timer_exit)
				{
					usleep(100000);
				}
				usleep(100000);
				break; //Break in order to return to hbmenu
			}
		}
		else if (kDown & KEY_A)
		{
			if (cpu_load)
			{
				CPUload_stop();
			}
			else
			{
				cpu_load = true;
				CPUload_start();
				CPUload_start_wait();
			}
		}
		else if (kDown & KEY_B)
		{
			Debug_mode_change_dialog();
		}
		else if (kHeld & KEY_DOWN)
		{
			usleep(100000);
			thread_num--;
			if (thread_num < 1)
			{
				thread_num = 1;
			}
		}
		else if (kHeld & KEY_UP)
		{
			usleep(100000);
			thread_num++;
			if (thread_num > Thread_max)
			{
				thread_num = Thread_max;
			}
		}
		else if (kDown & KEY_Y)
		{
			if (apt_init_succes)
			{
				Change_cpu_limit();
			}
		}
		else if (kDown & KEY_X)
		{
			if (news_init_succes)
			{
				if (news_cool_time == 0)
				{
					u16 test_title[] = { B,a,t,t,e,r,y,msg_space,c,h,e,c,k,e,r };
					int test_title_length = sizeof test_title / sizeof test_title[0];
					u32 titlelength = test_title_length - 1;

					u16 message_num[] = { _0,_1,_2,_3,_4,_5,_6,_7,_8,_9,msg_space };
					u16 message_no_yes[] = { msg_space,N,o,Y,e,s };

					Check_time();
					Check_battery();

					u16 test_message[] = { msg_0,message_num[hour_0],message_num[hour_1],msg_2,message_num[minute_0],message_num[minute_1],msg_2,message_num[second_0],message_num[second_1],msg_1
						,B,L,msg_3,message_num[battery_0],message_num[battery_1],msg_percent,msg_space
						,message_num[battery_v_0],msg_4,message_num[battery_v_1],message_num[battery_v_2],v,msg_space
						,B,C,msg_3,message_no_yes[battery_c_0],message_no_yes[battery_c_1],message_no_yes[battery_c_2],msg_space
						, 0x0 };
					u32 messageLength = 1000;

					NEWS_AddNotification(test_title, titlelength, test_message, messageLength, NULL, 0, false);
					news_cool_time = 20;
				}
			}
		}
		else if (kHeld & KEY_L && kHeld & KEY_R)
		{
			Battery_mark_init();
		}

		// Flush and swap frame-buffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	newsExit();
	ptmuExit();
	aptExit();
	mcuHwcExit();
	gfxExit();
	return 0;
}