#include "shess.h"

int main(void)
{
	Game game;

	char *line = NULL;
	size_t szLine = 0;
	ssize_t numLine;

	move_t m;

	memset(&game, 0, sizeof(game));

	board_setup_fen(&game.board, "3k4/8/8/8/8/8/1p6/2Q1K3 b - - 0 1", NULL);
	board_neat_output(&game.board, stdout);
	while ((numLine = getline(&line, &szLine, stdin)) > 0) {
		line[numLine - 1] = '\0';

		if (strcmp(line, "quit") == 0)
			break;

		if (strcmp(line, "moves") == 0) {
			MoveList *const moves =
				board_generate_moves(&game.board);
			for (size_t i = 0; i < moves->numMoves; i++) {
				if (i > 0)
					printf("  ");
				move_output(moves->moves[i], stdout);
			}
			printf("\n");
			continue;
		}

		if (move_parse(&m, TURN(&game.board), line) < 0) {
			printf("invalid move\n");
			continue;
		}
		printf("read move as: ");
		move_output(m, stdout);
		printf("\n");
		if (move_validate(&m, &game.board) < 0) {
			printf("ambigouos move\n");
			continue;
		}
		printf("found move!\n");
		board_play_move(&game.board, m);
		movelist_add(&game.data.moves, m);
		board_neat_output(&game.board, stdout);
	}
	return 0;
}
