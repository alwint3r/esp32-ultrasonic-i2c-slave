#include "ultrasonic.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

#define timeout_expired(start, len) ((esp_timer_get_time() - (start)) >= (len))

int ultrasonic_init(ultrasonic_sensor_t *sensor)
{
    if (sensor == NULL)
    {
        return -1;
    }

    gpio_config_t trigger_config;
    trigger_config.intr_type = GPIO_INTR_DISABLE;
    trigger_config.mode = GPIO_MODE_OUTPUT;
    trigger_config.pin_bit_mask = (1UL << sensor->trigger_pin);
    trigger_config.pull_down_en = 0;
    trigger_config.pull_up_en = 0;

    gpio_config(&trigger_config);

    gpio_config_t echo_config;
    echo_config.intr_type = GPIO_INTR_DISABLE;
    echo_config.mode = GPIO_MODE_INPUT;
    echo_config.pin_bit_mask = (1UL << sensor->echo_pin);
    echo_config.pull_down_en = 0;
    echo_config.pull_up_en = 0;

    gpio_config(&echo_config);

    return 0;
}

int ultrasonic_measure_distance(ultrasonic_sensor_t *sensor)
{
    if (sensor == NULL)
    {
        return -1;
    }

    portENTER_CRITICAL(&mux);

    gpio_set_level(sensor->trigger_pin, 0);
    ets_delay_us(4);
    gpio_set_level(sensor->trigger_pin, 1);
    ets_delay_us(10);
    gpio_set_level(sensor->trigger_pin, 0);

    if (gpio_get_level(sensor->echo_pin))
    {
        portEXIT_CRITICAL(&mux);
        return -1;
    }

    // wait for HIGH signal
    int64_t wait_start = esp_timer_get_time();
    while (!gpio_get_level(sensor->echo_pin))
    {
        if (timeout_expired(wait_start, 4 * 1000))
        {
            portEXIT_CRITICAL(&mux);
            return -1;
        }
    }

    int64_t pulse_start = esp_timer_get_time();
    int64_t current_time = pulse_start;
    while (gpio_get_level(sensor->echo_pin))
    {
        current_time = esp_timer_get_time();

        if (timeout_expired(pulse_start, 30 * 1000))
        {
            portEXIT_CRITICAL(&mux);
            return -1;
        }
    }

    portEXIT_CRITICAL(&mux);

    int pulse_width = (int)(current_time - pulse_start);

    return pulse_width / 58;
}
