#include "can_frame.h"
#include "tools.h"

using namespace hardware;

can_frame::can_frame() {
    ESP_LOGI(TAG, "Frame created");
    clear();
}

void can_frame::clear() {
    id = 0x00;
    length = 0;
    extended = 0;
    rtr = 0;
    self = 0;
    memset(&data, 0, sizeof(data));
    ESP_LOGI(TAG, "Frame cleared");
}

int can_frame::set(twai_message_t message) {
    ESP_LOGI(TAG, "Set message: id: %04x, bytes: %d, data: %s", message.identifier, message.data_length_code, message.data);
    ESP_LOG_BUFFER_HEXDUMP(TAG, &message, length, ESP_LOG_DEBUG);

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

    ESP_LOGI(TAG, "Set message: id: %04x, bytes: %d, data: %s", message.identifier, message.data_length_code, message.data);
    ESP_LOG_BUFFER_HEXDUMP(TAG, &message, length, ESP_LOG_DEBUG);
    return message;
}

bool can_frame::is() const {
    return id > 0x00 && length > 0 && rtr == 0;
}

uint16_t can_frame::get_word(int index) {
    if (index >= 0 && index + 1 < length) {
        uint16_t result = word(data.bytes[index], data.bytes[index + 1]);
        ESP_LOGI(TAG, "Get word: %d", result);
        return result;
    }
    ESP_LOGW(TAG, "Get word: index is outside");
    return 0;
}

bool can_frame::compare(can_frame& frame) {
    bool result = frame.id == id && frame.length == length;
    if (result) {
        for (int i = 0; i < length; i++) {
            result = frame.data.bytes[i] == data.bytes[i];
            if (!result) break;
        }
    }
    ESP_LOGI(TAG, "Compare: %s", result ? "true" : "false");
    return result;
}

bytes_t can_frame::get_bytes(const int index[], size_t size) {
    bytes_t result;
    int idx;
    for (int i = 0; i < size; i++) {
        idx = index[i];
        result.bit[i] = idx >= 0 && idx < 64 && data.bit[idx];
    }
    ESP_LOGI(TAG, "Get bytes: %s", result.bytes);
    ESP_LOG_BUFFER_HEXDUMP(TAG, result.bytes, 8, ESP_LOG_DEBUG);
    return result;
}
