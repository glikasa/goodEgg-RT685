#ifndef __MQTT_MANAGER__
#define __MQTT_MANAGER__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>


int mqtt_publish_async(const char *topic, const uint8_t *data, size_t len, uint8_t qos);

bool mqtt_is_connected();


#endif
