#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define US_ECHO_PIN GPIO_NUM_4
#define US_TRIG_PIN GPIO_NUM_5

static const char *TAG = "app";

#include "ultrasonic.h"
ultrasonic_sensor_t ultrasonic;

void print_buffer(uint8_t *buffer, size_t length);

static int last_distance = 0;

static void handle_distance_reading_request(uint8_t *request, size_t length);

static int i2c_slave_port = I2C_NUM_0;

static SemaphoreHandle_t slave_smphr;

void app_main()
{
    ultrasonic.echo_pin = US_ECHO_PIN;
    ultrasonic.trigger_pin = US_TRIG_PIN;

    if (ultrasonic_init(&ultrasonic) != 0)
    {
        ESP_LOGE(TAG, "Failed to initialize ultrasonic sensor.");
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        esp_restart();
    }

    slave_smphr = xSemaphoreCreateMutex();

    i2c_config_t slave_config = {
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .mode = I2C_MODE_SLAVE,
        .slave.addr_10bit_en = 0,
        .slave.slave_addr = 0x70,
    };

    esp_err_t err = i2c_param_config(i2c_slave_port, &slave_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure I2C slave, error: %d", (int)err);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        esp_restart();
    }

    err = i2c_driver_install(i2c_slave_port, I2C_MODE_SLAVE, 512, 512, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install driver, error: %d", (int)err);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        esp_restart();
    }

    while (1)
    {
        uint8_t buffer[32] = {0};
        int data_counter = 0;

        while (1)
        {
            if (xSemaphoreTake(slave_smphr, 100 / portTICK_PERIOD_MS) == pdFAIL)
            {
                break;
            }

            uint8_t read_data;
            int read_length = i2c_slave_read_buffer(i2c_slave_port, &read_data, 1, 1000 / portTICK_PERIOD_MS);

            xSemaphoreGive(slave_smphr);

            if (read_length == 1)
            {
                buffer[data_counter++] = read_data;
            }

            if (read_length == 0)
            {
                break;
            }
        }

        if (data_counter < 1)
        {
            continue;
        }

        ESP_LOGI(TAG, "Received %d bytes I2C packets.", data_counter);
        print_buffer(buffer, 32);

        if (buffer[0] == 0x01 || buffer[0] == 0x02)
        {
            // handle distance reading request.
            handle_distance_reading_request(buffer, data_counter);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void handle_distance_reading_request(uint8_t *request, size_t length)
{
    if (length < 1)
    {
        return;
    }

    if (xSemaphoreTake(slave_smphr, 100 / portTICK_PERIOD_MS) == pdFAIL)
    {
        ESP_LOGE(TAG, "Failed to acquire slave semaphore");
    }

    int output_distance = 0;

    int distance = ultrasonic_measure_distance(&ultrasonic);
    if (distance <= 0)
    {
        ESP_LOGI(TAG, "Failed to measure distance. Using last measurement instead");
        output_distance = last_distance;
    }
    else
    {
        last_distance = distance;
        output_distance = distance;
    }

    ESP_LOGI(TAG, "Measured distance: %d cm", output_distance);

    uint8_t output[2] = {(output_distance >> 0), (output_distance >> 8)};

    int err = i2c_slave_write_buffer(i2c_slave_port, output, 2, 100 / portTICK_PERIOD_MS);
    if (err == (int)ESP_FAIL)
    {
        ESP_LOGI(TAG, "Failed to write to I2C buffer for DISTANCE_L|DISTANCE_H");
    }

    ESP_LOGI(TAG, "Output buffer:");
    print_buffer(output, 2);

    xSemaphoreGive(slave_smphr);
}

void print_buffer(uint8_t *buffer, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}
