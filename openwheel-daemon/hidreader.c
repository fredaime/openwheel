#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include <sys/syslog.h>

#include "openwheel.h"
#include "helpers.h"



int main() {
    openlog("openwheel", LOG_PID, LOG_DAEMON);
    int fd = open(HIDRAW_DEVICE, O_RDONLY);
    if (fd < 0) {
        syslog(LOG_ERR, "Failed to open HID device %s", HIDRAW_DEVICE);
        return 1;
    }

    // Set up D-Bus connection
    DBusConnection* connection;
    DBusError err;

    dbus_error_init(&err);
    connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        syslog(LOG_ERR, "D-Bus connection error (%s)", err.message);
        dbus_error_free(&err);
    }
    if (connection == NULL) {
        return 1;
    }

    // Request the name 'org.asus.dial'
    int ret = dbus_bus_request_name(connection, "org.asus.dial", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        syslog(LOG_ERR, "Failed to request D-Bus name 'org.asus.dial' (%s)", err.message);
        dbus_error_free(&err);
    }
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        syslog(LOG_ERR, "Failed to request D-Bus name 'org.asus.dial'");
        return 1;
    }

    WheelPacket pkt;
    int was_down = 0;

    send_notification("OpenWheel", "OpenWheel openwheel-daemon started");
    while (1) {
        int bytesRead = read(fd, &pkt, sizeof(pkt));
        if (bytesRead < 0) {
            syslog(LOG_ERR, "Failed to read from HID device");
            break;
        }
        if (bytesRead == 4) {
            if (pkt.rotation_hb == ROTATE_PLUS) {
                send_dbus_signal(connection, DIAL_ROTATE_SIGNAL, 1);  // Send rotation event
                syslog(LOG_DEBUG, "sent rotate plus");
            }

            if (pkt.rotation_hb == ROTATE_MINUS) {
                send_dbus_signal(connection, DIAL_ROTATE_SIGNAL, -1);
                syslog(LOG_DEBUG,"sent rotate minus");
            }

            if (pkt.button == BUTTON_DOWN) {
                send_dbus_signal(connection, DIAL_PRESS_SIGNAL, 1);
                was_down = 1;
                syslog(LOG_DEBUG,"sent button down");
            }

            if (pkt.button == BUTTON_UP && was_down) {
                send_dbus_signal(connection, DIAL_PRESS_SIGNAL, 0);
                was_down = 0;
                syslog(LOG_DEBUG,"sent button up");
            }
        } else {
            syslog(LOG_ERR, "Malformed packet received");
        }
    }

    close(fd);
    return 0;
}
