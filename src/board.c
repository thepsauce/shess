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
	size_t sq = 0;
	piece_t piece;
	char up;
	uint32_t halfTurns, fullTurns;

	memset(board, 0, sizeof(*board));

	for (; *fen != ' ' && *fen != '\0'; fen++) {
		if (sq >= BOARD_WIDTH * BOARD_HEIGHT)
			goto end;
		switch (*fen) {
		case '/':
			sq /= BOARD_WIDTH;
			sq *= BOARD_WIDTH;
			sq += BOARD_WIDTH;
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
		case 'Q':
		case 'K':
			up = toupper(*fen);
			piece = MAKE_PIECE(up != *fen, CHAR_TO_TYPE(up));
			BOARD_SET(board, sq, piece);
		/* fall through */
		case '1':
			sq++;
			break;
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			sq += *fen - '0';
			break;
		default:
			goto end;
		}
	}

	if (*fen != ' ')
		goto end;
	fen++;

	if (*fen != 'b' && *fen != 'w')
		goto end;
	board->flags = *fen == 'w' ? SIDE_WHITE : SIDE_BLACK;

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
	while (isdigit(*fen)) {
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
	while (isdigit(*fen)) {
		fullTurns *= 10;
		fullTurns += *fen - '0';
	}
	board->flags |= fullTurns << HALF_TURN_SHIFT;

	if (*fen != '\0')
		goto end;

	return 0;

end:
	*pEnd = fen;
	return 1;
}

void board_play_move(Board *board, const move_t *move)
{
	(void) board;
	(void) move; /* TODO: */
}

void board_play_moves(Board *board, const move_t *moves, size_t numMoves)
{
	(void) board;
	(void) moves;
	(void) numMoves; /* TODO: */
}

void board_neat_output(const Board *board, FILE *fp)
{
	for (int8_t row = 0; row < BOARD_HEIGHT; row++) {
		fprintf(fp, "+---+---+---+---+---+---+---+---+\n");
		fprintf(fp, "|");
		for (int8_t col = 0; col < BOARD_WIDTH; col++) {
			const int32_t sq = (7 - row) * BOARD_WIDTH + col;
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
