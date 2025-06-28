#include "can_frame.h"
#include <cstring>
#include <esp32-hal-log.h>

namespace hardware
{
    CanFrame::CanFrame()
    {
        clear();
        log_d("CAN frame created");
    }

    void CanFrame::clear()
    {
        id = 0;
        length = 0;
        extended = 0;
        rtr = 0;
        filterIndex = -1;
        memset(&data, 0, sizeof(data));
        log_d("CAN frame cleared");
    }

    bool CanFrame::hasData() const
    {
        return id > 0 && length > 0 && rtr == 0;
    }

    uint16_t CanFrame::getWord(const int index) const
    {
        if (index >= 0 && index + 1 < length)
        {
            const uint16_t result = word(data.bytes[index], data.bytes[index + 1]);
            log_d("Get word: %u", result);
            return result;
        }
        log_w("Get word: index out of range");
        return 0;
    }

    bool CanFrame::compare(const CanFrame& frame) const
    {
        if (frame.id != id || frame.length != length)
        {
            log_d("Compare: IDs or lengths differ");
            return false;
        }

        for (int i = 0; i < length; i++)
        {
            if (frame.data.bytes[i] != data.bytes[i])
            {
                log_d("Compare: data differs at position %d", i);
                return false;
            }
        }

        log_d("Compare: frames match");
        return true;
    }

    Bytes CanFrame::getBytes(const int indexes[], const size_t size)
    {
        Bytes result = {};
        for (size_t i = 0; i < size; i++)
        {
            const int idx = indexes[i];
            if (idx >= 0 && idx < 64)
            {
                result.bit[i] = data.bit[idx];
            }
        }

        log_d("Get bytes: %zu bits processed", size);
        return result;
    }
} // namespace hardware
