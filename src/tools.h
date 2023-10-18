#ifndef ESP32_C3_CANBUS_ARDUINO_TOOLS_H
#define ESP32_C3_CANBUS_ARDUINO_TOOLS_H

#include <Arduino.h>

class Tools {
public:
    /**
     * Конвертировать байты в строку HEX
     * @param bytes      Массив байт
     * @param size       Размер массива
     * @param upper_case Строка HEX в верхнем регистре
     * @return Строка HEX
     */
    static String bytes2hex(uint8_t bytes[], size_t size, bool upper_case = true);

    /**
     * Конвертировать строку HEX в байты
     * @param hex   Строка HEX
     * @param bytes Массив байт
     * @param size  Размер массива
     * @return Результат выполнения
     */
    static bool hex2bytes(String &hex, uint8_t bytes[], size_t size);

    /**
     * Сравнить два массива данных
     * @param buf1 Буфер данных 1
     * @param buf2 Буфер данных 2
     * @param size Размер буфера
     * @return Результат сравнения
     */
    static bool compare(const uint8_t *buf1, const uint8_t *buf2, size_t size);

    /**
     * Читать форматированное время
     * @param buffer Буфер (16 символов)
     * @param time   Время, мс
     * @param day	 Показать дни
     * @param hour	 Показать часы
     * @param minute Показать минуты
     * @param second Показать секунды
     */
    static void get_time(char buffer[16], unsigned long time, bool day = false, bool hour = true, bool minute = true, bool second = true);
};

#endif //ESP32_C3_CANBUS_ARDUINO_TOOLS_H
