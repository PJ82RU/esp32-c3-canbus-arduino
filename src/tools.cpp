#include "tools.h"

String tools::bytes2hex(uint8_t bytes[], size_t size) {
    String res = "", buf;
    if (size != 0) {
        for (int i = 0; i < size; i++) {
            buf = String(bytes[i], HEX);
            res += (buf.length() != 2 ? "0" : "") + buf;
        }
        res.toUpperCase();
    }
    return res;
}

bool tools::hex2bytes(String& hex, uint8_t bytes[], size_t size) {
    size_t hexlen = hex.length();
    if (!bytes || size == 0 || hexlen == 0 || hexlen % 2 != 0) return false;

    size_t len = hexlen / 2;
    for (int i = 0, j = 0; j < size; i += 2, j++) {
        bytes[j] = j < len ? (hex[i] % 32 + 9) % 25 * 16 + (hex[i + 1] % 32 + 9) % 25 : 0;
    }
    return true;
}

bool tools::compare(const uint8_t* buf1, const uint8_t* buf2, size_t size) {
    for (int i = 0; i < size; ++i) {
        if (buf1[i] != buf2[i]) return false;
    }
    return true;
}

void tools::get_time(char buffer[16], unsigned long time, bool day) {
    uint8_t countDay, hour, minute, second;

    time /= 1000;
    second = time % 60;
    time /= 60;
    minute = time % 60;
    time /= 60;
    hour = time % 24;
    countDay = time / 24;

    if (day) sprintf(buffer, "%d.%02d:%02d:%02d", countDay, hour, minute, second);
    else sprintf(buffer, "%02d:%02d:%02d", hour, minute, second);
}
