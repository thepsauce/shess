#include "test.h"

int main(void)
{
	Game game;

	if (game_init(&game) < 0)
		return -1;
	board_setup_starting(&game.board);
	board_neat_output(&game.board, stdout);
	for (pos_t row = BOARD_HEIGHT; (row--) > 0; ) {
		for (pos_t col = 0; col < BOARD_WIDTH; col++) {
			const pos_t pos = row * BOARD_WIDTH + col;
			if (board_is_attacked(&game.board, pos, SIDE_ENEMY(TURN(&game.board))))
				printf("1");
			else
				printf("0");
			printf(" ");
		}
		printf("\n");
	}
	game_uninit(&game);
	return 0;
}
