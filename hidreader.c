#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dbus/dbus.h>

#define HIDRAW_DEVICE "/dev/hidraw2"
#define DIAL_ROTATE_SIGNAL "Rotate"
#define DIAL_PRESS_SIGNAL  "Press"

#define BUTTON_DOWN  0x01
#define BUTTON_UP    0x00
#define ROTATE_PLUS  0x01
#define ROTATE_MINUS 0xff

typedef struct {
    unsigned char report_id;
    unsigned char button;
    unsigned char rotation_hb;
    unsigned char rotation_lb;
} WheelPacket;

void send_dbus_signal(DBusConnection *connection, const char *signal_name, int value) {
    DBusMessage* msg;
    DBusMessageIter args;
    DBusError err;

    // Initialize error
    dbus_error_init(&err);

    // Create a new signal and add arguments to it
    msg = dbus_message_new_signal("/org/asus/dial",  // object name
                                  "org.asus.dial",   // interface name
                                  signal_name);      // signal name

    if (msg == NULL) {
        fprintf(stderr, "Error: Message NULL\n");
        exit(1);
    }

    // Append the integer value (rotation or press)
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &value)) {
        fprintf(stderr, "Error appending arguments\n");
        exit(1);
    }

    // Send the message and flush the connection
    if (!dbus_connection_send(connection, msg, NULL)) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(1);
    }

    dbus_connection_flush(connection);
    dbus_message_unref(msg);
}

int main() {
    int fd = open(HIDRAW_DEVICE, O_RDONLY);
    if (fd < 0) {
        perror("Unable to open HIDRAW device");
        return 1;
    }

    // Set up D-Bus connection
    DBusConnection* connection;
    DBusError err;

    dbus_error_init(&err);
    connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (connection == NULL) {
        return 1;
    }

    // Request the name 'org.asus.dial'
    int ret = dbus_bus_request_name(connection, "org.asus.dial", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        fprintf(stderr, "Failed to obtain D-Bus name 'org.asus.dial'\n");
        return 1;
    }

    WheelPacket pkt;
    int was_down = 0,
        has_moved = 0;
    while (1) {
        int bytesRead = read(fd, &pkt, sizeof(pkt));
        // printf("pkt.report_id: %d\n", pkt.report_id);
        // printf("pkt.button: %d\n", pkt.button);
        // printf("pkt.rotation_hb: %d\n", pkt.rotation_hb);
        // printf("pkt.rotation_lb: %d\n", pkt.rotation_lb);
        if (bytesRead == 4) {
            if (pkt.rotation_hb == ROTATE_PLUS) {
                send_dbus_signal(connection, DIAL_ROTATE_SIGNAL, 1);  // Send rotation event
                printf("sent rotate plus\n");
            }

            if (pkt.rotation_hb == ROTATE_MINUS) {
                send_dbus_signal(connection, DIAL_ROTATE_SIGNAL, -1);
                printf("sent rotate minus\n");
            }

            if (pkt.button == BUTTON_DOWN) {
                send_dbus_signal(connection, DIAL_PRESS_SIGNAL, 1);
                was_down = 1;
                printf("sent button down\n");
            }

            if (pkt.button == BUTTON_UP && was_down) {
                send_dbus_signal(connection, DIAL_PRESS_SIGNAL, 0);
                was_down = 0;
                printf("sent button up\n");
            }
        } else {
            fprintf(stderr, "Malformed packet received\n");
        }
    }

    close(fd);
    return 0;
}
