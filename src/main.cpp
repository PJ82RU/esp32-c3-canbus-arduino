#include <Arduino.h>
#include "can.h"

using namespace hardware;

Can can(GPIO_NUM_5, GPIO_NUM_4);

size_t on_receive(void *p_value, void *p_parameters) {
    Serial.println("on_receive");
    can_value_t *val = (can_value_t *) p_value;
    Serial.printf("Receive: 0x%03x 0x%02x\n", val->frame.id, val->frame.data.bytes);
    return 0;
}

size_t on_receive_200(void *p_value, void *p_parameters) {
    Serial.println("on_receive_200");
    can_value_t *val = (can_value_t *) p_value;
    Serial.printf("Receive: 0x%03x 0x%02x\n", val->frame.id, val->frame.data.bytes);
    return 0;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    can.callback.init(2);
    can.set_filter(0x420, 0xfff, false, can.callback.set(on_receive, nullptr, true));
    can.set_filter(0x200, 0xf00, false, can.callback.set(on_receive_200, nullptr, true));
    can.filter_enabled = true;
    if (can.begin(can_speed_t::CAN_SPEED_125KBIT)) {
        Serial.println("CANBUS initialized");
    } else {
        Serial.println("CANBUS not initialized");
    }
}

const uint8_t CAN290_DATA[8] = {0xC0, 0x20, 0x48, 0x65, 0x6c, 0x6c, 0x6f,
                                0x21};        // Первая часть строки информационного экрана
const uint8_t CAN291_DATA[8] = {0x85, 0x6f, 0x21, 0x20, 0x20, 0x20, 0x20,
                                0x20};        // Вторая часть строки информационного экрана
const uint8_t CAN28F_DATA[8] = {0x80, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00,
                                0x03};        // Параметры и завершающая часть команд 0х290 и 0х291

void loop() {
    CanFrame frame1;
    CanFrame frame2;
    CanFrame frame3;

    // отправляем данные в CAN-шину
    frame1.id = 0x290;
    frame1.length = 8;
    memcpy(frame1.data.bytes, CAN290_DATA, frame1.length);
    can.send(frame1);

    frame2.id = 0x291;
    frame2.length = 8;
    memcpy(frame2.data.bytes, CAN291_DATA, frame2.length);
    can.send(frame2);

    frame3.id = 0x28f;
    frame3.length = 8;
    memcpy(frame3.data.bytes, CAN28F_DATA, frame3.length);
    can.send(frame3);

    delay(250);
}