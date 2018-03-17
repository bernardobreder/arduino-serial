//
//  main.c
//  test-serial
//
//  Created by Bernardo Breder on 16/03/18.
//  Copyright © 2018 Breder Company. All rights reserved.
//

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

static unsigned char* usbio_read_bytes(struct usbio_t* self, unsigned char* buffer, size_t length, useconds_t timeout) {
    if (self->closed) return 0;
    size_t time = 0;
    useconds_t sleep_time = 10 * 1000;
    size_t readed = 0;
    while (readed != length && (timeout == 0 || time < timeout)) {
        ssize_t count = read(self->fd, buffer + readed, length - readed);
        if (count < 0) {
            usleep(sleep_time);
            time += sleep_time;
        }
        else { readed += count; }
    }
    if (timeout != 0 && time >= timeout) return 0;
    return buffer;
}

static unsigned char usbio_write_ping(struct usbio_t* self) {
    if (self->closed) return 1;
    const char bytes[] = { 0x01 };
    ssize_t writed = write(self->fd, bytes, 1);
    if (writed != 1) return 1;
    tcflush(self->fd, TCIOFLUSH);
    unsigned char buffer[] = { 0x00, 0x00 };
    if (!usbio_read_bytes(self, buffer, 1, 1000 * 1000)) return 1;
    if (buffer[0] != 0x01) return 1;
    return 0;
}

static unsigned char usbio_write_ping_retry(struct usbio_t* self, int retry) {
    if (self->closed) return 1;
    int n; for (n = 0 ; n < 3 ; n++) {
        if (!usbio_write_ping(self)) return 0;
    }
    return 1;
}

struct usbio_t* usbio_open(struct usbio_t* self, const char* portname, speed_t speed) {
    int fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) return 0;
    
    struct termios tty;
    if (tcgetattr(fd, &tty) < 0) {
        close(fd);
        return 0;
    }
    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);
    
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;
    
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0.5;
    
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        close(fd);
        return 0;
    }
    
    self->fd = fd;
    self->closed = 0;
    return self;
}

void usbio_close(struct usbio_t* self) {
    if (self->closed) return;
    close(self->fd);
    self->closed = 1;
}

unsigned char usbio_write(struct usbio_t* self, const char* command) {
    if (self->closed) return 1;
    size_t command_len = strlen(command);
    size_t cmd_len = command_len + 5;
    unsigned char* cmd = alloca(cmd_len);
    cmd[0] = 0x02;
    cmd[1] = (command_len >> 24) & 0xFF;
    cmd[2] = (command_len >> 16) & 0xFF;
    cmd[3] = (command_len >> 8) & 0xFF;
    cmd[4] = (command_len >> 0) & 0xFF;
    memcpy(cmd + 5, command, command_len);
    ssize_t wlen = write(self->fd, cmd, cmd_len);
    if (wlen != cmd_len) return 1;
    tcflush(self->fd, TCIOFLUSH);
    return 0;
}

char* usbio_read(struct usbio_t* self, size_t* length) {
    if (self->closed) return 0;
    // Realiza a leitura do opcode
    size_t head_max = 5;
    unsigned char head_buf[5];
    if (!usbio_read_bytes(self, head_buf, head_max, 1000 * 1000)) {
        return 0;
    }
    // Calcula a quantidade de bytes para ser lido
    if (head_buf[0] != 0x02) return 0;
    size_t data_len = (size_t) ((size_t) head_buf[1] << 24) + ((size_t) head_buf[2] << 16) + ((size_t) head_buf[3] << 8) + head_buf[4];
    // Realiza a leitura dos bytes
    unsigned char body_buf[data_len];
    if (!usbio_read_bytes(self, body_buf, data_len, 4000 * 1000)) {
        return 0;
    }
    // Aloca o resultado
    char* data = (char*) malloc((data_len + 1) * sizeof(char*));
    if (!data) return 0;
    data[data_len] = 0;
    strncpy(data, (char*) body_buf, data_len);
    if (length) *length = data_len;
    return data;
}

char* usbio_request(struct usbio_t* self, const char* command) {
    if (self->closed) return 0;
    if (usbio_write(self, command)) return 0;
    char* result = usbio_read(self, 0);
    if (result) return result;
    if (usbio_write_ping_retry(self, 8)) return 0;
    if (usbio_write(self, command)) return 0;
    return usbio_read(self, 0);
}

int execute() {
    char *portname = "/dev/tty.usbmodem621";
    struct usbio_t* usb = usbio_open(alloca(sizeof(struct usbio_t)), portname, B9600);
    printf("Opened\n");
    int n; for (n = 0 ; n < 10 ; n++) {
        char* resp = usbio_request(usb, "Bernardo Breder");
        if (!resp) break;
        printf("Resp(%d): %s\n", n + 1, resp);
        free(resp);
    }
    usbio_close(usb);
    printf("Closed\n");
    return 0;
}

int main() {
    setbuf(stdout, 0);
    for (;;) {
        execute();
    }
    return 0;
}

