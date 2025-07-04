#ifndef HARDWARE_CAN_H
#define HARDWARE_CAN_H

#include "can_frame.h"
#include "esp32_c3_objects/thread.h"
#include "esp32_c3_objects/semaphore.h"
#include "esp32_c3_objects/callback.h"
#include "driver/twai.h"

namespace canbus
{
    /**
     * @brief Константы CAN-интерфейса
     */
    constexpr uint8_t CAN_NUM_FILTER = 32;            ///< Количество фильтров
    constexpr uint8_t CAN_RX_BUFFER_SIZE = 64;        ///< Размер буфера приема
    constexpr uint16_t CAN_RECEIVE_MS_TO_TICKS = 100; ///< Таймаут приема (мс)
    constexpr uint16_t CAN_SEND_MS_TO_TICKS = 4;      ///< Таймаут отправки (мс)

    /**
     * @brief Скорости CAN-шины
     */
    enum class CanSpeed
    {
        SPEED_25KBIT,  ///< 25 кбит/с
        SPEED_50KBIT,  ///< 50 кбит/с
        SPEED_100KBIT, ///< 100 кбит/с
        SPEED_125KBIT, ///< 125 кбит/с
        SPEED_250KBIT, ///< 250 кбит/с
        SPEED_500KBIT, ///< 500 кбит/с
        SPEED_800KBIT, ///< 800 кбит/с
        SPEED_1MBIT    ///< 1 Мбит/с
    };

    /**
     * @brief Структура фильтра CAN
     */
    struct CanFilter
    {
        bool configured = false;    ///< Флаг настройки фильтра
        bool extended = false;      ///< Флаг расширенного формата
        uint32_t id = 0;            ///< Идентификатор
        uint32_t mask = 0;          ///< Маска
        int16_t callbackIndex = -1; ///< Индекс callback-функции
    };

    /**
     * @brief Класс для работы с CAN-интерфейсом
     */
    class Can
    {
    public:
        /**
         * @brief Конструктор
         * @param txPin Пин TX
         * @param rxPin Пин RX
         */
        Can(gpio_num_t txPin, gpio_num_t rxPin);

        /**
         * @brief Деструктор
         */
        ~Can();

        // Запрет копирования
        Can(const Can&) = delete;
        Can& operator=(const Can&) = delete;

        /**
         * @brief Инициализация CAN-интерфейса
         * @param callback Функция обратного вызова
         * @return true если инициализация прошла успешно
         */
        bool begin(esp32_c3_objects::Callback* callback);

        /**
         * @brief Деинициализация CAN-интерфейса
         */
        void end();

        /**
         * @brief Получить текущее состояние CAN-интерфейса
         * @return Состояние интерфейса
         */
        twai_state_t getState() const;

        /**
         * @brief Ожидание перехода в рабочее состояние
         * @param timeout Таймаут ожидания (мс)
         * @return true если интерфейс в рабочем состоянии
         */
        bool waitRunning(unsigned long timeout = 0) const;

        /**
         * @brief Установка скорости CAN-шины
         * @param speed Скорость передачи
         */
        void setSpeed(CanSpeed speed);

        /**
         * @brief Установка фильтра
         * @param index Индекс фильтра
         * @param id Идентификатор
         * @param mask Маска
         * @param extended Флаг расширенного формата
         * @param callbackIndex Индекс callback-функции
         * @return Индекс установленного фильтра или -1 при ошибке
         */
        int setFilter(uint8_t index, uint32_t id, uint32_t mask, bool extended, int16_t callbackIndex = -1);

        /**
         * @brief Установка фильтра (автовыбор индекса)
         * @param id Идентификатор
         * @param mask Маска
         * @param extended Флаг расширенного формата
         * @param callbackIndex Индекс callback-функции
         * @return Индекс установленного фильтра или -1 при ошибке
         */
        int setFilter(uint32_t id, uint32_t mask, bool extended, int16_t callbackIndex = -1);

        /**
         * @brief Получить параметры фильтра
         * @param index Индекс фильтра
         * @return Структура с параметрами фильтра
         */
        CanFilter getFilter(int16_t index) const;

        /**
         * @brief Очистить все фильтры
         */
        void clearFilters();

        /**
         * @brief Отправить CAN-кадр
         * @param frame CAN-кадр для отправки
         * @return true если отправка прошла успешно
         */
        bool send(CanFrame& frame) const;

        /**
         * @brief Получить CAN-кадр из буфера
         * @param frame CAN-кадр для заполнения
         * @return true если кадр получен
         */
        bool receive(CanFrame& frame) const;

        /**
         * @brief Обработчик ответа
         * @param value Указатель на данные
         * @param params Указатель на параметры
         */
        static void onResponse(void* value, void* params);

    protected:
        /**
         * @brief Дружественная функция для задачи watchdog
         */
        friend void canWatchdogTask(void* params);

        /**
         * @brief Дружественная функция для задачи приема
         */
        friend void canReceiveTask(void* params);

        /**
         * @brief Обработчик watchdog-таймера
         */
        void handleWatchdog();

        /**
         * @brief Обработчик приема сообщений
         * @return true если интерфейс готов к работе
         */
        bool handleReceive() const;

    private:
        /**
         * @brief Установка и запуск драйвера TWAI
         * @return true если операция выполнена успешно
         */
        bool installAndStartDriver();

        /**
         * @brief Остановка и удаление драйвера TWAI
         */
        void stopAndUninstallDriver();

        /**
         * @brief Обработка входящего сообщения
         * @param message Входящее сообщение
         */
        void processFrame(const twai_message_t& message) const;

        /// Поток для мониторинга состояния
        esp32_c3_objects::Thread mWatchdogThread;
        /// Поток для приема сообщений
        esp32_c3_objects::Thread mReceiveThread;
        /// Callback-механизм
        esp32_c3_objects::Callback* mCallback = nullptr;
        /// Семафор для синхронизации
        esp32_c3_objects::Semaphore mSemaphore;

        /// Текущая скорость
        CanSpeed mSpeed = CanSpeed::SPEED_125KBIT;
        /// Флаг готовности драйвера
        bool mDriverReady = false;
        /// Информация о состоянии
        twai_status_info_t mStatusInfo = {};
        /// Массив фильтров
        CanFilter mFilters[CAN_NUM_FILTER];

        /// Конфигурация драйвера
        twai_general_config_t mDriverConfig = {};
        /// Конфигурация таймингов
        twai_timing_config_t mTimingConfig = {};
        /// Конфигурация фильтра
        twai_filter_config_t mFilterConfig = {};
    };
} // namespace hardware

#endif // HARDWARE_CAN_H
