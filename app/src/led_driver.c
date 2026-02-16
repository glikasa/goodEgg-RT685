#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/sys/util.h>

#define STRIP_NODE      DT_ALIAS(led_strip)
#define STRIP_NUM_LEDS  DT_PROP(STRIP_NODE, chain_length)

#define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);
static struct led_rgb pixels[STRIP_NUM_LEDS];

/* Predefined colors */
static const struct led_rgb colors[] = {
    RGB(0xFF, 0x00, 0x00), /* Red */
    RGB(0x00, 0xFF, 0x00), /* Green */
    RGB(0x00, 0x00, 0xFF), /* Blue */
    RGB(0xFF, 0xFF, 0x00), /* Yellow */
    RGB(0xFF, 0x00, 0xFF), /* Magenta */
    RGB(0x00, 0xFF, 0xFF), /* Cyan */
    RGB(0xFF, 0xFF, 0xFF), /* White */
};

/* Set all LEDs to a single color */
void set_all_leds(struct led_rgb color){

    for (int i = 0; i < STRIP_NUM_LEDS; i++) {
        pixels[i] = color;
    }
    led_strip_update_rgb(strip, pixels, STRIP_NUM_LEDS);
}

/* Clear all LEDs */
void clear_all_leds(void){

    struct led_rgb off = RGB(0, 0, 0);
    set_all_leds(off);
}

/* Rainbow chase effect */
void rainbow_chase(int delay_ms){

    static int offset = 0;
    for(int i = 0; i < STRIP_NUM_LEDS; i++) {

        pixels[i] = colors[(i + offset) % ARRAY_SIZE(colors)];
    }
    led_strip_update_rgb(strip, pixels, STRIP_NUM_LEDS);
    offset = (offset + 1) % ARRAY_SIZE(colors);
    k_msleep(delay_ms);
}

/* Running light effect */
void running_light(struct led_rgb color, int delay_ms){

    for (int i = 0; i < STRIP_NUM_LEDS; i++) {
        clear_all_leds();
        pixels[i] = color;
        led_strip_update_rgb(strip, pixels, STRIP_NUM_LEDS);
        k_msleep(delay_ms);
    }
}

/* Fade effect */
void fade_in_out(struct led_rgb color, int steps, int delay_ms){

    struct led_rgb temp;
    /* Fade in */
    for (int j = 0; j <= steps; j++) {
        temp.r = (color.r * j) / steps;
        temp.g = (color.g * j) / steps;
        temp.b = (color.b * j) / steps;
        set_all_leds(temp);
        k_msleep(delay_ms);
    }
    
    /* Fade out */
    for (int j = steps; j >= 0; j--) {
        temp.r = (color.r * j) / steps;
        temp.g = (color.g * j) / steps;
        temp.b = (color.b * j) / steps;
        set_all_leds(temp);
        k_msleep(delay_ms);
    }
}

int main(void){

    int ret;

    printk("WS2812 LED Strip Demo\n");

    if(!device_is_ready(strip)) {
        printk("LED strip device %s is not ready\n", strip->name);
        return -1;
    }

    printk("Found LED strip device %s\n", strip->name);
    printk("Number of LEDs: %d\n", STRIP_NUM_LEDS);

    while (1) {
        /* Cycle through solid colors */
        printk("Solid colors...\n");
        for(int i = 0; i < ARRAY_SIZE(colors); i++) {
            set_all_leds(colors[i]);
            k_msleep(500);
        }

        /* Rainbow chase */
        printk("Rainbow chase...\n");
        for(int i = 0; i < 50; i++) {
            rainbow_chase(100);
        }

        /* Running light */
        printk("Running light...\n");
        for(int i = 0; i < ARRAY_SIZE(colors); i++) {

            running_light(colors[i], 50);
        }

        /* Fade effect */
        printk("Fade effect...\n");
        fade_in_out(colors[0], 50, 20);
        fade_in_out(colors[1], 50, 20);
        fade_in_out(colors[2], 50, 20);

        clear_all_leds();
        k_msleep(500);
    }

    return 0;
}
