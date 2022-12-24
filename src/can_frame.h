#ifndef ESP32_C3_CANBUS_ARDUINO_CAN_FRAME_H
#define ESP32_C3_CANBUS_ARDUINO_CAN_FRAME_H

#include "Arduino.h"
#include "driver/twai.h"

namespace hardware {

#pragma pack(push,1)
    typedef union bits_u {
        struct {
            bool bit1 : 1;
            bool bit2 : 1;
            bool bit3 : 1;
            bool bit4 : 1;
            bool bit5 : 1;
            bool bit6 : 1;
            bool bit7 : 1;
            bool bit8 : 1;
        } value;
        uint8_t byte;
    } bits_t;

    typedef union bytes_u {
        uint64_t uint64;
        uint32_t uint32[2];
        uint16_t uint16[4];
        uint8_t  uint8[8];
        int64_t int64;
        int32_t int32[2];
        int16_t int16[4];
        int8_t  int8[8];

        uint8_t bytes[8];
        bits_t bits[8];
    } bytes_t;
#pragma pack(pop)

    class can_frame {
    public:
        uint32_t id{};          // 11 or 29 bit identifier
        bytes_t data{};         // Data bytes (not relevant in RTR frame)
        uint8_t length{};       // Data length code
        uint32_t extended{};    // Extended Frame Format (29bit ID)
        uint32_t self{};        // Transmit as a Self Reception Request. Unused for received.
        uint32_t rtr{};         // Message is a Remote Frame

        can_frame();

        /** Очистить значения */
        void clear();

        /**
         * Запись данных
         * @param message Сообщение TWAI
         * @return Количество записанных байт
         */
        int set(twai_message_t message);

        /**
         * Чтение данных
         * @return Сообщение TWAI
         */
        twai_message_t get();

        /** Наличие данных */
        bool is();

        /**
         * Чтение значения word
         * @param index Индекс значения word
         * @return Значение
         */
        uint16_t getWord(int index);
    };
}

#endif //ESP32_C3_CANBUS_ARDUINO_CAN_FRAME_H
