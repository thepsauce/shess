#include "shess.h"

void board_setup_starting(Board *board)
{
	memset(board, 0, sizeof(*board));

	board->flags = SIDE_WHITE;
	board->flags |= CASTLE_SHORT_WHITE | CASTLE_LONG_WHITE |
		CASTLE_SHORT_BLACK | CASTLE_LONG_BLACK;

	BOARD_SET(board, 0, MAKE_PIECE(SIDE_WHITE, TYPE_ROOK));
	BOARD_SET(board, 1, MAKE_PIECE(SIDE_WHITE, TYPE_KNIGHT));
	BOARD_SET(board, 2, MAKE_PIECE(SIDE_WHITE, TYPE_BISHOP));
	BOARD_SET(board, 3, MAKE_PIECE(SIDE_WHITE, TYPE_QUEEN));
	BOARD_SET(board, 4, MAKE_PIECE(SIDE_WHITE, TYPE_KING));
	BOARD_SET(board, 5, MAKE_PIECE(SIDE_WHITE, TYPE_BISHOP));
	BOARD_SET(board, 6, MAKE_PIECE(SIDE_WHITE, TYPE_KNIGHT));
	BOARD_SET(board, 7, MAKE_PIECE(SIDE_WHITE, TYPE_ROOK));

	BOARD_SET(board, 8, MAKE_PIECE(SIDE_WHITE, TYPE_PAWN));
	BOARD_SET(board, 9, MAKE_PIECE(SIDE_WHITE, TYPE_PAWN));
	BOARD_SET(board, 10, MAKE_PIECE(SIDE_WHITE, TYPE_PAWN));
	BOARD_SET(board, 11, MAKE_PIECE(SIDE_WHITE, TYPE_PAWN));
	BOARD_SET(board, 12, MAKE_PIECE(SIDE_WHITE, TYPE_PAWN));
	BOARD_SET(board, 13, MAKE_PIECE(SIDE_WHITE, TYPE_PAWN));
	BOARD_SET(board, 14, MAKE_PIECE(SIDE_WHITE, TYPE_PAWN));
	BOARD_SET(board, 15, MAKE_PIECE(SIDE_WHITE, TYPE_PAWN));

	BOARD_SET(board, 48, MAKE_PIECE(SIDE_BLACK, TYPE_PAWN));
	BOARD_SET(board, 49, MAKE_PIECE(SIDE_BLACK, TYPE_PAWN));
	BOARD_SET(board, 50, MAKE_PIECE(SIDE_BLACK, TYPE_PAWN));
	BOARD_SET(board, 51, MAKE_PIECE(SIDE_BLACK, TYPE_PAWN));
	BOARD_SET(board, 52, MAKE_PIECE(SIDE_BLACK, TYPE_PAWN));
	BOARD_SET(board, 53, MAKE_PIECE(SIDE_BLACK, TYPE_PAWN));
	BOARD_SET(board, 54, MAKE_PIECE(SIDE_BLACK, TYPE_PAWN));
	BOARD_SET(board, 55, MAKE_PIECE(SIDE_BLACK, TYPE_PAWN));

	BOARD_SET(board, 56, MAKE_PIECE(SIDE_BLACK, TYPE_ROOK));
	BOARD_SET(board, 57, MAKE_PIECE(SIDE_BLACK, TYPE_KNIGHT));
	BOARD_SET(board, 58, MAKE_PIECE(SIDE_BLACK, TYPE_BISHOP));
	BOARD_SET(board, 59, MAKE_PIECE(SIDE_BLACK, TYPE_QUEEN));
	BOARD_SET(board, 60, MAKE_PIECE(SIDE_BLACK, TYPE_KING));
	BOARD_SET(board, 61, MAKE_PIECE(SIDE_BLACK, TYPE_BISHOP));
	BOARD_SET(board, 62, MAKE_PIECE(SIDE_BLACK, TYPE_KNIGHT));
	BOARD_SET(board, 63, MAKE_PIECE(SIDE_BLACK, TYPE_ROOK));
}

