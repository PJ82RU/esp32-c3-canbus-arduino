#ifndef ESP32_C3_CANBUS_ARDUINO_CAN_H
#define ESP32_C3_CANBUS_ARDUINO_CAN_H

#include "can_frame.h"

namespace hardware {

    typedef enum e_can_speed {
        CAN_SPEED_25KBIT,
        CAN_SPEED_50KBIT,
        CAN_SPEED_100KBIT,
        CAN_SPEED_125KBIT,
        CAN_SPEED_250KBIT,
        CAN_SPEED_500KBIT,
        CAN_SPEED_800KBIT,
        CAN_SPEED_1MBIT,
        CAN_SPEED_MAX
    } e_can_speed_t;

    class can {
    public:
        /**
         * Инициализация
         * @param gpio_tx Контакт TX
         * @param gpio_rx Контакт RX
         * @param speed Скорость can-шины
         * @return Результат выполнения
         */
        bool begin(gpio_num_t gpio_tx, gpio_num_t gpio_rx, e_can_speed_t speed);

        /**
         * Отправить кадр данных
         * @param frame Кадр данных
         * @param timeout Время ожидания отправки данных, мс
         * @return Результат выполнения
         */
        bool send(can_frame& frame, int timeout = 1000);

        /**
         * Получить кадр данных
         * @param frame Кадр данных
         * @param timeout Время ожидания входящих данных, мс
         * @return Количество полученных байт
         */
        int receive(can_frame& frame, int timeout = 5);

    private:
        bool _init = false;

        /**
         * Установка/переустановка драйвера
         * @param gpio_tx Контакт TX
         * @param gpio_rx Контакт RX
         * @param mode Режим работы контроллера TWAI
         * @param speed Скорость can-шины
         * @return Результат выполнения
         */
        bool _driver_install(gpio_num_t gpio_tx, gpio_num_t gpio_rx, twai_mode_t mode, e_can_speed_t speed);

        /** Удаление драйвера */
        void _driver_uninstall();
    };
}

extern hardware::can cand;

#endif //ESP32_C3_CANBUS_ARDUINO_CAN_H
