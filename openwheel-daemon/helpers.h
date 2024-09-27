//
// Created by faime on 27/09/24.
//

#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dbus/dbus.h>
#include <string.h>
#include <dirent.h>


void send_dbus_signal(DBusConnection *connection, const char *signal_name, int value);

int read_sysfs_file(const char *path, char *buffer, size_t buffer_size);

int find_hidraw_device(char *device_path, size_t path_size);

void daemonize();

void send_notification(const char *summary, const char *body);


#endif //HELPERS_H
