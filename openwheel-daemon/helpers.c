//
// Created by faime on 27/09/24.
//

#include "helpers.h"
#include "openwheel.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>

void daemonize()
{
    pid_t sid;

    // Fork off the parent process
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    // Exit the parent process
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    // Change the current working directory to root to avoid keeping directories in use
    if (chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }

    // Set file permissions mask to 0 to ensure proper access rights
    umask(0);

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Optionally redirect STDIN, STDOUT, and STDERR to a log file
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
}

void signal_handler(int signal)
{
    switch (signal) {
        case SIGHUP:
            syslog(LOG_INFO, "Received SIGHUP signal.");
        // Reload configuration
        break;
        case SIGTERM:
            syslog(LOG_INFO, "Received SIGTERM signal, shutting down.");
        // Clean up and shut down
        exit(EXIT_SUCCESS);
        break;
        default:
            syslog(LOG_INFO, "Unhandled signal %d", signal);
        break;
    }
}

void setup_signal_handlers()
{
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

void write_pid_file()
{
    FILE *pid_file = fopen("/var/run/openwheel.pid", "w");
    if (pid_file == NULL) {
        syslog(LOG_ERR, "Could not open PID file.");
        exit(EXIT_FAILURE);
    }
    fprintf(pid_file, "%d\n", getpid());
    fclose(pid_file);
}

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

// Helper function to read a file into a buffer
int read_sysfs_file(const char *path, char *buffer, size_t buffer_size) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return -1;
    }

    if (fgets(buffer, buffer_size, fp) == NULL) {
        fclose(fp);
        return -1;
    }

    fclose(fp);

    // Remove trailing newline if it exists
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    return 0;
}

// Function to search for the ASUS2020 device in /sys/class/hidraw
int find_hidraw_device(char *device_path, size_t path_size) {
    struct dirent *entry;
    DIR *dp = opendir(SYSFS_HIDRAW_PATH);

    if (dp == NULL) {
        perror("opendir");
        return -1;
    }

    // Iterate through the hidrawX directories
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_type == DT_LNK) {
            char sysfs_device_path[BUFFER_SIZE];
            char product_name[BUFFER_SIZE];

            // Build the path to the 'device' symlink
            snprintf(sysfs_device_path, sizeof(sysfs_device_path),
                     "%s%s/device/product", SYSFS_HIDRAW_PATH, entry->d_name);

            // Read the product name of the HID device
            if (read_sysfs_file(sysfs_device_path, product_name, sizeof(product_name)) == 0) {
                // Check if the product name matches "ASUS2020"
                if (strstr(product_name, DEVICE_NAME)) {
                    // Build the path to the hidraw device
                    snprintf(device_path, path_size, "/dev/%s", entry->d_name);
                    closedir(dp);
                    return 0;  // Found the device
                }
            }
        }
    }

    closedir(dp);
    return -1;  // Device not found
}

// Function to send notification via D-Bus
void send_notification(const char *summary, const char *body) {
    DBusMessage *msg;
    DBusMessageIter args;
    DBusError err;
    DBusConnection *conn;
    dbus_uint32_t serial = 0;

    // Initialize the error
    dbus_error_init(&err);

    // Connect to the session bus
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error: %s\n", err.message);
        dbus_error_free(&err);
    }
    if (conn == NULL) {
        return;
    }

    // Create a new method call to org.freedesktop.Notifications
    msg = dbus_message_new_method_call("org.freedesktop.Notifications", // Target service
                                       "/org/freedesktop/Notifications", // Object path
                                       "org.freedesktop.Notifications",  // Interface
                                       "Notify");                        // Method

    if (msg == NULL) {
        fprintf(stderr, "Message Null\n");
        return;
    }

    // Append arguments to the message
    dbus_message_iter_init_append(msg, &args);
    const char *app_name = "ASUS Dial Daemon";
    dbus_uint32_t id = 0; // notification ID
    const char *icon = "dialog-information";
    int32_t timeout = 5000; // 5 seconds

    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &app_name);  // Application name
    dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &id);        // Notification ID
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &icon);      // Icon
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &summary);   // Summary (title)
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &body);      // Body (message)
    dbus_message_iter_append_basic(&args, DBUS_TYPE_ARRAY, &timeout);    // Timeout

    // Send the message
    if (!dbus_connection_send(conn, msg, &serial)) {
        fprintf(stderr, "Out Of Memory!\n");
    }

    dbus_connection_flush(conn);
    dbus_message_unref(msg);
}