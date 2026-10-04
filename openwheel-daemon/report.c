/* SPDX-License-Identifier: MIT */
#include "report.h"
size_t wheel_process_report(const uint8_t *bytes, size_t length,
                            int *button_down, WheelEvent events[2]) {
    if (length != 4 || (bytes[0] != 0x01 && bytes[0] != 0x0a)) return 0;
    if (bytes[0] == 0x0a && bytes[1] > 1) return 0;
    int pressed = bytes[1] & 1;
    unsigned encoded = (unsigned)bytes[2] | ((unsigned)bytes[3] << 8);
    int delta = encoded >= 0x8000 ? (int)encoded - 0x10000 : (int)encoded;
    size_t count = 0;
    if (pressed && !*button_down) events[count++] = (WheelEvent){WHEEL_PRESS, 1};
    if (delta) events[count++] = (WheelEvent){WHEEL_ROTATE, delta};
    if (!pressed && *button_down) events[count++] = (WheelEvent){WHEEL_PRESS, 0};
    *button_down = pressed;
    return count;
}
