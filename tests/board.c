#include "test.h"

int main(void)
{
	Board b;
	const char *const fen = "8/8/3k4/8/3K4/8/3R4/8 w - - 0 1";
	const char *end;

	board_setup_starting(&b);
	board_neat_output(&b, stdout);

	if (board_setup_fen(&b, fen, &end) != 0)
		printf("failed parsing fen at: %s\n", end);
	board_neat_output(&b, stdout);

	return 0;
}
