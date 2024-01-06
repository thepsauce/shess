#include "test.h"

#define BITS(s) ((s[0]-'a')+(s[1]-'1')*BOARD_WIDTH)

int main(void)
{
	FILE *fp;
	Game game;

	if (game_init(&game) < 0)
		return -1;

	const move_t moves[] = {
		MAKE_PIECE(SIDE_WHITE, TYPE_QUEEN) |
			(BITS("a4") << MOVE_FROM_SHIFT) |
			(BITS("c6") << MOVE_TO_SHIFT),
		MAKE_PIECE(SIDE_WHITE, TYPE_ROOK) |
			(BITS("c6") << MOVE_TO_SHIFT) |
			MOVE_CONFUSED | MOVE_CHECK,
		MAKE_PIECE(SIDE_WHITE, TYPE_PAWN) |
			(BITS("c7") << MOVE_FROM_SHIFT) |
			(BITS("c8") << MOVE_TO_SHIFT) |
			(TYPE_KNIGHT << MOVE_PROMOTION_SHIFT),
		(MOVE_RESIGNATION << MOVE_END_CONDITION_SHIFT),

	};
	for (size_t i = 0; i < ARRLEN(moves); i++) {
		move_output(moves[i], stdout);
		printf("\n");
	}

	fp = fopen("test.txt", "r");
	gamedata_input(&game.data, fp);
	fclose(fp);
	printf("Game data:\n");
	gamedata_output(&game.data, stdout);

	game_uninit(&game);
	return 0;
}