/* Example: rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1 */
int board_setup_fen(Board *board, const char *fen, const char **pEnd)
{
	move_t row, col;
	piece_t piece;
	char up;
	uint32_t halfTurns, fullTurns;

	memset(board, 0, sizeof(*board));

	row = BOARD_HEIGHT - 1;
	col = 0;
	while (*fen != ' ' && *fen != '\0') {
		switch (*fen) {
		case '/':
			row--;
			col = 0;
			fen++;
			break;
		case 'p':
		case 'n':
		case 'b':
		case 'r':
		case 'q':
		case 'k':

		case 'P':
		case 'N':
		case 'B':
		case 'R':
		case 'Q':
		case 'K':
			up = toupper(*fen);
			piece = MAKE_PIECE(up != *fen, CHAR_TO_TYPE(up));
			BOARD_SET(board, row * BOARD_WIDTH + col, piece);
		/* fall through */
		case '1':
			col++;
			fen++;
			break;
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			col += *fen - '0';
			fen++;
			break;
		default:
			goto end;
		}
		if (row == 0 && col == BOARD_WIDTH)
			break;
	}

	if (*fen != ' ')
		goto end;
	fen++;

	if (*fen != 'b' && *fen != 'w')
		goto end;
	board->flags = *fen == 'w' ? SIDE_WHITE : SIDE_BLACK;
	fen++;

	if (*fen != ' ')
		goto end;
	fen++;

	for (; *fen != ' ' && *fen != '\0'; fen++) {
		if (*fen == '-') {
			fen++;
			break;
		}
		switch (*fen) {
		case 'K':
			board->flags |= CASTLE_SHORT_WHITE;
			break;
		case 'Q':
			board->flags |= CASTLE_LONG_WHITE;
			break;
		case 'k':
			board->flags |= CASTLE_SHORT_BLACK;
			break;
		case 'q':
			board->flags |= CASTLE_LONG_BLACK;
			break;
		}
	}

	if (*fen != ' ')
		goto end;
	fen++;

	if (*fen != '-') {
		move_t sq;

		sq = 0;
		if (*fen < 'a' || *fen > 'h')
			goto end;
		sq += *fen - 'a';
		fen++;
		if (*fen < '1' || *fen > '8')
			goto end;
		sq += (*fen - '1') * BOARD_WIDTH;
		fen++;
		board->flags |= sq << EN_PASSANT_SHIFT;
	} else {
		fen++;
	}

	if (*fen != ' ')
		goto end;
	fen++;

	if (!isdigit(*fen))
		goto end;
	halfTurns = *fen - '0';
	for (fen++; isdigit(*fen); fen++) {
		halfTurns *= 10;
		halfTurns += *fen - '0';
	}
	board->flags |= halfTurns << HALF_TURN_SHIFT;

	if (*fen != ' ')
		goto end;
	fen++;

	if (!isdigit(*fen))
		goto end;
	fullTurns = *fen - '0';
	for (fen++; isdigit(*fen); fen++) {
		fullTurns *= 10;
		fullTurns += *fen - '0';
	}
	board->flags |= fullTurns << HALF_TURN_SHIFT;

	if (*fen != '\0')
		goto end;

	return 0;

end:
	if (pEnd != NULL)
		*pEnd = fen;
	return 1;
}

int board_play_move(Board *board, move_t move)
{
	move_t from, to;
	piece_t promot;
	piece_t piece;

	if (move & MOVE_CONFUSED)
		/* only validated moves are allowed in here */
		return -1;

	if (move & MOVE_CASTLE) {
		const move_t off = TURN(board) == SIDE_WHITE ? 0 :
			(BOARD_HEIGHT - 1) * BOARD_WIDTH;
		piece = MAKE_PIECE(TURN(board), TYPE_KING);
		const piece_t rook = MAKE_PIECE(TURN(board), TYPE_ROOK);
		if (move & MOVE_CASTLE_SHORT) {
			BOARD_SET(board, off + 6, piece);
			BOARD_SET(board, off + 7, 0);
			BOARD_SET(board, off + 5, rook);
		} else {
			BOARD_SET(board, off + 2, piece);
			BOARD_SET(board, off + 3, 0);
			BOARD_SET(board, off + 0, rook);
		}
		BOARD_SET(board, off + 4, 0);
	} else {
		from = MOVE_FROM(move);
		to = MOVE_TO(move);
		promot = MOVE_PROMOTION(move);

		piece = BOARD_GET(board, from);
		if (promot != 0)
			piece = TYPE_SET(piece, promot);
		BOARD_SET(board, to, piece);
		BOARD_SET(board, from, 0);

		const move_t enp = EN_PASSANT(board);
		if (TYPE(piece) == TYPE_PAWN && enp != 0 && to == enp) {
			move_t thePawn; /* the pawn that moved two squares */

			const int dir = DIRECTION(TURN(board));
			thePawn = to / BOARD_WIDTH;
			thePawn -= dir;
			thePawn *= BOARD_WIDTH;
			thePawn += to % BOARD_WIDTH;
			BOARD_SET(board, thePawn, 0);
		}
	}

	/* remove en passant */
	board->flags &= ~(EN_PASSANT_MASK << EN_PASSANT_SHIFT);

	switch (TYPE(piece)) {
	case TYPE_PAWN: {
		/* set en passant if a pawn moved by two squares */
		move_t absDelta;

		if (to > from)
			absDelta = to - from;
		else
			absDelta = from - to;
		if (absDelta == 2 * BOARD_WIDTH) {
			const int dir = DIRECTION(TURN(board));
			board->flags |= (from + dir * BOARD_WIDTH) <<
				EN_PASSANT_SHIFT;
		}
		break;
	}
	case TYPE_ROOK: {
		/* remove castle rights for this side */
		const move_t row = TURN(board) == SIDE_WHITE ? 0 :
			BOARD_HEIGHT - 1;
		if (from == row * BOARD_WIDTH)
			/* *the* bit trick */
			board->flags &= ~(CASTLE_LONG_WHITE << TURN(board));
		else if (from == row * BOARD_WIDTH + BOARD_WIDTH - 1)
			board->flags &= ~(CASTLE_SHORT_WHITE << TURN(board));
		break;
	}
	case TYPE_KING:
		board->flags &= ~((CASTLE_SHORT_WHITE | CASTLE_LONG_WHITE)
			<< TURN(board));
		break;
	}
	if (TYPE(piece) != TYPE_PAWN && !(move & MOVE_TAKES))
		SET_HALF_TURN(board, HALF_TURN(board) + 1);
	else
		SET_HALF_TURN(board, 0);
	if (TURN(board) == SIDE_BLACK)
		SET_FULL_TURN(board, FULL_TURN(board) + 1);
	SWITCH_TURN(board);
	return 0;
}

