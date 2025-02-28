#include "can.h"
#include "esp32-hal-log.h"

namespace hardware
{
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    void task_can_watchdog(void* pv_parameters)
    {
        if (pv_parameters)
        {
            const auto can = static_cast<Can*>(pv_parameters);
            constexpr TickType_t x_delay = 200 / portTICK_PERIOD_MS;
            for (;;)
            {
                vTaskDelay(x_delay);
                can->handle_can_watchdog();
            }
        }
    }

    void task_can_receive(void* pv_parameters)
    {
        if (pv_parameters)
        {
            const auto can = static_cast<Can*>(pv_parameters);
            for (;;)
            {
                if (!can->handle_can_receive()) vTaskDelay(pdMS_TO_TICKS(CAN_RECEIVE_MS_TO_TICKS));
            }
        }
    }

    void Can::on_response(void* p_value, void* p_parameters)
    {
        auto* frame = static_cast<CanFrame*>(p_value);
        auto* can = static_cast<Can*>(p_parameters);
        can->send(*frame);
    }

#pragma clang diagnostic pop

    Can::Can(gpio_num_t gpio_tx, gpio_num_t gpio_rx) :
        thread_can_watchdog("TASK_CAN_WATCHDOG", 2048, 10),
        thread_can_receive("TASK_CAN_RECEIVE", 4096, 19),
        callback(CAN_RX_BUFFER_SIZE, sizeof(CanFrame), "CALLBACK_CAN",
                 2048),
        semaphore(true)
    {
        twai_general_config = TWAI_GENERAL_CONFIG_DEFAULT(gpio_tx, gpio_rx, TWAI_MODE_NORMAL);
        twai_timing_config = TWAI_TIMING_CONFIG_125KBITS();
        twai_filter_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        callback.parent_callback.set(on_response, this);
        clear_filter();
    }

    Can::~Can()
    {
        end();
    }

    bool Can::twai_install_and_start()
    {
        if (!twai_ready)
        {
            if (twai_driver_install(&twai_general_config, &twai_timing_config, &twai_filter_config) == ESP_OK)
            {
                log_i("TWAI driver installed");
                if (twai_start() == ESP_OK)
                {
                    log_i("TWAI driver started");
                    twai_ready = true;
                }
                else
                {
                    log_w("Failed to start TWAI driver");
                }
            }
            else
            {
                log_w("Failed to install TWAI driver");
            }
        }
        else
        {
            log_w("The driver is already installed");
        }
        return twai_ready;
    }

    void Can::twai_stop_and_uninstall()
    {
        twai_ready = false;
        twai_stop();
        vTaskDelay(pdMS_TO_TICKS(100));
        log_i("TWAI driver stopped");

        twai_driver_uninstall();
        log_i("TWAI driver uninstalled");
    }

    bool Can::begin(const can_speed_t speed)
    {
        bool result = false;
        if (semaphore.take())
        {
            if (twai_general_config.tx_io != gpio_num_t::GPIO_NUM_NC &&
                twai_general_config.rx_io != gpio_num_t::GPIO_NUM_NC)
            {
                if (twai_ready) twai_stop_and_uninstall();
                set_timing(speed);
                result = twai_install_and_start() && thread_can_receive.start(&task_can_receive, this, 1) &&
                    thread_can_watchdog.start(&task_can_watchdog, this, 1);
            }
            else
            {
                log_w("The object has not been initialized");
            }
            semaphore.give();
        }
        return result;
    }

    void Can::end()
    {
        if (semaphore.take())
        {
            if (twai_ready)
            {
                thread_can_watchdog.stop();
                thread_can_receive.stop();
                twai_stop_and_uninstall();
            }
            semaphore.give();
        }
    }

    twai_state_t Can::state() const
    {
        return twai_status_info.state;
    }

    bool Can::wait_running(const unsigned long timeout) const
    {
        const unsigned long ms_stop = millis() + timeout;
        bool result;
        while (!((result = twai_status_info.state == twai_state_t::TWAI_STATE_RUNNING)) &&
            (timeout == 0 || ms_stop > millis()))
            vTaskDelay(pdMS_TO_TICKS(50));

        return result;
    }

    bool Can::set_timing(const can_speed_t speed)
    {
        bool result = false;
        if (semaphore.take())
        {
            if (twai_ready)
            {
                switch (speed)
                {
                case can_speed_t::CAN_SPEED_25KBIT:
                    twai_timing_config = TWAI_TIMING_CONFIG_25KBITS();
                    log_i("Canbus rate 25KBit");
                    break;
                case can_speed_t::CAN_SPEED_50KBIT:
                    twai_timing_config = TWAI_TIMING_CONFIG_50KBITS();
                    log_i("Canbus rate 50KBit");
                    break;
                case can_speed_t::CAN_SPEED_100KBIT:
                    twai_timing_config = TWAI_TIMING_CONFIG_100KBITS();
                    log_i("Canbus rate 100KBit");
                    break;
                case can_speed_t::CAN_SPEED_125KBIT:
                    twai_timing_config = TWAI_TIMING_CONFIG_125KBITS();
                    log_i("Canbus rate 125KBit");
                    break;
                case can_speed_t::CAN_SPEED_250KBIT:
                    twai_timing_config = TWAI_TIMING_CONFIG_250KBITS();
                    log_i("Canbus rate 250KBit");
                    break;
                case can_speed_t::CAN_SPEED_500KBIT:
                    twai_timing_config = TWAI_TIMING_CONFIG_500KBITS();
                    log_i("Canbus rate 500KBit");
                    break;
                case can_speed_t::CAN_SPEED_800KBIT:
                    twai_timing_config = TWAI_TIMING_CONFIG_800KBITS();
                    log_i("Canbus rate 800KBit");
                    break;
                case can_speed_t::CAN_SPEED_1MBIT:
                    twai_timing_config = TWAI_TIMING_CONFIG_1MBITS();
                    log_i("Canbus rate 1MBit");
                    break;
                }
                result = true;
                log_d("The speed of the can bus has been changed");
            }
            else
            {
                log_d("The object was not running");
            }
            semaphore.give();
        }
        return result;
    }

