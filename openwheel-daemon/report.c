/* SPDX-License-Identifier: MIT */
#include "report.h"

enum {
    REPORT_SIZE = 4,
    REPORT_ID_LEGACY = 0x01,
    REPORT_ID_ASUS2020 = 0x0a,
    BUTTON_BIT = 0x01,
};

size_t wheel_process_report(const uint8_t *bytes, size_t length,
                            bool *button_down, WheelEvent events[WHEEL_MAX_EVENTS])
{
    if (length != REPORT_SIZE ||
        (bytes[0] != REPORT_ID_LEGACY && bytes[0] != REPORT_ID_ASUS2020)) {
        return 0;
    }
    if (bytes[0] == REPORT_ID_ASUS2020 && bytes[1] > BUTTON_BIT) {
        return 0;
    }

    const bool pressed = (bytes[1] & BUTTON_BIT) != 0;
    const int encoded = bytes[2] | (bytes[3] << 8);
    const int delta = encoded >= 0x8000 ? encoded - 0x10000 : encoded;
    size_t count = 0;
    if (pressed && !*button_down) {
        events[count++] = (WheelEvent){WHEEL_PRESS, 1};
    }
    if (delta != 0) {
        events[count++] = (WheelEvent){WHEEL_ROTATE, delta};
    }
    if (!pressed && *button_down) {
        events[count++] = (WheelEvent){WHEEL_PRESS, 0};
    }
    *button_down = pressed;
    return count;
}
