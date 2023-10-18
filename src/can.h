#ifndef ESP32_C3_CANBUS_ARDUINO_CAN_H
#define ESP32_C3_CANBUS_ARDUINO_CAN_H

#include "can_frame.h"
#include "types.h"

namespace hardware {
    class Can {
    public:
        /**
         * Инициализация
         * @param gpio_tx Контакт TX
         * @param gpio_rx Контакт RX
         * @param speed Скорость can-шины
         * @return Результат выполнения
         */
        bool begin(gpio_num_t gpio_tx, gpio_num_t gpio_rx, can_speed_t speed);

        /**
         * Отправить кадр данных
         * @param frame Кадр данных
         * @param timeout Время ожидания отправки данных, мс
         * @return Код выполнения:
         *      0 - сообщение отправлено;
         *      1 - сообщение ожидает отправления;
         *      -1 - ошибка отправки;
         *      -2 - объект не инициализирован или данные отсутствуют;
         */
        int send(CanFrame &frame, int timeout = 1000);

        /**
         * Получить кадр данных
         * @param frame Кадр данных
         * @param timeout Время ожидания входящих данных, мс
         * @return Количество полученных байт
         */
        int receive(CanFrame &frame, int timeout = 5);

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
        bool _driver_install(gpio_num_t gpio_tx, gpio_num_t gpio_rx, twai_mode_t mode, can_speed_t speed);

        /** Удаление драйвера */
        void _driver_uninstall();
    };
}

#endif //ESP32_C3_CANBUS_ARDUINO_CAN_H
