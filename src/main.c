#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"

/* MQTT library from https://github.com/tuanpmt/esp_mqtt */
#include "mqtt.h"

static MQTT_Client mqtt_client;
static volatile os_timer_t some_timer;
static uint8 wifi_status = STATION_IDLE;

struct station_config STATION_CONFIG = {
	.ssid = WIFI_SSID,
	.password = WIFI_PASS,
	.bssid_set = 0
};

static void on_mqtt_connected(uint32_t *args)
{
	os_printf("Connected to MQTT broker\n");
	const char *msg = "Hello, world!";
	MQTT_Publish(&mqtt_client, MQTT_TOPIC_PREFIX "/hello", msg, strlen(msg),
		     0, 0);
}

static void on_scan_ready(void *arg, STATUS status)
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

void on_timer(void *arg)
{
	uint8 new_status = wifi_station_get_connect_status();

	if (new_status != wifi_status) {
		if (new_status == STATION_GOT_IP) {
			os_printf("Attempting to connect to MQTT broker\n");
			MQTT_Connect(&mqtt_client);
		} else {
			MQTT_Disconnect(&mqtt_client);
		}
	}

	if (new_status != STATION_GOT_IP) {
		os_printf("Not connected, initiating scan...\n");
		wifi_station_scan(NULL, on_scan_ready);
	}

	wifi_status = new_status;
}

void ICACHE_FLASH_ATTR user_init()
{
	/* set baud rate */
	uart_div_modify(0, UART_CLK_FREQ / 115200);
	os_printf("Starting...\n");

	wifi_set_opmode(1); /* station mode */
	wifi_station_set_config(&STATION_CONFIG);

	MQTT_InitConnection(&mqtt_client, MQTT_HOST, MQTT_PORT, 0);
	MQTT_OnConnected(&mqtt_client, on_mqtt_connected);
	MQTT_InitClient(&mqtt_client, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS,
			MQTT_KEEPALIVE, MQTT_CLEAN_SESSION);

	/* setup timer (5000ms, repeating) */
	os_timer_setfn(&some_timer, (os_timer_func_t *)on_timer, NULL);
	os_timer_arm(&some_timer, 5000, 1);
}
