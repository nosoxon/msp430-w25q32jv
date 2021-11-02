#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "instructions.h"

uint8_t hex_value(char h) {
	if (h >= '0' && h <= '9') return h - '0';
	if (h >= 'a' && h <= 'f') return h - 'a' + 10;
	if (h >= 'A' && h <= 'F') return h - 'A' + 10;
	return 0;
}

struct cmd_s *parse_instruction(char *input) {
	struct cmd_s *cmd = malloc(sizeof(struct cmd_s));
	if (!cmd) return NULL;

	char *data;
	cmd->rxc = strtoul(input, &data, 0);
	/* return if conversion fails, else advance past space */
	if (input == data++) goto cleanup;

	cmd->txc = strlen(data);
	/* data length must be positive and even */
	if (!cmd->txc || cmd->txc % 2 != 0) goto cleanup;

	cmd->txc >>= 1;
	cmd->tx_buf = malloc(cmd->txc);
	if (!cmd->tx_buf)  goto cleanup;

	for (size_t i = 0; i < cmd->txc; ++i)
		cmd->tx_buf[i] = (hex_value(data[2*i]) << 4) + hex_value(data[2*i + 1]);

	return cmd;

cleanup:
	free(cmd);
	return NULL;
}