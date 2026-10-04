/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "report.h"
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); exit(1); \
} } while (0)
int main(void) {
    WheelEvent events[2];
    int down = 0;
    const int deltas[] = {-32768, -3600, -257, -256, -2, -1, 0, 1, 2, 256, 257, 3600, 32767};
    const uint8_t reports[] = {0x01, 0x0a};
    for (size_t r = 0; r < sizeof(reports); r++) {
        for (size_t i = 0; i < sizeof(deltas) / sizeof(deltas[0]); i++) {
            unsigned encoded = (unsigned)(deltas[i] & 0xffff);
            uint8_t packet[] = {reports[r], 0, encoded & 0xff, encoded >> 8};
            size_t count = wheel_process_report(packet, sizeof(packet), &down, events);
            CHECK(count == (size_t)(deltas[i] != 0));
            if (count) CHECK(events[0].type == WHEEL_ROTATE && events[0].value == deltas[i]);
        }
    }
    uint8_t packet[] = {0x0a, 1, 1, 0};
    CHECK(wheel_process_report(packet, 4, &down, events) == 2);
    CHECK(events[0].type == WHEEL_PRESS && events[0].value == 1);
    CHECK(events[1].type == WHEEL_ROTATE && events[1].value == 1);
    CHECK(down == 1);
    CHECK(wheel_process_report(packet, 4, &down, events) == 1);
    CHECK(events[0].type == WHEEL_ROTATE);
    packet[1] = 0; packet[2] = 0xff; packet[3] = 0xff;
    CHECK(wheel_process_report(packet, 4, &down, events) == 2);
    CHECK(events[0].type == WHEEL_ROTATE && events[0].value == -1);
    CHECK(events[1].type == WHEEL_PRESS && events[1].value == 0);
    CHECK(down == 0);
    packet[2] = 0; packet[3] = 0;
    CHECK(wheel_process_report(packet, 4, &down, events) == 0);
    packet[1] = 3;
    CHECK(wheel_process_report(packet, 4, &down, events) == 0 && down == 0);
    packet[0] = 1; packet[1] = 0x81;
    CHECK(wheel_process_report(packet, 4, &down, events) == 1 && down == 1);
    packet[0] = 0x02; packet[1] = 0;
    CHECK(wheel_process_report(packet, 4, &down, events) == 0 && down == 1);
    CHECK(wheel_process_report(packet, 3, &down, events) == 0 && down == 1);
    puts("HID report decoding and held-rotation ordering: passed");
    return 0;
}
