#include "can.h"
#include "esp32-hal-log.h"

namespace hardware {

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    void task_can_watchdog(void *pv_parameters) {
        if (!pv_parameters) return;
        Can *can = (Can *) pv_parameters;
        const TickType_t x_delay = 200 / portTICK_PERIOD_MS;

        for (;;) {
            vTaskDelay(x_delay);

            if (twai_get_status_info(&can->twai_status_info) == ESP_OK) {
                if (can->twai_status_info.state == TWAI_STATE_BUS_OFF) {
                    if (twai_initiate_recovery() != ESP_OK)
                        log_w("Could not initiate bus recovery");
                }
            }
        }
    }

    void task_can_receive(void *pv_parameters) {
        if (!pv_parameters) return;
        Can *can = (Can *) pv_parameters;

        for (;;) {
            twai_message_t twai_message;
            if (can->twai_ready) {
                if (twai_receive(&twai_message, pdMS_TO_TICKS(CAN_RECEIVE_MS_TO_TICKS)) == ESP_OK)
                    can->frame_processing(twai_message);
            } else vTaskDelay(pdMS_TO_TICKS(CAN_RECEIVE_MS_TO_TICKS));
        }
    }

    void Can::on_response(void *p_value, void *p_parameters) {
        auto *frame = (CanFrame *) p_value;
        auto *can = (Can *) p_parameters;
        can->send(*frame);
    }

#pragma clang diagnostic pop

    Can::Can(gpio_num_t gpio_tx, gpio_num_t gpio_rx) : callback(CAN_RX_BUFFER_SIZE, sizeof(CanFrame), "CAN_CALLBACK") {
        twai_general_config = TWAI_GENERAL_CONFIG_DEFAULT(gpio_tx, gpio_rx, TWAI_MODE_NORMAL);
        twai_timing_config = TWAI_TIMING_CONFIG_125KBITS();
        twai_filter_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

        mutex = xSemaphoreCreateMutex();
        clear_filter();

        callback.cb_receive = on_response;
        callback.p_receive_parameters = this;

        xTaskCreatePinnedToCore(&task_can_receive, "CAN_RECEIVE", 4096, this, 19, &task_receive, 1);
        log_i("Task receive created");

        xTaskCreatePinnedToCore(&task_can_watchdog, "CAN_WATCHDOG", 2048, this, 10, &task_watchdog, 1);
        log_i("Task watchdog created");
    }

    Can::~Can() {
        end();

        vTaskDelete(task_watchdog);
        log_i("Task watchdog deleted");
        vTaskDelete(task_receive);
        log_i("Task receive deleted");
    }

    bool Can::twai_install_and_start() {
        if (!twai_ready) {
            if (twai_driver_install(&twai_general_config, &twai_timing_config, &twai_filter_config) == ESP_OK) {
                log_i("TWAI driver installed");

                if (twai_start() == ESP_OK) {
                    log_i("TWAI driver started");
                    twai_ready = true;
                } else
                    log_w("Failed to start TWAI driver");
            } else
                log_w("Failed to install TWAI driver");
        } else
            log_w("The driver is already installed");

        return twai_ready;
    }

    void Can::twai_stop_and_uninstall() {
        twai_ready = false;
        twai_stop();
        vTaskDelay(pdMS_TO_TICKS(100));
        log_i("TWAI driver stopped");

        twai_driver_uninstall();
        log_i("TWAI driver uninstalled");
    }

    bool Can::begin(can_speed_t speed) {
        if (twai_ready) twai_stop_and_uninstall();

        set_timing(speed);
        return twai_install_and_start();
    }

    void Can::end() {
        if (twai_ready) twai_stop_and_uninstall();
    }

    bool Can::set_timing(can_speed_t speed) {
        if (twai_ready) return false;

        switch (speed) {
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
        return true;
    }

    int Can::set_filter(uint8_t index, uint32_t id, uint32_t mask, bool extended, int16_t index_callback) {
        if (index < CAN_NUM_FILTER) {
            if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
                can_filter_t *filter = &filters[index];
                filter->configured = true;
                filter->extended = extended;
                filter->id = id & mask;
                filter->mask = mask;
                filter->index_callback = index_callback;
                xSemaphoreGive(mutex);
            }
            return index;
        }
        return -1;
    }

    int Can::set_filter(uint32_t id, uint32_t mask, bool extended, int16_t index_callback) {
        for (int i = 0; i < CAN_NUM_FILTER; i++) {
            if (!filters[i].configured)
                return set_filter(i, id, mask, extended, index_callback);
        }
        log_w("Could not set filter");
        return -1;
    }

    can_filter_t Can::get_filter(int16_t index) {
        return index > -1 && index < CAN_NUM_FILTER ? filters[index] : can_filter_t();
    }

    void Can::clear_filter() {
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
            for (auto &filter: filters) {
                filter.configured = false;
                filter.extended = false;
                filter.id = 0;
                filter.mask = 0;
                filter.index_callback = -1;
            }
            xSemaphoreGive(mutex);
        }
    }

    void Can::frame_processing(twai_message_t &twai_message) {
        for (int8_t i = 0; i < CAN_NUM_FILTER; i++) {
            can_filter_t *filter = &filters[i];
            if (filter->configured) {
                if ((twai_message.identifier & filter->mask) == filter->id &&
                    (twai_message.extd == filter->extended)) {
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

    bool Can::send(CanFrame &frame) {
        if (!twai_ready || twai_status_info.state != TWAI_STATE_RUNNING) {
            log_w("Canbus is not running");
            return false;
        }
        if (!frame.is()) {
            log_w("Frame data is missing");
            return false;
        }
        if (frame.freq != 0) {
            unsigned long ms = millis();
            if (frame.ms_next > ms) {
                log_d("Time is not to send data: %d > %d", frame.ms_next, ms);
                return false;
            }
            frame.ms_next = ms + frame.freq;
        }

        twai_message_t message{};
        message.identifier = frame.id;
        message.data_length_code = frame.length;
        message.rtr = frame.rtr;
        message.extd = frame.extended;
        memcpy(message.data, frame.data.bytes, CAN_FRAME_DATA_SIZE);

        switch (twai_transmit(&message, pdMS_TO_TICKS(CAN_SEND_MS_TO_TICKS))) {
            case ESP_OK:
                log_d("Message sent successfully");
                return true;
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
        }
        return false;
    }

    bool Can::receive(CanFrame &frame) {
        return callback.read(&frame);
    }
}

