#ifndef HARDWARE_CAN_FRAME_H
#define HARDWARE_CAN_FRAME_H

#include <Arduino.h>

namespace canbus
{
    /**
     * @brief Константы CAN-фрейма
     */
    constexpr uint8_t CAN_FRAME_DATA_SIZE = 8; ///< Размер данных CAN-фрейма
    constexpr uint16_t CAN_FRAME_FREQ = 250;   ///< Частота отправки по умолчанию (мс)

    /**
     * @brief Класс для работы с отдельными битами
     */
    class BitRef
    {
    public:
        /**
         * @brief Оператор присваивания значения биту
         */
        BitRef& operator=(const bool x)
        {
            *mRef = (*mRef & ~(1 << mPos));
            if (x) *mRef = *mRef | (1 << mPos);
            return *this;
        }

        /**
         * @brief Оператор приведения к bool
         */
        explicit operator bool() const
        {
            return (*mRef & (1 << mPos)) != 0;
        }

        /**
         * @brief Конструктор
         * @param ref Указатель на байт
         * @param pos Позиция бита в байте (0-7)
         */
        BitRef(uint8_t* ref, const int pos) : mRef(ref), mPos(pos)
        {
        }

    private:
        uint8_t* mRef; ///< Указатель на байт
        int mPos;      ///< Позиция бита
    };

    /**
     * @brief Объединение для работы с данными CAN-фрейма
     */
    union Bytes
    {
        uint64_t uint64;                    ///< 64-битное беззнаковое целое
        uint32_t uint32[2];                 ///< Массив 32-битных беззнаковых
        uint16_t uint16[4];                 ///< Массив 16-битных беззнаковых
        uint8_t uint8[8];                   ///< Массив 8-битных беззнаковых
        int64_t int64;                      ///< 64-битное знаковое целое
        int32_t int32[2];                   ///< Массив 32-битных знаковых
        int16_t int16[4];                   ///< Массив 16-битных знаковых
        int8_t int8[8];                     ///< Массив 8-битных знаковых
        uint8_t bytes[CAN_FRAME_DATA_SIZE]; ///< Массив байт

        /**
         * @brief Структура для работы с битами
         */
        struct
        {
            uint8_t field[8]; ///< Поле для битовых операций

            /**
             * @brief Оператор чтения бита
             */
            bool operator[](const int pos) const
            {
                if (pos < 0 || pos > 63) return false;
                return (field[pos / 8] >> (pos % 8)) & 1;
            }

            /**
             * @brief Оператор записи бита
             */
            BitRef operator[](const int pos)
            {
                if (pos < 0 || pos > 63) return {&field[0], 0};
                return {&field[pos / 8], pos % 8};
            }
        } bit;
    };

    /**
     * @brief Класс CAN-фрейма
     */
    class CanFrame
    {
    public:
        uint32_t id = 0;                     ///< 11- или 29-битный идентификатор
        Bytes data;                          ///< Данные фрейма
        uint8_t length = 0;                  ///< Длина данных (0-8)
        uint32_t extended = 0;               ///< Флаг расширенного формата (29 бит)
        uint32_t rtr = 0;                    ///< Флаг удаленного запроса
        int8_t filterIndex = -1;             ///< Индекс фильтра
        uint16_t frequency = CAN_FRAME_FREQ; ///< Частота отправки (мс)
        unsigned long nextSendTime = 0;      ///< Время следующей отправки (мс)

        /**
         * @brief Конструктор
         */
        CanFrame();

        /**
         * @brief Очистка фрейма
         */
        void clear();

        /**
         * @brief Проверка наличия данных
         * @return true если фрейм содержит данные
         */
        [[nodiscard]] bool hasData() const;

        /**
         * @brief Получение 16-битного значения
         * @param index Начальный индекс в массиве данных
         * @return 16-битное значение
         */
        [[nodiscard]] uint16_t getWord(int index) const;

        /**
         * @brief Сравнение фреймов
         * @param frame Фрейм для сравнения
         * @return true если фреймы идентичны
         */
        [[nodiscard]] bool compare(const CanFrame& frame) const;

        /**
         * @brief Получение байтов по битовым индексам
         * @param indexes Массив индексов битов
         * @param size Размер массива индексов
         * @return Объединение с полученными данными
         */
        Bytes getBytes(const int indexes[], size_t size);
    };
} // namespace hardware

#endif // HARDWARE_CAN_FRAME_H
