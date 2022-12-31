#include "can.h"

using namespace hardware;
volatile bool can::_init = false;
const char* can::TAG = "can";

bool can::driver_install(gpio_num_t gpio_tx, gpio_num_t gpio_rx, twai_mode_t mode, e_can_speed_t speed)
{
    if (_init) driver_uninstall();

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(gpio_tx, gpio_rx, mode);
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    twai_timing_config_t t_config;
    switch (speed) {

        case CAN_SPEED_25KBIT:
            t_config = TWAI_TIMING_CONFIG_25KBITS();
            ESP_LOGI(TAG, "Canbus rate 25KBit");
            break;
        case CAN_SPEED_50KBIT:
            t_config = TWAI_TIMING_CONFIG_50KBITS();
            ESP_LOGI(TAG, "Canbus rate 50KBit");
            break;
        case CAN_SPEED_100KBIT:
            t_config = TWAI_TIMING_CONFIG_100KBITS();
            ESP_LOGI(TAG, "Canbus rate 100KBit");
            break;
        case CAN_SPEED_125KBIT:
            t_config = TWAI_TIMING_CONFIG_125KBITS();
            ESP_LOGI(TAG, "Canbus rate 125KBit");
            break;
        case CAN_SPEED_250KBIT:
            t_config = TWAI_TIMING_CONFIG_250KBITS();
            ESP_LOGI(TAG, "Canbus rate 250KBit");
            break;
        case CAN_SPEED_500KBIT:
            t_config = TWAI_TIMING_CONFIG_500KBITS();
            ESP_LOGI(TAG, "Canbus rate 500KBit");
            break;
        case CAN_SPEED_800KBIT:
            t_config = TWAI_TIMING_CONFIG_800KBITS();
            ESP_LOGI(TAG, "Canbus rate 800KBit");
            break;
        default:
            t_config = TWAI_TIMING_CONFIG_1MBITS();
            ESP_LOGI(TAG, "Canbus rate 1MBit");
            break;
    }

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver installed, mode %d", mode);
        delay(100);

        if (twai_start() == ESP_OK) {
            ESP_LOGI(TAG, "TWAI driver started");
            delay(100);
            _init = true;
        } else {
            ESP_LOGW(TAG, "Failed to start TWAI driver");
            twai_driver_uninstall();
        }
    } else ESP_LOGW(TAG, "Failed to install TWAI driver");

    return _init;
}

void can::driver_uninstall()
{
    if (_init) {
        _init = false;

        twai_stop();
        delay(50);
        twai_driver_uninstall();
        ESP_LOGI(TAG, "TWAI driver uninstalled");
        delay(50);
    }
}

bool can::begin(gpio_num_t gpio_tx, gpio_num_t gpio_rx, e_can_speed_t speed)
{
    ESP_LOGI(TAG, "Canbus begin");
    return driver_install(gpio_tx, gpio_rx, TWAI_MODE_NORMAL, speed);
}

bool can::send(can_frame& frame, int timeout)
{
    if (!_init || !frame.is()) return false;

    if (frame.freq > 0) {
        unsigned long ms = millis();
        if (frame.ms_next > ms) return false;
        frame.ms_next = ms + frame.freq;
    }

    twai_message_t message = frame.get();
    esp_err_t err = twai_transmit(&message, pdMS_TO_TICKS(timeout));
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Message sent successfully");
        return true;
    }

    ESP_LOGW(TAG, "Failed to send message, ret:%02x", err);
    return false;
}

int can::receive(can_frame& frame, int timeout)
{
    if (!_init) return 0;

    twai_message_t message;
    esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(timeout));
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Message receive successfully");
        return frame.set(message);
    }
    return 0;
}