    int Can::set_filter(const uint8_t index, const uint32_t id, const uint32_t mask, const bool extended,
                        const int16_t index_callback)
    {
        int result;
        if (index < CAN_NUM_FILTER && semaphore.take())
        {
            can_filter_t* filter = &filters[index];
            filter->configured = true;
            filter->extended = extended;
            filter->id = id & mask;
            filter->mask = mask;
            filter->index_callback = index_callback;
            result = index;
            log_d("The filter was added successfully");
            semaphore.give();
        }
        else
        {
            result = -1;
            log_d("Error adding the filter. The filter index is outside the range");
        }
        return result;
    }

    int Can::set_filter(const uint32_t id, const uint32_t mask, const bool extended, const int16_t index_callback)
    {
        for (int i = 0; i < CAN_NUM_FILTER; i++)
        {
            if (!filters[i].configured)
                return set_filter(i, id, mask, extended, index_callback);
        }
        log_w("Could not set filter");
        return -1;
    }

    can_filter_t Can::get_filter(const int16_t index) const
    {
        return index > -1 && index < CAN_NUM_FILTER ? filters[index] : can_filter_t();
    }

    void Can::clear_filter()
    {
        if (semaphore.take())
        {
            for (auto& filter : filters)
            {
                filter.configured = false;
                filter.extended = false;
                filter.id = 0;
                filter.mask = 0;
                filter.index_callback = -1;
            }
            semaphore.give();
        }
    }

    void Can::frame_processing(const twai_message_t& twai_message)
    {
        for (int8_t i = 0; i < CAN_NUM_FILTER; i++)
        {
            const can_filter_t* filter = &filters[i];
            if (filter->configured)
            {
                if ((twai_message.identifier & filter->mask) == filter->id &&
                    (twai_message.extd == filter->extended))
                {
                    CanFrame frame;
                    frame.id = twai_message.identifier;
                    frame.length = twai_message.data_length_code;
                    frame.rtr = twai_message.rtr;
                    frame.extended = twai_message.extd;
                    frame.f_idx = i;
                    memcpy(frame.data.bytes, twai_message.data, CAN_FRAME_DATA_SIZE);
                    callback.call(&frame, i);

                    log_d("The message 0x%04x was received by the filter successfully", twai_message.identifier);
                    return;
                }
            }
        }
        CanFrame frame;
        frame.id = twai_message.identifier;
        frame.length = twai_message.data_length_code;
        frame.rtr = twai_message.rtr;
        frame.extended = twai_message.extd;
        frame.f_idx = -1;
        memcpy(frame.data.bytes, twai_message.data, CAN_FRAME_DATA_SIZE);
        callback.call(&frame);

        log_d("Message 0x%04x receive successfully", twai_message.identifier);
    }

    bool Can::send(CanFrame& frame)
    {
        bool result = false;
        if (frame.is())
        {
            if (semaphore.take())
            {
                if (twai_ready && twai_status_info.state == TWAI_STATE_RUNNING)
                {
                    const unsigned long ms = millis();
                    if (frame.ms_next <= ms)
                    {
                        if (frame.freq != 0) frame.ms_next = ms + frame.freq;

                        twai_message_t message{};
                        message.identifier = frame.id;
                        message.data_length_code = frame.length;
                        message.rtr = frame.rtr;
                        message.extd = frame.extended;
                        memcpy(message.data, frame.data.bytes, CAN_FRAME_DATA_SIZE);

                        switch (twai_transmit(&message, pdMS_TO_TICKS(CAN_SEND_MS_TO_TICKS)))
                        {
                        case ESP_OK:
                            log_d("Message sent successfully");
                            result = true;
                            break;
                        case ESP_ERR_TIMEOUT:
                            log_w("Failed to send message: TIMEOUT");
                            break;
                        case ESP_ERR_INVALID_ARG:
                            log_w("Failed to send message: INVALID ARG");
                            break;
                        case ESP_FAIL:
                            log_w("Failed to send message: FAIL");
                            break;
                        case ESP_ERR_INVALID_STATE:
                            log_w("Failed to send message: INVALID STATE");
                            break;
                        case ESP_ERR_NOT_SUPPORTED:
                            log_w("Failed to send message: NOT SUPPORTED");
                            break;
                        default:
                            break;
                        }
                    }
                    else
                    {
                        log_d("Time is not to send data: %d > %d", frame.ms_next, ms);
                    }
                }
                else
                {
                    log_w("Canbus is not running");
                }
                semaphore.give();
            }
        }
        else
        {
            log_w("Frame data is missing");
        }
        return result;
    }

    bool Can::receive(CanFrame& frame)
    {
        return callback.read(&frame);
    }

    void Can::handle_can_watchdog()
    {
        if (twai_get_status_info(&twai_status_info) == ESP_OK)
        {
            if (twai_status_info.state == TWAI_STATE_BUS_OFF)
            {
                if (twai_initiate_recovery() != ESP_OK)
                    log_w("Could not initiate bus recovery");
            }
        }
    }

    bool Can::handle_can_receive()
    {
        if (twai_ready)
        {
            twai_message_t twai_message;
            if (twai_receive(&twai_message, pdMS_TO_TICKS(CAN_RECEIVE_MS_TO_TICKS)) == ESP_OK)
                frame_processing(twai_message);
        }
        return twai_ready;
    }
}
