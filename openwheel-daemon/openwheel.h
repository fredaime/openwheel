//
// Created by faime on 27/09/24.
//

#ifndef OPENWHEEL_H
#define OPENWHEEL_H

#define HIDRAW_DEVICE "/dev/hidraw2"
#define DIAL_ROTATE_SIGNAL "Rotate"
#define DIAL_PRESS_SIGNAL  "Press"
#define BUTTON_DOWN  0x01
#define BUTTON_UP    0x00
#define ROTATE_PLUS  0x01
#define ROTATE_MINUS 0xff
#define DEVICE_NAME "ASUS2020"
#define SYSFS_HIDRAW_PATH "/sys/class/hidraw/"
#define BUFFER_SIZE 256

typedef struct {
    unsigned char report_id;
    unsigned char button;
    unsigned char rotation_hb;
    unsigned char rotation_lb;
} WheelPacket;

#endif //OPENWHEEL_H
