#include "shess.h"

int movelist_add(MoveList *list, move_t move)
{
	move_t *newMoves;

	newMoves = realloc(list->moves, sizeof(*list->moves) *
			(list->numMoves + 1));
	if (newMoves == NULL)
		return -1;
	list->moves = newMoves;
	list->moves[list->numMoves++] = move;
	return 0;
}

bool movelist_targets(MoveList *list, pos_t to)
{
	for (size_t i = 0; i < list->numMoves; i++)
		if (MOVE_TO(list->moves[i]) == to)
			return true;
	return false;
}

void move_getmatching(move_t move, Board *board, MoveList *matching)
{
	MoveList *moves;

	if (MOVE_SIDE(move) != TURN(board))
		return;
	moves = board_generate_moves(board);
	if (moves == NULL)
		return;
	const pos_t from = MOVE_FROM(move);
	for (size_t i = 0; i < moves->numMoves; i++) {
		const move_t o = moves->moves[i];
		if ((move & o) & MOVE_CASTLE) {
			movelist_add(matching, o);
			continue;
		}
		if ((move | o) & MOVE_CASTLE)
			continue;
		if ((move & ~o) & MOVE_TAKES)
			continue;
		if (MOVE_TO(o) != MOVE_TO(move))
			continue;
		if (MOVE_TYPE(move) != 0 && MOVE_TYPE(move) != MOVE_TYPE(o))
			continue;
		const pos_t of = MOVE_FROM(o);
		if (!(move & MOVE_CONFUSED_COL) &&
				from % BOARD_WIDTH != of % BOARD_WIDTH)
			continue;
		if (!(move & MOVE_CONFUSED_ROW) &&
				from / BOARD_WIDTH != of / BOARD_WIDTH)
			continue;
		movelist_add(matching, o);
	}
}

void move_output(move_t move, FILE *fp)
{
	move_t end;

	if (move & MOVE_CASTLE_SHORT) {
		fprintf(fp, "O-O");
	} else if (move & MOVE_CASTLE_LONG) {
		fprintf(fp, "O-O-O");
	} else if ((end = MOVE_END_CONDITION(move)) > 0) {
		switch (end) {
		case MOVE_RESIGNATION:
			fprintf(fp, "(resigns)");
			break;
		case MOVE_STALEMATE:
			fprintf(fp, "(stalemate)");
			break;
		case MOVE_DRAW:
			fprintf(fp, "(draw)");
			break;
		}
	} else {
		const piece_t type = MOVE_TYPE(move);
		const piece_t prom = MOVE_PROMOTION(move);
		const pos_t from = MOVE_FROM(move);
		const pos_t to = MOVE_TO(move);
		if (type != TYPE_PAWN)
			fputc(TYPE_TO_CHAR(type), fp);
		if (move & MOVE_CONFUSED) {
			if (!(move & MOVE_CONFUSED_COL))
				fputc(from % BOARD_WIDTH + 'a', fp);
			if (!(move & MOVE_CONFUSED_ROW))
				fputc(from / BOARD_WIDTH + '1', fp);
		}
		if (move & MOVE_TAKES)
			fputc('x', fp);
		const pos_t row = to / BOARD_WIDTH;
		const pos_t col = to % BOARD_WIDTH;
		fprintf(fp, "%c%c", 'a' + col, '1' + row);
		if (prom != 0)
			fprintf(fp, "=%c", TYPE_TO_CHAR(prom));
	}
	if (move & MOVE_CHECK)
		fputc('+', fp);
	if (move & MOVE_CHECKMATE)
		fputc('#', fp);
}

int move_parse(move_t *pMove, piece_t side, const char *str)
{
	move_t move;

	move = side << MOVE_SIDE_SHIFT;
	if (str[0] == 'O' || str[0] == 'o') {
		if (str[1] != '-')
			return -1;
		if (str[2] != 'O' && str[2] != 'o')
			return -1;
		if (str[3] == '-') {
			move |= MOVE_CASTLE_LONG;
			if (str[3] != 'O' && str[3] != 'o')
				return -1;
		} else {
			move |= MOVE_CASTLE_SHORT;
		}
	} else {
		pos_t col, row;

		switch (str[0]) {
		case 'P':
		case 'p':
		case 'N':
		case 'n':
		case 'B':
		/* no `case 'b':` for the bishop */
		case 'R':
		case 'r':
		case 'Q':
		case 'q':
		case 'K':
		case 'k':
			move |= CHAR_TO_TYPE(toupper(str[0])) <<
				MOVE_TYPE_SHIFT;
			str++;
			break;
		}

		move |= MOVE_CONFUSED;

		if (isdigit(str[0])) {
			if (str[0] == '0' || str[0] == '9')
				return -1;
			move ^= MOVE_CONFUSED_ROW;
			move |= ((str[0] - '1') * BOARD_WIDTH) <<
				MOVE_FROM_SHIFT;
			str++;
		} else if (isalpha(str[0]) && str[0] != 'x') {
			if (str[0] < 'a' || str[0] > 'h')
				return -1;
			move ^= MOVE_CONFUSED_COL;
			move |= (str[0] - 'a') << MOVE_FROM_SHIFT;
			str++;

			if (isdigit(str[0])) {
				if (str[0] < '1' || str[0] > '8')
					return -1;
				move ^= MOVE_CONFUSED_ROW;
				move |= ((str[0] - '1') * BOARD_WIDTH) <<
					MOVE_FROM_SHIFT;
				str++;
			}
			if (str[0] != 'x' && str[0] != '-' &&
					!isalpha(str[0])) {
				move |= MOVE_CONFUSED;
				move |= MOVE_FROM(move) << MOVE_TO_SHIFT;
				goto after;
			}
		} else if (str[0] != 'x' && str[0] != '-') {
			return -1;
		}

		if (str[0] == 'x') {
			move |= MOVE_TAKES;
			str++;
		}
		if (str[0] == '-')
			str++;

		if (str[0] < 'a' || str[0] > 'h')
			return -1;
		col = str[0] - 'a';
		str++;

		if (str[0] < '1' || str[0] > '8')
			return -1;
		row = str[0] - '1';
		str++;

		move |= (row * BOARD_WIDTH + col) << MOVE_TO_SHIFT;

	after:
		if (str[0] == '=') {
			str++;
			switch (str[0]) {
			case 'N':
			case 'B':
			case 'R':
			case 'Q':
			case 'K':
				move |= CHAR_TO_TYPE(str[0]) <<
					MOVE_PROMOTION_SHIFT;
				str++;
				break;
			default:
				return -1;
			}
		}
	}

	if (str[0] == '+' || str[0] == '#')
		move |= str[0] == '+' ? MOVE_CHECK : MOVE_CHECKMATE;

	*pMove = move;
	return 0;
}
