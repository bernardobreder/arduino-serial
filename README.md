# Introdução

O objetivo desse projeto é exercitar a comunicação de uma placa arduino com um processo C. Nesse projeto, foi criado uma biblioteca para exercitar essa comunicação

# Biblioteca

Para iniciar a comunicação com o arduino, deve ser aberto a comunicação da seguinte forma:

```c
char *portname = "/dev/tty.usbmodem621";
struct usbio_t* usb = usbio_open(alloca(sizeof(struct usbio_t)), portname, B9600);
``` 

Para fechar a comunicação com o arduino, deve ser executado o comando de fechamento

```c
usbio_close(usb);
``` 
