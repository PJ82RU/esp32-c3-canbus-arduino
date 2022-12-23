#include "can_frame.h"

using namespace Hardware;

can_frame::can_frame()
{
    clear();
}

void can_frame::clear()
{
    id = 0x00;
    length = 0;
    extended = 0;
    rtr = 0;
    self = 0;
    memset(&data, 0, sizeof(data));
}

int can_frame::set(twai_message_t message)
{
    id = message.identifier;
    length = message.data_length_code;
    rtr = message.rtr;
    for (int i = 0; i < length; i++) {
        data.bytes[i] = message.data[i];
    }
    return length;
}

twai_message_t can_frame::get()
{
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

bool can_frame::is()
{
    return id > 0x00 && length > 0 && rtr == 0;
}