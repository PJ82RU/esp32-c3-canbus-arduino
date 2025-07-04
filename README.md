# ESP32-C3 CANBus Library

![Лицензия](https://img.shields.io/badge/license-Unlicense-blue.svg)
![PlatformIO](https://img.shields.io/badge/platform-ESP32--C3-green.svg)
![Version](https://img.shields.io/badge/version-1.1.0-orange)

Библиотека для работы с CAN-шиной на ESP32-C3 через TWAI-контроллер.

## Особенности

- Поддержка стандартных (11-bit) и расширенных (29-bit) идентификаторов
- Гибкая система фильтрации сообщений (32 фильтра)
- Callback-механизм для обработки входящих сообщений
- Поддержка всех стандартных скоростей CAN (25 кбит/с - 1 Мбит/с)
- Потокобезопасная реализация
- Интеграция с FreeRTOS

## Установка

### PlatformIO

Добавьте в platformio.ini:

```ini
lib_deps =
    https://github.com/PJ82RU/esp32-c3-canbus-arduino.git
```

### Arduino IDE

1. Скачайте ZIP из GitHub
2. Скетч → Подключить библиотеку → Добавить .ZIP библиотеку

## Быстрый старт

```cpp
#include "hardware/can.h"

hardware::Can can(GPIO_NUM_5, GPIO_NUM_6);

void setup() {
can.begin(nullptr); // Инициализация без callback
can.setSpeed(hardware::CanSpeed::SPEED_250KBIT);
}

void loop() {
hardware::CanFrame frame;
frame.id = 0x123;
frame.data.uint8[0] = 0xAA;
frame.length = 1;

    can.send(frame);
    delay(1000);
}
```

## Примеры

1. `basic` - Базовая отправка/прием сообщений
2. `advanced` - Использование фильтров и callback-ов

## Документация

### Класс `CanFrame`

- `id` - Идентификатор сообщения (11/29 бит)
- `data` - Полезная нагрузка (до 8 байт)
- `length` - Длина данных (0-8)
- `extended` - Флаг расширенного формата
- `rtr` - Флаг удаленного запроса

### Класс `Can`

- `begin()` - Инициализация CAN-контроллера
- `setSpeed()` - Установка скорости
- `setFilter()` - Настройка фильтров
- `send()` - Отправка сообщения
- `receive()` - Получение сообщения

## Лицензия

Библиотека распространяется как общественное достояние (Unlicense).