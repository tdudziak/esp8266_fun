#include <stdarg.h>

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"

/* MQTT library from https://github.com/tuanpmt/esp_mqtt */
#include "mqtt.h"

static MQTT_Client mqtt_client;
static volatile os_timer_t some_timer;
static uint8 wifi_status = STATION_IDLE;

static void ICACHE_FLASH_ATTR dmesg(const char *format, ...)
{
	const size_t BUFFER_SIZE = 256;

	char *buffer = (char *)os_malloc(BUFFER_SIZE);
	if (!buffer)
		return;
	;

	va_list args;
	va_start(args, format);
	int n = ets_vsnprintf(buffer, BUFFER_SIZE, format, args);
	va_end(args);

	if (n > 0 && n < BUFFER_SIZE)
		MQTT_Publish(&mqtt_client, TOPIC_DMESG, buffer, n, 0, 1);

	os_free(buffer);
}

static void on_scan_ready(void *arg, STATUS status)
{
	if (status != OK) {
		dmesg("Scan failed");
		return;
	}

	struct bss_info *bss = (struct bss_info *)arg;
	// FIXME: would be much better to only send one message
	while (bss != NULL) {
		bss->ssid[sizeof(bss->ssid) - 1] = '\0';
		dmesg("ssid=%s channel=%d", bss->ssid, bss->channel);
		bss = bss->next.stqe_next;
	}
}

static void on_mqtt_connected(uint32_t *args)
{
	dmesg("Hello, world!");
	MQTT_InitLWT(&mqtt_client, TOPIC_DMESG, "Goodbye, cruel world!", 0, 1);
	MQTT_Subscribe(&mqtt_client, TOPIC_CONTROL, 0);
}

void on_timer(void *arg)
{
	uint8 new_status = wifi_station_get_connect_status();

	if (new_status != wifi_status) {
		if (new_status == STATION_GOT_IP) {
			MQTT_Connect(&mqtt_client);
		} else {
			MQTT_Disconnect(&mqtt_client);
		}
	}

	wifi_status = new_status;
}

void on_mqtt_data(uint32_t *args, const char *topic, uint32_t topic_len,
		  const char *data, uint32_t data_len)
{
	char cmd[256];

	if (data_len < sizeof(cmd)) {
		os_memcpy(cmd, data, data_len);
		cmd[data_len] = '\0';

		/* echo back the message */
		dmesg(cmd);

		if (strcmp("SCAN", cmd) == 0) {
			wifi_station_scan(NULL, on_scan_ready);
		} else if (strcmp("ADC_READ", cmd) == 0) {
			uint16 adc_in = system_adc_read();
			dmesg("adc_in=%hu", (unsigned short)adc_in);
		}
	}
}

void ICACHE_FLASH_ATTR user_init()
{
	/* set baud rate */
	uart_div_modify(0, UART_CLK_FREQ / 115200);

	wifi_set_opmode(1); /* station mode */
	struct station_config config;
	os_bzero(&config, sizeof(config));
	strncpy((char *)config.ssid, WIFI_SSID, sizeof(config.ssid));
	strncpy((char *)config.password, WIFI_PASS, sizeof(config.password));
	wifi_station_set_config(&config);

	MQTT_InitConnection(&mqtt_client, MQTT_HOST, MQTT_PORT, 0);
	MQTT_InitClient(&mqtt_client, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS,
			MQTT_KEEPALIVE, MQTT_CLEAN_SESSION);
	MQTT_OnConnected(&mqtt_client, on_mqtt_connected);
	MQTT_OnData(&mqtt_client, on_mqtt_data);

	dmesg("Starting up");

	/* setup timer (1000ms, repeating) */
	os_timer_setfn(&some_timer, (os_timer_func_t *)on_timer, NULL);
	os_timer_arm(&some_timer, 1000, 1);
}
