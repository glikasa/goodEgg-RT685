
#include <zephyr/kernel.h>
#include "mqtt_manager.h"

int main(void)
{
    int counter = 0;
    char payload[64];

    printk("MQTT Publisher Started\n");

    /* Wait for MQTT to connect */
    while (!mqtt_is_connected()) {

        k_sleep(K_SECONDS(5));
    }

    while (1) {
        snprintf(payload, sizeof(payload), "{\"counter\":%d,\"temp\":25.5}", counter++);
        
        /* This returns immediately - message is queued and thread is signaled */
        int err = mqtt_publish_async("goodEgg/sensors/", payload, strlen(payload), 1);
        
        if (err) {
            printk("Failed to queue message: %d\n", err);
        }
        k_sleep(K_SECONDS(5));
    }
    return 0;
}
