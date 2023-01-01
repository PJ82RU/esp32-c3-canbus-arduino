#ifndef ESP32_C3_CANBUS_ARDUINO_CAN_FRAME_H
#define ESP32_C3_CANBUS_ARDUINO_CAN_FRAME_H

#include "Arduino.h"
#include "driver/twai.h"

namespace hardware {

    // --- Заимствовано у Collin Kidder
    class bit_ref {
    public:
        bit_ref& operator = (bool x) {
            *_ref = (*_ref & ~(1 << _pos));
            if (x) *_ref = *_ref | (1 << _pos);
            return *this;
        }

        operator bool() const {
            if (*_ref & (1 << _pos)) return true;
            return false;
        }

        bit_ref(uint8_t *ref, int pos) {
            _ref = ref;
            _pos = pos;
        }

    private:
        uint8_t *_ref;
        int _pos;
    };
    // ---

    typedef union u_bytes {
        uint64_t uint64;
        uint32_t uint32[2];
        uint16_t uint16[4];
        uint8_t  uint8[8];
        int64_t int64;
        int32_t int32[2];
        int16_t int16[4];
        int8_t  int8[8];

        uint8_t bytes[8];
        // --- Заимствовано у Collin Kidder
        struct {
            uint8_t field[8];
            bool operator[](int pos) const {
                if (pos < 0 || pos > 63) return false;
                return (field[pos / 8] >> pos) & 1;
            }

            bit_ref operator[](int pos) {
                if (pos < 0 || pos > 63) return {(uint8_t*)&field[0], 0};
                auto *ptr = (uint8_t*)&field[0];
                return {ptr + (pos / 8), pos & 7};
            }
        } bit;
        // ---
    } bytes_t;

    class can_frame {
    public:
        uint32_t id{};          // 11 or 29 bit identifier
        bytes_t data{};         // Data bytes (not relevant in RTR frame)
        uint8_t length{};       // Data length code
        uint32_t extended{};    // Extended Frame Format (29bit ID)
        uint32_t self{};        // Transmit as a Self Reception Request. Unused for received.
        uint32_t rtr{};         // Message is a Remote Frame

        uint16_t freq = 0;
        unsigned long ms_next = 0;

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
        bool is() const;

        /**
         * Чтение значения word
         * @param index Позиция в массиве байт
         * @return Значение
         */
        uint16_t get_word(int index);

        /**
         * Сравнить кадр CAN
         * @param frame Кадр CAN
         * @return Результат выполнения
         */
        bool compare(can_frame& frame);

        /**
         * Чтение бит в произвольном порядке
         * @param index Массив индексов бит
         * @param size  Размер массива индексов бит
         * @return bytes_t
         */
        bytes_t get_bytes(const int index[], size_t size);
    };
}

#endif //ESP32_C3_CANBUS_ARDUINO_CAN_FRAME_H
