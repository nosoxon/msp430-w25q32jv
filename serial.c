#include <msp430g2553.h>
#include <string.h>

#include "serial.h"

volatile uint32_t SPI_RXC = 0;
volatile uint32_t SPI_TXC = 0;

/* send a single byte over SPI */
void spi_send_byte(uint8_t b) {
	while (!(IFG2 & UCB0TXIFG));
	UCB0TXBUF = b;
}

void spi_send(uint8_t *payload, size_t len) {
	SPI_TXC += len;
	while (len--) {
		spi_send_byte(*payload++);
		--SPI_TXC;
	}
}

/* send a single byte over UART */
void uart_send_byte(uint8_t b) {
	while (!(IFG2 & UCA0TXIFG));
	UCA0TXBUF = b;
}

void uart_send(uint8_t *payload, size_t len) {
	while (len--) uart_send_byte(*payload++);
}

/* send a string over UART */
void uart_print(char *string) {
	uart_send((uint8_t*) string, strlen(string));
}
