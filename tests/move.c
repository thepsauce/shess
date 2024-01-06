#include "shess.h"

int main(void)
{
	Game game;

	char *line = NULL;
	size_t szLine = 0;
	ssize_t numLine;

	move_t m;
	UndoData ud;

	if (game_init(&game) < 0)
		return -1;

	//board_setup_fen(&game.board, "3k4/8/8/8/8/8/1p6/2Q1K3 b - - 0 1", NULL);
	//board_setup_starting(&game.board);
	//board_setup_fen(&game.board, "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4", NULL);
	board_setup_fen(&game.board, "rnbqk1nr/ppp2ppp/8/2bpP3/2B1P3/5N2/PPP2PPP/RNBQK2R w KQkq d6 0 6", NULL);
	board_neat_output(&game.board, stdout);
	while ((numLine = getline(&line, &szLine, stdin)) > 0) {
		line[numLine - 1] = '\0';

		if (strcmp(line, "quit") == 0)
			break;

		if (strcmp(line, "undo") == 0) {
			if (history_undo(&game.data.history, &game.board) == 0)
				board_neat_output(&game.board, stdout);
			continue;
		}

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
		board_play_move(&game.board, m, &ud);
		history_add(&game.data.history, &ud);
		board_neat_output(&game.board, stdout);
	}
	return 0;
}
