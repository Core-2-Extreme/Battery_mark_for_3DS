#include <3ds.h>
#include <string>

#include "headers.hpp"

int Change_brightness(bool top_screen, bool bottom_screen, int brightness)
{
	gspLcdInit();
	Result function_result;

	if (top_screen && bottom_screen)
		function_result = GSPLCD_SetBrightnessRaw(GSPLCD_SCREEN_BOTH, brightness);
	else if (top_screen)
		function_result = GSPLCD_SetBrightnessRaw(GSPLCD_SCREEN_TOP, brightness);
	else if (bottom_screen)
		function_result = GSPLCD_SetBrightnessRaw(GSPLCD_SCREEN_BOTTOM, brightness);
	else
		function_result = -1;

	gspLcdExit();
	return function_result;
}

int Wifi_disable()
{
	nwmExtInit();
	Result function_result;

	function_result = NWMEXT_ControlWirelessEnabled(false);

	nwmExtExit();
	return function_result;
}

int Wifi_enable()
{
	nwmExtInit();
	Result function_result;

	function_result = NWMEXT_ControlWirelessEnabled(true);

	nwmExtExit();
	return function_result;
}

int Change_screen_state(bool top_screen, bool bottom_screen, bool state)
{
	gspLcdInit();
	Result function_result;

	if (top_screen && bottom_screen)
	{
		if (state)
			function_result = GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);
		else
			function_result = GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTH);
	}
	else if (top_screen)
	{
		if (state)
			function_result = GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_TOP);
		else
			function_result = GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_TOP);
	}
	else if (bottom_screen)
	{
		if (state)
			function_result = GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTTOM);
		else
			function_result = GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTTOM);
	}
	else
		function_result = -1;

	gspLcdExit();
	return function_result;
}
