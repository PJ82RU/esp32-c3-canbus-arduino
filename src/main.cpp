#include <Arduino.h>
#include "can.h"

using namespace hardware;

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (can.begin(GPIO_NUM_5, GPIO_NUM_4, e_can_speed::CAN_SPEED_125KBIT)) {
        Serial.println("CANBUS initialized");
    } else {
        Serial.println("CANBUS not initialized");
    }
}

const uint8_t CAN290_DATA[8] = { 0xC0, 0x20, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x21 };		// Первая часть строки информационного экрана
const uint8_t CAN291_DATA[8] = { 0x85, 0x6f, 0x21, 0x20, 0x20, 0x20, 0x20, 0x20 };		// Вторая часть строки информационного экрана
const uint8_t CAN28F_DATA[8] = { 0x80, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x03 };		// Параметры и завершающая часть команд 0х290 и 0х291

void loop() {
    can_frame frame;

    if (can.receive(frame) > 0)
    {
        // получаем данные из CAN-шины
    }

    // отправляем данные в CAN-шину
    frame.id = 0x290;
    frame.length = 8;
    memcpy(frame.data.bytes, CAN290_DATA, frame.length);
    can.send(frame);

    frame.id = 0x291;
    frame.length = 8;
    memcpy(frame.data.bytes, CAN291_DATA, frame.length);
    can.send(frame);

    frame.id = 0x28f;
    frame.length = 8;
    memcpy(frame.data.bytes, CAN28F_DATA, frame.length);
    can.send(frame);

    delay(250);
}