int board_play_moves(Board *board, const MoveList *moves)
{
	move_t move;

	for (size_t i = 0; i < moves->numMoves; i++) {
		move = moves->moves[i];
		if (move_validate(&move, board) < 0)
			return -1;
		board_play_move(board, move);
	}
	return 0;
}

int board_unplay_move(Board *board, move_t move)
{
	(void) board;
	(void) move; /* TODO: */
	return 0;
}

void board_neat_output(const Board *board, FILE *fp)
{
	for (move_t row = 0; row < BOARD_HEIGHT; row++) {
		fprintf(fp, "+---+---+---+---+---+---+---+---+\n");
		fprintf(fp, "|");
		for (move_t col = 0; col < BOARD_WIDTH; col++) {
			const move_t sq = (BOARD_HEIGHT - 1 - row) *
				BOARD_WIDTH + col;
			const piece_t piece = BOARD_GET(board, sq);

			if (col > 0)
				fprintf(fp, "|");

			if (TYPE(piece) == 0) {
				fprintf(fp, "   ");
			} else {
				char ch;

				ch = TYPE_TO_CHAR(TYPE(piece));
				if (SIDE(piece) == SIDE_BLACK)
					ch = tolower(ch);
				fprintf(fp, " %c ", ch);
			}
		}
		fprintf(fp, "| %d\n", 8 - row);
	}
	fprintf(fp, "+---+---+---+---+---+---+---+---+\n");
	fprintf(fp, "  a   b   c   d   e   f   g   h  \n");
}

static size_t write_int(char *str, uint32_t i)
{
	size_t len = 0;
	size_t st, en;

	if (i == 0) {
		*str = '0';
		return 1;
	}

	for (; i > 0; i /= 10)
		str[len++] = '0' + i % 10;
	st = 0;
	en = len;
	while (st + 1 < en) {
		const char tmp = str[st];
		str[st] = str[en];
		str[en] = tmp;
		st++;
		en--;
	}
	return en;
}

const char *board_to_fen(const Board *board)
{
	static char fen[128];
	size_t index = 0;
	move_t empty = 0;
	char ch;

	for (move_t row = 0; row < BOARD_HEIGHT; row++) {
		if (row > 0)
			fen[index++] = '/';
		for (move_t col = 0; col < BOARD_WIDTH; col++) {
			const move_t sq = row * BOARD_WIDTH + col;
			const piece_t piece = BOARD_GET(board, sq);
			if (TYPE(piece) == 0) {
				empty++;
			} else {
				if (empty > 0) {
					index += write_int(&fen[index], empty);
					empty = 0;
				}
				ch = TYPE_TO_CHAR(TYPE(piece));
				if (SIDE(piece) == SIDE_BLACK)
					ch = tolower(ch);
				fen[index++] = ch;
			}
		}
		if (empty > 0) {
			index += write_int(&fen[index], empty);
			empty = 0;
		}
	}

	fen[index++] = ' ';
	fen[index++] = tolower(SIDE_TO_CHAR(TURN(board)));

	fen[index++] = ' ';
	if (board->flags & CASTLE_SHORT_WHITE)
		fen[index++] = 'W';
	if (board->flags & CASTLE_LONG_WHITE)
		fen[index++] = 'Q';
	if (board->flags & CASTLE_SHORT_BLACK)
		fen[index++] = 'w';
	if (board->flags & CASTLE_LONG_BLACK)
		fen[index++] = 'q';
	if (!(board->flags & (CASTLE_SHORT_WHITE | CASTLE_LONG_WHITE |
				CASTLE_SHORT_BLACK | CASTLE_LONG_BLACK)))
		fen[index++] = '-';

	fen[index++] = ' ';
	const move_t enp = EN_PASSANT(board);
	if (enp == 0) {
		fen[index++] = '-';
	} else {
		fen[index++] = 'a' + enp % BOARD_WIDTH;
		fen[index++] = '1' + enp / BOARD_WIDTH;
	}

	fen[index++] = ' ';
	index += write_int(&fen[index], HALF_TURN(board));

	fen[index++] = ' ';
	index += write_int(&fen[index], FULL_TURN(board));

	fen[index] = '\0';
	return fen;
}
