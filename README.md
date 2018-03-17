# Introdução

O objetivo desse projeto é exercitar a comunicação de uma placa arduino com um processo C. Nesse projeto, foi criado uma biblioteca para exercitar essa comunicação. Como a comunicação entre o PC e o Arduino não é tão harmonica, um protocolo de comunicação foi criada para garantir a entrega correta do pacote e o recebimento pelo PC.

# Biblioteca

A biblioteca criada por esse projeto foi implementada em cima de um protocolo de comunicação tanto no PC, quanto no Arduino. Para isso, o Arduino deve possuir um código base para receber e tratar o comando, gerenciada por uma biblioteca que gerencia a comunicação bi-direcional dos dados.

Um projeto base abaixo mostra no método `loop()` o recebimento da requisição vinda do PC e o tratamento da solicitação pelo método `execute(String)`.

```c++
String input = "";

void setup() {
    Serial.begin(9600);
    input.reserve(200);
}

String execute(String request) {
    return request;
}

void loop() {
    int len = input.length();
    unsigned char* buf = input.c_str();
    if (len > 0) {
        int opcode = buf[0] & 0xFF;
        if (opcode == 0x01) {
            Serial.write(0x01);
            input = input.substring(1);
            Serial.flush();
        } else if (opcode == 0x02) {
            if (len >= 5) {
                size_t request_len = (((size_t)buf[1]) << 24) + (((size_t)buf[2]) << 16) + (((size_t)buf[3]) << 8) + buf[4];
                if (len >= 5 + request_len) {
                    String request = input.substring(5, 5 + request_len);
                    String response = execute(request);
                    size_t response_len = response.length();
                    Serial.write(0x02);
                    Serial.write((response_len >> 24) & 0xFF);
                    Serial.write((response_len >> 16) & 0xFF);
                    Serial.write((response_len >> 8) & 0xFF);
                    Serial.write((response_len >> 0) & 0xFF);
                    Serial.print(response);
                    Serial.flush();
                    input = input.substring(5 + request_len);
                }
            }
        } else {
            input = "";
        }
    }
}

void serialEvent() {
    while (Serial.available()) {
        input += (char) Serial.read();
        if (!Serial.available()) { delay(100); }
    }
}
```

No PC, para iniciar a comunicação com o Arduino, deve ser aberto a comunicação da seguinte forma:

```c
char *portname = "/dev/tty.usbmodem621";
struct usbio_t* usb = usbio_open(alloca(sizeof(struct usbio_t)), portname, B9600);
``` 

Para fechar a comunicação com o arduino, deve ser executado o comando abaixo:

```c
usbio_close(usb);
``` 

Para enviar uma mensagem e esperar pela resposta, deve ser executado o comando abaixo:

```c 
char* resp = usbio_request(usb, "Echo...");
if (resp) {
    printf("Resp(%d): %s\n", n + 1, resp);
    free(resp);
}
``` 

Uma demo de programa C para execução de Echo

```c
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
``` 
