#include "shess.h"

char *read_line(void)
{
	static char *line;
	static size_t szLine;
	ssize_t lenLine;
	char *ptr;

	if ((lenLine = getline(&line, &szLine, stdin)) <= 0)
		return NULL;
	for (; isspace(line[lenLine - 1]); lenLine--);
	line[lenLine] = '\0';
	for (ptr = line; isspace(*ptr); ptr++);
	return ptr;
}

move_t choose_move(MoveList *list)
{
	char *line;

	if (list->numMoves == 1)
		return list->moves[0];

	printf("Choose one of the following moves:\n");
	for (size_t i = 0; i < list->numMoves; i++) {
		printf("%zu) ", i + 1);
		move_output(list->moves[i], stdout);
		printf("\n");
	}
	printf("[N]one\n");

	if ((line = read_line()) == NULL)
		return (move_t) -1;
	if (*line == '\0' || isalpha(*line))
		return (move_t) -1;
	if (isdigit(*line)) {
		size_t num;

		num = *line - '0';
		while (line++, isdigit(*line)) {
			num *= 10;
			num += *line - '0';
		}
		if (*line != '\0') {
			printf("invalid input\n");
			return (move_t) -1;
		}
		if (num == 0 || num > list->numMoves) {
			printf("invalid input: number invalid\n");
			return (move_t) -1;
		}
		return list->moves[num - 1];
	} else {
		printf("invalid input\n");
	}
	return (move_t) -1;
}

int main(void)
{
	Game game;

	char *line;

	bool stalemate;
	MoveList moves;

	move_t m;
	UndoData ud;

	if (game_init(&game) < 0)
		return -1;

	//board_setup_fen(&game.board, "3k4/8/8/8/8/8/1p6/2Q1K3 b - - 0 1", NULL);
	//board_setup_starting(&game.board);
	//board_setup_fen(&game.board, "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4", NULL);
	board_setup_fen(&game.board, "rnbqk1nr/ppp2ppp/8/2bpP3/2B1P3/5N2/PPP2PPP/RNBQK2R w KQkq d6 0 6", NULL);
	goto end;
	while ((line = read_line()) != NULL) {
		if (strcmp(line, "quit") == 0)
			break;

		if (strcmp(line, "undo") == 0) {
			if (history_undo(&game.data.history, &game.board) == 0)
				goto end;
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
			if (moves->numMoves == 0)
				printf("No moves!\n");
			else
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

		memset(&moves, 0, sizeof(moves));
		move_getmatching(m, &game.board, &moves);
		if (moves.numMoves == 0) {
			printf("illegal move\n");
			continue;
		}
		m = choose_move(&moves);
		free(moves.moves);
		if (m == (move_t) -1)
			continue;
		board_play_move(&game.board, m, &ud);
		history_add(&game.data.history, &ud);

	end:
		printf("%s\n", board_to_fen(&game.board));
		board_neat_output(&game.board, stdout);
		stalemate = board_generate_moves(&game.board)->numMoves == 0;
		if (game.board.flags & CHECK) {
			if (stalemate)
				printf("Checkmate. %s wins!\n",
						TURN(&game.board) ==
						SIDE_BLACK ? "White" :
						"Black");
			else
				printf("You are in check!\n");
		} else if (stalemate) {
			printf("Draw. Stalemate!\n");
		}
	}
	return 0;
}
