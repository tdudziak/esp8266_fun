#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"

static volatile os_timer_t some_timer;

void some_timerfunc(void *arg) { os_printf_plus("Hello!\n"); }

void ICACHE_FLASH_ATTR user_init()
{
	// set baud rate
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	os_printf("Starting...\n");

	// setup timer (500ms, repeating)
	os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);
	os_timer_arm(&some_timer, 1000, 1);
}
