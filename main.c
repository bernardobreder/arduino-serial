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
#include "arduino-serial.h"

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

