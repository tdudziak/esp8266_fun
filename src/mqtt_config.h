#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__

#define MQTT_CLIENT_ID     "esp8266_tmd"
#define MQTT_USER          ""
#define MQTT_PASS          ""

#define MQTT_HOST          "test.mosquitto.org"
#define MQTT_PORT          1883
#define MQTT_BUF_SIZE      1024
#define MQTT_KEEPALIVE     20 /* seconds */
#define MQTT_CLEAN_SESSION 0
#define MQTT_TOPIC_PREFIX  MQTT_CLIENT_ID

#define MQTT_RECONNECT_TIMEOUT 5 /* seconds */
#define PROTOCOL_NAMEv31 /* MQTT version 3.1 compatible with Mosquitto v0.15 */

#define TOPIC_DMESG        (MQTT_TOPIC_PREFIX "/dmesg")
#define TOPIC_CONTROL      (MQTT_TOPIC_PREFIX "/control")

#endif /* __MQTT_CONFIG_H__ */
