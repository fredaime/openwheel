/* SPDX-License-Identifier: MIT */
#include "report.h"

#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

static void test_signed_delta(uint8_t report_id)
{
    const int deltas[] = {-32768, -3600, -257, -256, -2, -1, 0, 1, 2, 256, 257, 3600, 32767};
    WheelEvent events[WHEEL_MAX_EVENTS];
    bool button_down = false;

    for (size_t i = 0; i < sizeof(deltas) / sizeof(deltas[0]); i++) {
        const uint16_t encoded = (uint16_t)deltas[i];
        const uint8_t report[] = {report_id, 0, encoded & 0xff, encoded >> 8};
        size_t count = wheel_process_report(report, sizeof(report), &button_down, events);
        CHECK(count == (size_t)(deltas[i] != 0));
        if (count != 0) {
            CHECK(events[0].type == WHEEL_ROTATE && events[0].value == deltas[i]);
        }
        CHECK(!button_down);
    }
}

static void test_button_edges(uint8_t report_id, uint8_t button_flags)
{
    const uint8_t press[] = {report_id, button_flags | 1, 0x01, 0x01};
    const uint8_t release[] = {report_id, button_flags, 0xff, 0xfe};
    const uint8_t idle[] = {report_id, button_flags, 0, 0};
    WheelEvent events[WHEEL_MAX_EVENTS];
    bool button_down = false;

    CHECK(wheel_process_report(press, sizeof(press), &button_down, events) == 2);
    CHECK(events[0].type == WHEEL_PRESS && events[0].value == 1);
    CHECK(events[1].type == WHEEL_ROTATE && events[1].value == 257);
    CHECK(button_down);
    CHECK(wheel_process_report(press, sizeof(press), &button_down, events) == 1);
    CHECK(events[0].type == WHEEL_ROTATE && events[0].value == 257);
    CHECK(wheel_process_report(release, sizeof(release), &button_down, events) == 2);
    CHECK(events[0].type == WHEEL_ROTATE && events[0].value == -257);
    CHECK(events[1].type == WHEEL_PRESS && events[1].value == 0);
    CHECK(!button_down);
    CHECK(wheel_process_report(idle, sizeof(idle), &button_down, events) == 0);
}

static void test_ignored_reports(void)
{
    const uint8_t other_id[] = {0x02, 0, 1, 0};
    const uint8_t other_asus2020[] = {0x0a, 3, 1, 0};
    const uint8_t wrong_length[] = {0x0a, 0, 1, 0, 0};
    WheelEvent events[WHEEL_MAX_EVENTS];
    bool button_down = true;

    CHECK(wheel_process_report(other_id, sizeof(other_id), &button_down, events) == 0);
    CHECK(button_down);
    CHECK(wheel_process_report(other_asus2020, sizeof(other_asus2020), &button_down, events) == 0);
    CHECK(button_down);
    CHECK(wheel_process_report(wrong_length, sizeof(wrong_length), &button_down, events) == 0);
    CHECK(button_down);
    CHECK(wheel_process_report(wrong_length, 3, &button_down, events) == 0);
    CHECK(button_down);
}

int main(void)
{
    test_signed_delta(0x01);
    test_signed_delta(0x0a);
    test_button_edges(0x01, 0x80);
    test_button_edges(0x0a, 0);
    test_ignored_reports();
    return EXIT_SUCCESS;
}
