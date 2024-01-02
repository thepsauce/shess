#include "test.h"

int main(void)
{
	Board b;

	board_setup_starting(&b);
	board_neat_output(&b, stdout);
	return 0;
}
