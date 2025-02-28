#include "can_frame.h"
#include "esp32-hal-log.h"

namespace hardware
{
    CanFrame::CanFrame()
    {
        clear();
        log_d("Frame created");
    }

    void CanFrame::clear()
    {
        id = 0x00;
        length = 0;
        extended = 0;
        rtr = 0;
        f_idx = -1;
        memset(&data, 0, sizeof(data));
        log_d("Frame cleared");
    }

    bool CanFrame::is() const
    {
        return id > 0x00 && length > 0 && rtr == 0;
    }

    uint16_t CanFrame::get_word(const int index) const
    {
        if (index >= 0 && index + 1 < length)
        {
            const uint16_t result = word(data.bytes[index], data.bytes[index + 1]);
            log_d("Get word: %d", result);
            return result;
        }
        log_w("Get word: index is outside");
        return 0;
    }

    bool CanFrame::compare(const CanFrame& frame) const
    {
        bool result = frame.id == id && frame.length == length;
        if (result)
        {
            for (int i = 0; i < length; i++)
            {
                result = frame.data.bytes[i] == data.bytes[i];
                if (!result) break;
            }
        }
        log_d("Compare: %s", result ? "true" : "false");
        return result;
    }

    bytes_t CanFrame::get_bytes(const int index[], const size_t size)
    {
        bytes_t result;
        for (uint8_t& i : result.bytes) i = 0;
        for (int i = 0; i < size; i++)
        {
            const int idx = index[i];
            result.bit[i] = idx >= 0 && idx < 64 && data.bit[idx];
        }
        log_d("Get bytes: %s", result.bytes);
        return result;
    }
}
