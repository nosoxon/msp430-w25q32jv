#include <sys/types.h>

#ifndef SERIAL_H
#define SERIAL_H

void spi_send_byte(uint8_t b);
void spi_send(uint8_t *payload, size_t len);

void uart_send_byte(uint8_t b);
void uart_send(uint8_t *payload, size_t len);
void uart_print(char *string);

extern volatile uint32_t SPI_RXC;
extern volatile uint32_t SPI_TXC;

#endif /* SERIAL_H */