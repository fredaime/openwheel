/* SPDX-License-Identifier: MIT */
#ifndef OPENWHEEL_REPORT_H
#define OPENWHEEL_REPORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { WHEEL_MAX_EVENTS = 2 };

typedef enum {
    WHEEL_ROTATE,
    WHEEL_PRESS,
} WheelEventType;

typedef struct {
    WheelEventType type;
    int value;
} WheelEvent;

/* Initialize button_down to false when opening a device. A report produces at
 * most one button edge and one rotation. A press precedes rotation and a
 * release follows it so consumers can recognize a held rotation. */
size_t wheel_process_report(const uint8_t *bytes, size_t length,
                            bool *button_down, WheelEvent events[WHEEL_MAX_EVENTS]);

#ifdef __cplusplus
}
#endif

#endif
