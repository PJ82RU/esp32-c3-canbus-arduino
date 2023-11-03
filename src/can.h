#ifndef ESP32_C3_CANBUS_ARDUINO_CAN_H
#define ESP32_C3_CANBUS_ARDUINO_CAN_H

#include "can_frame.h"
#include "driver/twai.h"

#define CAN_NUM_FILTER              32
#define CAN_RX_BUFFER_SIZE          64
#define CAN_TX_BUFFER_SIZE          16
#define CAN_RECEIVE_MS_TO_TICKS     100
#define CAN_SEND_MS_TO_TICKS        4

namespace hardware {
    typedef enum can_speed_t {
        CAN_SPEED_25KBIT,
        CAN_SPEED_50KBIT,
        CAN_SPEED_100KBIT,
        CAN_SPEED_125KBIT,
        CAN_SPEED_250KBIT,
        CAN_SPEED_500KBIT,
        CAN_SPEED_800KBIT,
        CAN_SPEED_1MBIT
    } can_speed_t;

    typedef void (*event_can_receive_t)(CanFrame &);

    typedef struct can_filter_t {
        bool configured;
        bool extended;
        uint32_t id;
        uint32_t mask;
        event_can_receive_t cb_receive;
    } can_filter_t;


    class Can {
    public:
        /** Функция обратного вызова входящего кадра */
        event_can_receive_t cb_receive = nullptr;

        /** Вкл. фильтр */
        bool filter_enabled = false;
        /** Выкл. обратный вызов */
        bool callback_disabled = false;

        /** Canbus
         * @param gpio_tx Контакт TX
         * @param gpio_rx Контакт RX
         */
        Can(gpio_num_t gpio_tx, gpio_num_t gpio_rx);

        ~Can();

        /**
         * Сторожевой пес. Следит за состоянием can-шины
         * @param pv_parameters
         */
        friend void task_watchdog(void *pv_parameters);

        /**
         * Следим за входящими сообщениями.
         * Все входящие отправляем в обработку (фильтр), далее в очередь и обратный вызов входящего кадра
         * @param pv_parameters
         */
        friend void task_receive(void *pv_parameters);

        /**
         * Обратный вызов входящего кадра
         * @param pv_parameters
         */
        friend void task_callback(void *pv_parameters);

        /**
         * Запуск сервисов can-шины
         * @param speed Скорость can-шины
         * @return Результат выполнения
         */
        bool begin(can_speed_t speed = can_speed_t::CAN_SPEED_125KBIT);

        /** Остановить сервисы can-шины */
        void end();

        /**
         * Изменить скорость can-шины
         * @param speed Значение
         * @return Результат выполнения
         */
        bool set_timing(can_speed_t speed);

        /**
         * Записать фильтр
         * @param index     Индекс фильтра
         * @param id        ID кадра
         * @param mask      Маска
         * @param extended  Расширенный формат кадра (29-битный идентификатор)
         * @param cb        Функция обратного вызова входящего кадра
         * @return Индекс фильтра
         */
        int set_filter(uint8_t index, uint32_t id, uint32_t mask, bool extended, event_can_receive_t cb = nullptr);

        /**
         * Записать фильтр
         * @param id        ID кадра
         * @param mask      Маска
         * @param extended  Расширенный формат кадра (29-битный идентификатор)
         * @param cb        Функция обратного вызова входящего кадра
         * @return Индекс фильтра
         */
        int set_filter(uint32_t id, uint32_t mask, bool extended, event_can_receive_t cb = nullptr);

        /**
         * Прочитать фильтр
         * @param index Индекс фильтра
         * @return Значение
         */
        can_filter_t get_filter(int8_t index);

        /** Очистить фильтры */
        void clear_filter();

        /**
         * Отправить кадр данных
         * @param frame Кадр данных
         * @return Результат выполнения
         */
        bool send(CanFrame &frame);

        /**
         * Чтение кадра данных из буфера
         * @param frame Кадр данных
         * @return Результат выполнения
         */
        bool receive(CanFrame &frame);

    protected:
        /** Драйвер TWAI установлен и запущен */
        bool twai_ready = false;
        /** Статус TWAI */
        twai_status_info_t twai_status_info{};
        /** Фильтры */
        can_filter_t filters[CAN_NUM_FILTER]{};

        /**
         * Получить функцию обратного вызова
         * @param filter_index Индекс фильтра
         * @return Значение
         */
        event_can_receive_t get_callback(int8_t filter_index);

        /**
         * Обработка входящего сообщения
         * @param twai_message Сообщение
         * @return Результат выполнения
         */
        bool frame_processing(twai_message_t &twai_message);

    private:
        TaskHandle_t task_cb{};
        TaskHandle_t task_rx{};
        TaskHandle_t task_wd{};

        /** Конфигурация TWAI */
        twai_general_config_t twai_general_config{};
        /** Тайминги TWAI */
        twai_timing_config_t twai_timing_config{};
        /** Фильтр TWAI */
        twai_filter_config_t twai_filter_config{};

        /** Установить и запустить драйвер TWAI */
        bool twai_install_and_start();

        /** Остановить и удалить драйвер TWAI */
        void twai_stop_and_uninstall();
    };
}

#endif //ESP32_C3_CANBUS_ARDUINO_CAN_H
