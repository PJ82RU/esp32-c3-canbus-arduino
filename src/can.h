#ifndef ESP32_C3_CANBUS_ARDUINO_CAN_H
#define ESP32_C3_CANBUS_ARDUINO_CAN_H

#include "can_frame.h"

namespace Hardware {

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
        static bool begin(gpio_num_t gpio_tx, gpio_num_t gpio_rx, e_can_speed_t speed);

        /**
         * Отправить кадр данных
         * @param frame Кадр данных
         * @return Результат выполнения
         */
        static bool send(can_frame& frame);

        /**
         * Получить кадр данных
         * @param frame Кадр данных
         * @return Количество полученных байт
         */
        static int receive(can_frame& frame);

    private:
        static volatile bool _init;

        /**
         * Установка/переустановка драйвера
         * @param gpio_tx Контакт TX
         * @param gpio_rx Контакт RX
         * @param mode Режим работы контроллера TWAI
         * @param speed Скорость can-шины
         * @return Результат выполнения
         */
        static bool driver_install(gpio_num_t gpio_tx, gpio_num_t gpio_rx, twai_mode_t mode, e_can_speed_t speed);

        /** Удаление драйвера */
        static void driver_uninstall();
    };
}


#endif //ESP32_C3_CANBUS_ARDUINO_CAN_H
