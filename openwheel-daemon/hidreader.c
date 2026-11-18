#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <dbus/dbus.h>
#include <sys/syslog.h>
#include <errno.h>
#include <string.h>

#include "openwheel.h"
#include "helpers.h"
#include "report.h"
int main(int argc, char **argv) {
    const bool notify = argc == 1;
    if (!notify && (argc != 2 || strcmp(argv[1], "--no-notify") != 0)) {
        fprintf(stderr, "Usage: %s [--no-notify]\n", argv[0]);
        return 1;
    }
    openlog("openwheel", LOG_PID, LOG_DAEMON);

    char device_path[BUFFER_SIZE];
    if (find_hidraw_device(device_path, sizeof(device_path)) < 0) {
        syslog(LOG_ERR, "ASUS Dial device not found");
        return 1;
    }

    syslog(LOG_INFO, "Found ASUS Dial at %s", device_path);
    int fd = open(device_path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        syslog(LOG_ERR, "Failed to open HID device %s: %s", device_path, strerror(errno));
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
        close(fd);
        return 1;
    }

    // Request the name 'org.asus.dial'
    int ret = dbus_bus_request_name(connection, "org.asus.dial", DBUS_NAME_FLAG_DO_NOT_QUEUE, &err);
    if (dbus_error_is_set(&err)) {
        syslog(LOG_ERR, "Failed to request D-Bus name 'org.asus.dial' (%s)", err.message);
        dbus_error_free(&err);
    }
    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        syslog(LOG_ERR, "Failed to request D-Bus name 'org.asus.dial'");
        dbus_connection_unref(connection);
        close(fd);
        return 1;
    }

    uint8_t packet[64];
    WheelEvent events[WHEEL_MAX_EVENTS];
    bool button_down = false;
    if (notify) {
        send_notification("OpenWheel", "OpenWheel openwheel-daemon started");
    }
    for (;;) {
        ssize_t bytes_read = read(fd, packet, sizeof(packet));
        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;
            }
            syslog(LOG_ERR, "HID read failed for %s: %s", device_path, strerror(errno));
            break;
        }
        if (bytes_read == 0) {
            syslog(LOG_ERR, "HID device disconnected: %s", device_path);
            break;
        }

        size_t count = wheel_process_report(packet, (size_t)bytes_read, &button_down, events);
        for (size_t i = 0; i < count; i++) {
            const char *name = events[i].type == WHEEL_ROTATE
                ? DIAL_ROTATE_SIGNAL : DIAL_PRESS_SIGNAL;
            send_dbus_signal(connection, name, events[i].value);
        }
    }
    close(fd);
    dbus_connection_unref(connection);
    return 1; /* Let a session supervisor reconnect after device removal. */
}
