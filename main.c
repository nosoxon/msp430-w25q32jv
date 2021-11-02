#include <msp430g2553.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "instructions.h"
#include "serial.h"

typedef enum {
	STATE_TX,
	STATE_RX,
	STATE_IDLE
} spi_state_t;

volatile spi_state_t spi_state = STATE_IDLE;

#define MAX_CMD_LEN 64

static volatile size_t cmd_len = 0;
static uint8_t cmd_buf[MAX_CMD_LEN];

int main(void) {
	WDTCTL = WDTPW + WDTHOLD; // Stop WDT

	P1DIR |= BIT0;
	P1OUT  = BIT0; // Set P1.0

	/* DCO freq = 16MHz (SLAU144J:5.2.5) */
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL  = CALDCO_16MHZ;

	/* Select pin functions (SLAS735J) */
	P1SEL  |= 0b11100110;          // P1.1 = UCA0RXD, P1.2 = UCA0TXD
	P1SEL2 |= 0b11100110;          // P1.5 = UCB0CLK, P1.6 = UCB0SOMI, P1.7 = UCB0SIMO

	/* Configure hardware UART (SLAU144J:15) */
	UCA0CTL1 |= UCSSEL_2;          // Use SMCLK
	UCA0BR0   = 104;               // 9600 baud
	UCA0BR1   = 0;
	UCA0MCTL  = UCBRS0 + UCOS16;   // modulation
	UCA0CTL1 &= ~UCSWRST;          // disable reset

	/* Configure SPI (SLAU144J:16) */
	UCB0CTL1  = UCSWRST;           // software reset
	UCB0CTL0 |= UCCKPL             // clock phase
		  + UCMSB              // MSB first
		  + UCMST              // master mode
		  + UCSYNC;            // synchronous mode
	UCB0CTL1 |= UCSSEL_2;	       // Use SMCLK
	UCB0BR0 = 8;                   // SMCLK / 8
	UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST;          // disable reset

	IE2 |= UCA0RXIE + UCB0RXIE;    // enable USCI_A0 + USCI_B0 RX interrupts

	P1DIR  |= BIT4;                // P1.4 = SMCLK
	P1SEL  |= BIT4;
	P1SEL2 &= ~BIT4;

	__delay_cycles(100);

	_BIS_SR(LPM0_bits + GIE);      // Enter LPM0, interrupts enabled
	return 0;
}

void execute_cmd(char *instructions) {
	struct cmd_s *cmd = parse_instruction(instructions);
	if (!cmd) return;

	P1OUT &= ~BIT0;                // CS low
	spi_state = STATE_TX;
	SPI_RXC = cmd->rxc;
	spi_send(cmd->tx_buf, cmd->txc);

	free(cmd->tx_buf);
	free(cmd);
}

void on_uart_recv(uint8_t rx) {
	if (rx == '\r' || cmd_len > MAX_CMD_LEN - 2) {
		if (!cmd_len) return;

		cmd_buf[cmd_len] = '\0';
		cmd_len = 0;

		uart_print("\r\n");
		execute_cmd((char *) cmd_buf);
	} else {
		cmd_buf[cmd_len++] = rx;
		uart_send_byte(rx);
	}
}

void on_spi_recv(uint8_t rx) {
	switch (spi_state) {
		case STATE_RX:
			if (SPI_RXC) {
				--SPI_RXC;
				spi_send_byte(0x00);
				uart_send_byte(rx);
			} else {
				uart_print("\r\n");
				spi_state = STATE_IDLE;
				P1OUT |= BIT0;
			}
			break;
		case STATE_TX:
			if (!SPI_TXC && SPI_RXC) {
				spi_state = STATE_RX;
				spi_send_byte(0x00);
			}
			break;
		case STATE_IDLE:
		default:
			_no_operation();
			break;
	}
}

void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCIAB0RX_ISR(void) {
	     if (IFG2 & UCA0RXIFG) on_uart_recv(UCA0RXBUF);
	else if (IFG2 & UCB0RXIFG) on_spi_recv(UCB0RXBUF);
}

