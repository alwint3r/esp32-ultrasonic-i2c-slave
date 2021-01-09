#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include "driver/gpio.h"

// this ultrasonic related functions is directly inspired from
// esp-idf-lib ultrasonic component
// https://github.com/UncleRus/esp-idf-lib/blob/master/components/ultrasonic/ultrasonic.c
// credits goes to UncleRus.

typedef struct
{
    gpio_num_t trigger_pin;
    gpio_num_t echo_pin;
} ultrasonic_sensor_t;

int ultrasonic_init(ultrasonic_sensor_t *sensor);

int ultrasonic_measure_distance(ultrasonic_sensor_t *sensor);



#endif