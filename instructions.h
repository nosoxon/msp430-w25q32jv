#include <stdint.h>

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

struct cmd_s {
	uint32_t    rxc;
	uint32_t    txc;
	uint8_t     *tx_buf;
};

struct cmd_s *parse_instruction(char *input);

#endif /* INSTRUCTIONS_H */
