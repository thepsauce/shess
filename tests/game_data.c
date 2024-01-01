#include "test.h"

#define BITS(s) ((s[0]-'a')+(s[1]-'1')*BOARD_WIDTH)

extern char piece_letters[];

void print_move(move_t move, FILE *fp)
{
	move_t end;

	if (move & MOVE_CASTLE_SHORT) {
		fprintf(fp, "O-O");
	} else if (move & MOVE_CASTLE_LONG) {
		fprintf(fp, "O-O-O");
	} else if ((end = MOVE_END_CONDITION(move)) > 0) {
		switch (end) {
		case MOVE_RESIGNATION:
			printf("(resigns)");
			break;
		case MOVE_STALEMATE:
			printf("(stalemate)");
			break;
		case MOVE_DRAW:
			printf("(draw)");
			break;
		}
	} else {
		const piece_t type = MOVE_TYPE(move);
		const piece_t prom = MOVE_PROMOTION(move);
		const int8_t from = MOVE_FROM(move);
		const int8_t to = MOVE_TO(move);
		if (!(move & MOVE_CONFUSED)) {
			const int row = from / BOARD_WIDTH;
			const int col = from % BOARD_WIDTH;
			fprintf(fp, "%c%c", 'a' + col, '1' + row);
			if (move & MOVE_TAKES)
				fputc('x', fp);
			else
				fputc('-', fp);
		} else {
			if (type != TYPE_PAWN)
				fputc(piece_letters[type], fp);
			if (move & MOVE_TAKES)
				fputc('x', fp);
		}
		const int row = to / BOARD_WIDTH;
		const int col = to % BOARD_WIDTH;
		fprintf(fp, "%c%c", 'a' + col, '1' + row);
		if (prom != 0)
			fprintf(fp, "=%c", piece_letters[prom]);
		if (move & MOVE_CHECK)
			fputc('+', fp);
		if (move & MOVE_CHECKMATE)
			fputc('#', fp);
	}
}

int main(void)
{
	FILE *fp;
	GameData gd;

	const move_t moves[] = {
		(SIDE_WHITE << MOVE_SIDE_SHIFT) |
			(TYPE_QUEEN << MOVE_TYPE_SHIFT) |
			(BITS("a4") << MOVE_FROM_SHIFT) |
			(BITS("c6") << MOVE_TO_SHIFT),
		(SIDE_WHITE << MOVE_SIDE_SHIFT) |
			(TYPE_ROOK << MOVE_TYPE_SHIFT) |
			(BITS("c6") << MOVE_TO_SHIFT) |
			MOVE_CONFUSED | MOVE_CHECK,
		(MOVE_RESIGNATION << MOVE_END_CONDITION_SHIFT),
	};
	for (size_t i = 0; i < ARRLEN(moves); i++) {
		print_move(moves[i], stdout);
		printf("\n");
	}

	fp = fopen("test.txt", "r");
	memset(&gd, 0, sizeof(gd));
	game_data_input(&gd, fp);
	fclose(fp);
	printf("Game data:\n");
	game_data_output(&gd, stdout);
	return 0;
}
