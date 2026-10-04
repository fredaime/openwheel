/* SPDX-License-Identifier: MIT */
#ifndef OPENWHEEL_REPORT_H
#define OPENWHEEL_REPORT_H
#include <stddef.h>
#include <stdint.h>
typedef enum { WHEEL_ROTATE, WHEEL_PRESS } WheelEventType;
typedef struct { WheelEventType type; int value; } WheelEvent;
/* Press-down precedes rotation, and release follows rotation, so consumers can
 * distinguish a held rotation without adding another D-Bus signal. */
size_t wheel_process_report(const uint8_t *bytes, size_t length,
                            int *button_down, WheelEvent events[2]);
#endif
