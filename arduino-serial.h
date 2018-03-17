
#ifndef ARDUINO_SERIAL_H
#define ARDUINO_SERIAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

struct usbio_t {
    int fd;
    unsigned char closed;
};

struct usbio_t* usbio_open(struct usbio_t* self, const char* portname, speed_t speed);

void usbio_close(struct usbio_t* self);

unsigned char usbio_write(struct usbio_t* self, const char* command);

char* usbio_read(struct usbio_t* self, size_t* length);

char* usbio_request(struct usbio_t* self, const char* command);

#endif
