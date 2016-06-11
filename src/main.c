#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"

static volatile os_timer_t some_timer;

void scan_callback(void *arg, STATUS status)
{
	if (status != OK) {
		os_printf("Scan failed\n");
		return;
	}

	struct bss_info *bss = (struct bss_info *)arg;
	while (bss != NULL) {
		os_printf("%32s (channel %d)\n", bss->ssid, bss->channel);
		bss = bss->next.stqe_next;
	}
	os_printf("\n");
}

void some_timerfunc(void *arg) { wifi_station_scan(NULL, scan_callback); }

void ICACHE_FLASH_ATTR user_init()
{
	/* set baud rate */
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	os_printf("Starting...\n");

	/* we operate in station mode */
	wifi_set_opmode(1);

	/* setup timer (5000ms, repeating) */
	os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);
	os_timer_arm(&some_timer, 5000, 1);
}
