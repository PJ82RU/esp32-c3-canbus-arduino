#ifndef ESP32_C3_CANBUS_ARDUINO_CAN_FRAME_H
#define ESP32_C3_CANBUS_ARDUINO_CAN_FRAME_H

#include <Arduino.h>

#define CAN_FRAME_DATA_SIZE     8
#define CAN_FRAME_FREQ          250

namespace hardware
{
    // --- Заимствовано у Collin Kidder
    class bit_ref
    {
    public:
        bit_ref& operator=(const bool x)
        {
            *_ref = (*_ref & ~(1 << _pos));
            if (x) *_ref = *_ref | (1 << _pos);
            return *this;
        }

        operator bool() const
        {
            if (*_ref & (1 << _pos)) return true;
            return false;
        }

        bit_ref(uint8_t* ref, const int pos)
        {
            _ref = ref;
            _pos = pos;
        }

    private:
        uint8_t* _ref;
        int _pos;
    };

    // ---

    typedef union u_bytes
    {
        uint64_t uint64;
        uint32_t uint32[2];
        uint16_t uint16[4];
        uint8_t uint8[8];
        int64_t int64;
        int32_t int32[2];
        int16_t int16[4];
        int8_t int8[8];

        uint8_t bytes[CAN_FRAME_DATA_SIZE];

        // --- Заимствовано у Collin Kidder
        struct
        {
            uint8_t field[8];

            bool operator[](const int pos) const
            {
                if (pos < 0 || pos > 63) return false;
                return (field[pos / 8] >> pos) & 1;
            }

            bit_ref operator[](const int pos)
            {
                if (pos < 0 || pos > 63) return {(uint8_t*)&field[0], 0};
                auto* ptr = (uint8_t*)&field[0];
                return {ptr + (pos / 8), pos & 7};
            }
        } bit;

        // ---
    } bytes_t;

    class CanFrame
    {
    public:
        uint32_t id{}; // 11- или 29-разрядный идентификатор
        bytes_t data{}; // Байты данных (не относящиеся к кадру RTR)
        uint8_t length{}; // Размер данных
        uint32_t extended{}; // Расширенный формат кадра (29-битный идентификатор)
        uint32_t rtr{}; // Сообщение - это удаленный кадр
        int8_t f_idx{}; // Индекс фильтра
        uint16_t freq = CAN_FRAME_FREQ; // значение частоты отправки данных, мс
        unsigned long ms_next = 0; // время следующей отправки данных, мс

        CanFrame();

        /** Очистить значения */
        void clear();

        /** Наличие данных */
        bool is() const;

        /**
         * Чтение значения word
         * @param index Позиция в массиве байт
         * @return Значение
         */
        uint16_t get_word(int index) const;

        /**
         * Сравнить кадр CAN
         * @param frame Кадр CAN
         * @return Результат выполнения
         */
        bool compare(const CanFrame& frame) const;

        /**
         * Чтение бит в произвольном порядке
         * @param index Массив индексов бит
         * @param size Размер массива индексов бит
         * @return bytes_t
         */
        bytes_t get_bytes(const int index[], size_t size);
    };
}

#endif //ESP32_C3_CANBUS_ARDUINO_CAN_FRAME_H
