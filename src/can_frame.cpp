#include "can_frame.h"
#include "tools.h"
#include "esp32-hal-log.h"

using namespace hardware;

can_frame::can_frame() {
    log_d("Frame created");
    clear();
}

void can_frame::clear() {
    id = 0x00;
    length = 0;
    extended = 0;
    rtr = 0;
    self = 0;
    memset(&data, 0, sizeof(data));
    log_d("Frame cleared");
}

int can_frame::set(twai_message_t message) {
    log_d("Set message: id: %04x, bytes: %d, data: %s", message.identifier, message.data_length_code, message.data);

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

    log_d("Set message: id: %04x, bytes: %d, data: %s", message.identifier, message.data_length_code, message.data);
    return message;
}

bool can_frame::is() const {
    return id > 0x00 && length > 0 && rtr == 0;
}

uint16_t can_frame::get_word(int index) {
    if (index >= 0 && index + 1 < length) {
        uint16_t result = word(data.bytes[index], data.bytes[index + 1]);
        log_d("Get word: %d", result);
        return result;
    }
    log_w("Get word: index is outside");
    return 0;
}

bool can_frame::compare(can_frame &frame) {
    bool result = frame.id == id && frame.length == length;
    if (result) {
        for (int i = 0; i < length; i++) {
            result = frame.data.bytes[i] == data.bytes[i];
            if (!result) break;
        }
    }
    log_d("Compare: %s", result ? "true" : "false");
    return result;
}

bytes_t can_frame::get_bytes(const int index[], size_t size) {
    bytes_t result;
    int idx;
    for (int i = 0; i < size; i++) {
        idx = index[i];
        result.bit[i] = idx >= 0 && idx < 64 && data.bit[idx];
    }
    log_d("Get bytes: %s", result.bytes);
    return result;
}
