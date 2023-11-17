# esp32-c3-canbus-arduino
ESP32-C3 CANBUS Arduino



### Создание объекта

```c++
Can can(GPIO_NUM_5, GPIO_NUM_4);
```



### Пример инициализации

```c++
can.callback.init(2);
can.set_filter(0x420, 0xfff, false, can.callback.set(on_receive, nullptr, true));
can.set_filter(0x200, 0xf00, false, can.callback.set(on_receive_200, nullptr, true));
if (can.begin(can_speed_t::CAN_SPEED_125KBIT)) {
    Serial.println("CANBUS initialized");
} else {
    Serial.println("CANBUS not initialized");
}
```



### Отправка данных

```c++
frame.id = 0x290;
frame.length = 8;
memcpy(frame.data.bytes, CAN290_DATA, frame.length);
can.send(frame);
```
