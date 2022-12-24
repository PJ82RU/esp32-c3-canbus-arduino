#include "can.h"

using namespace hardware;
volatile bool can::_init = false;

bool can::driver_install(gpio_num_t gpio_tx, gpio_num_t gpio_rx, twai_mode_t mode, e_can_speed_t speed)
{
    if (_init) driver_uninstall();

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(gpio_tx, gpio_rx, mode);
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    twai_timing_config_t t_config;
    switch (speed) {

        case CAN_SPEED_25KBIT:
            t_config = TWAI_TIMING_CONFIG_25KBITS();
            break;
        case CAN_SPEED_50KBIT:
            t_config = TWAI_TIMING_CONFIG_50KBITS();
            break;
        case CAN_SPEED_100KBIT:
            t_config = TWAI_TIMING_CONFIG_100KBITS();
            break;
        case CAN_SPEED_125KBIT:
            t_config = TWAI_TIMING_CONFIG_125KBITS();
            break;
        case CAN_SPEED_250KBIT:
            t_config = TWAI_TIMING_CONFIG_250KBITS();
            break;
        case CAN_SPEED_500KBIT:
            t_config = TWAI_TIMING_CONFIG_500KBITS();
            break;
        case CAN_SPEED_800KBIT:
            t_config = TWAI_TIMING_CONFIG_800KBITS();
            break;
        default:
            t_config = TWAI_TIMING_CONFIG_1MBITS();
            break;
    }

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        delay(100);

        if (twai_start() == ESP_OK) {
            delay(100);
            _init = true;
        } else twai_driver_uninstall();
    }

    return _init;
}

void can::driver_uninstall()
{
    if (_init) {
        _init = false;

        twai_stop();
        delay(50);
        twai_driver_uninstall();
        delay(50);
    }
}

bool can::begin(gpio_num_t gpio_tx, gpio_num_t gpio_rx, e_can_speed_t speed)
{
    return driver_install(gpio_tx, gpio_rx, TWAI_MODE_NORMAL, speed);
}

bool can::send(can_frame& frame, int timeout)
{
    if (!_init || !frame.is()) return false;

    twai_message_t message = frame.get();
    return twai_transmit(&message, pdMS_TO_TICKS(timeout)) == ESP_OK;
}

int can::receive(can_frame& frame, int timeout)
{
    if (!_init) return 0;

    twai_message_t message;
    return twai_receive(&message, pdMS_TO_TICKS(timeout)) == ESP_OK ? frame.set(message) : 0;
}
