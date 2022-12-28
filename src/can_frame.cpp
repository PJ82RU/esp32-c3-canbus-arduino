#include "can_frame.h"
#include "tools.h"

using namespace hardware;

can_frame::can_frame() {
    clear();
}

void can_frame::clear() {
    id = 0x00;
    length = 0;
    extended = 0;
    rtr = 0;
    self = 0;
    memset(&data, 0, sizeof(data));
}

int can_frame::set(twai_message_t message) {
    id = message.identifier;
    length = message.data_length_code;
    rtr = message.rtr;
    for (int i = 0; i < length; i++) {
        data.bytes[i] = message.data[i];
    }
    return length;
}

twai_message_t can_frame::get() {
    twai_message_t message;
    memset(&message, 0, sizeof(message));
    message.identifier = id;
    message.data_length_code = length;
    for (int i = 0; i < length; i++) {
        message.data[i] = data.bytes[i];
    }
    message.self = self;
    message.extd = extended;

    return message;
}

bool can_frame::is() const {
    return id > 0x00 && length > 0 && rtr == 0;
}

uint16_t can_frame::get_word(int index) {
    return index + 1 < length ? word(data.bytes[index], data.bytes[index + 1]) : 0;
}

bool can_frame::compare(can_frame& frame) {
    if (frame.id != id || frame.length != length) return false;
    for (int i = 0; i < length; i++) {
        if (frame.data.bytes[i] != data.bytes[i]) return false;
    }
    return true;
}

bytes_t can_frame::get_bytes(int index[], size_t size) {
    bytes_t result;
    for (int i = 0; i < size; i++) {
        result.bit[i] = data.bit[index[i]];
    }
    return result;
}

String can_frame::to_string() {
    uint8_t ids[2] = { (uint8_t) (id >> 8), (uint8_t) (id & 0xFF) };
    return "0x" + tools::bytes2hex(ids, 2) + " 0x" + tools::bytes2hex(data.bytes, length);
}
