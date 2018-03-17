# Introdução

O objetivo desse projeto é exercitar a comunicação de uma placa arduino com um processo C. Nesse projeto, foi criado uma biblioteca para exercitar essa comunicação

# Biblioteca

A biblioteca criada por esse projeto foi implementada em cima de um protocolo de comunicação tanto no processo C, quanto no código do arduino. Dessa forma, o arduino deve implementar o protocolo para que a biblioteca funcione corretamente, já que a comunicação Serail é muito precaria de confiabilidade de transmissão bi-direcional de dados.

O código exemplo do arduino realizado nesse projeto de Echo é o seguinte:

```c++
String input = "";

void setup() {
    Serial.begin(9600);
    input.reserve(200);
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
                size_t count = (((size_t)buf[1]) << 24) + (((size_t)buf[2]) << 16) + (((size_t)buf[3]) << 8) + buf[4];
                if (len >= 5 + count) {
                    String msg = input.substring(5, 5 + count);
                    Serial.write(0x02);
                    Serial.write((count >> 24) & 0xFF);
                    Serial.write((count >> 16) & 0xFF);
                    Serial.write((count >> 8) & 0xFF);
                    Serial.write((count >> 0) & 0xFF);
                    Serial.print(msg);
                    Serial.flush();
                    input = input.substring(5 + count);
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

Para iniciar a comunicação com o arduino, deve ser aberto a comunicação da seguinte forma:

```c
char *portname = "/dev/tty.usbmodem621";
struct usbio_t* usb = usbio_open(alloca(sizeof(struct usbio_t)), portname, B9600);
``` 

Para fechar a comunicação com o arduino, deve ser executado o comando de fechamento

```c
usbio_close(usb);
``` 
