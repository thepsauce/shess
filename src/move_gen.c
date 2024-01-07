#include "shess.h"

struct cache_board {
	Board board;
	MoveList *moves;
} *cached_boards;
size_t num_cached_boards;

struct offset {
	offset_t rows;
	offset_t cols;
};

static const struct offset knight_offsets[] = {
	{ -2, -1 },
	{ -2,  1 },
	{  2, -1 },
	{  2,  1 },
	{ -1, -2 },
	{ -1,  2 },
	{  1, -2 },
	{  1,  2 }
};

static const struct offset bishop_offsets[] = {
	{ -1, -1 },
	{ -1,  1 },
	{  1, -1 },
	{  1,  1 },
};

static const struct offset rook_offsets[] = {
	{  0, -1 },
	{  0,  1 },
	{ -1,  0 },
	{  1,  0 }
};

static const struct offset king_offsets[] = {
	{ -1, -1 },
	{ -1,  1 },
	{  1, -1 },
	{  1,  1 },
	{  0, -1 },
	{  0,  1 },
	{ -1,  0 },
	{  1,  0 }
};

bool board_is_attacked(Board *board, pos_t sq, piece_t side)
{
	pos_t to;
	piece_t piece;
	/* 1. is a rook/queen above/below/left/right of this square
	 * 2. is a knight targetting this square
	 * 3. is a pawn attacking this square
	 * 4. is the king touching this square
	 */
	/* rook/queen not diagonal */
	for (size_t i = 0; i < ARRLEN(rook_offsets); i++) {
		const struct offset o = rook_offsets[i];
		to = sq;
		while (to = OFFSET(to, o.rows, o.cols), to != (pos_t) -1) {
			piece = BOARD_GET(board, to);
			if (piece != 0)
				break;
		}
		if (to != (pos_t) -1 && SIDE(piece) == side &&
				(TYPE(piece) == TYPE_ROOK || TYPE(piece) == TYPE_QUEEN))
			return true;
	}

	/* bishop/queen diagonal */
	for (size_t i = 0; i < ARRLEN(bishop_offsets); i++) {
		const struct offset o = bishop_offsets[i];
		to = sq;
		while (to = OFFSET(to, o.rows, o.cols), to != (pos_t) -1) {
			piece = BOARD_GET(board, to);
			if (piece != 0)
				break;
		}
		if (to != (pos_t) -1 && SIDE(piece) == side &&
				(TYPE(piece) == TYPE_BISHOP || TYPE(piece) == TYPE_QUEEN))
			return true;
	}

	/* knight */
	for (size_t i = 0; i < ARRLEN(knight_offsets); i++) {
		const struct offset o = knight_offsets[i];
		const pos_t to = OFFSET(sq, o.rows, o.cols);
		if (to == (pos_t) -1)
			continue;
		piece = BOARD_GET(board, to);
		if (MAKE_PIECE(side, TYPE_KNIGHT) == piece)
			return true;
	}

	/* pawn */
	for (offset_t i = -1; i <= 1; i += 2) {
		to = OFFSET(sq, -DIRECTION(side), i);
		if (to == (pos_t) -1)
			continue;
		piece = BOARD_GET(board, to);
		if (MAKE_PIECE(side, TYPE_PAWN) == piece)
			return true;
	}

	/* king */
	for (size_t i = 0; i < ARRLEN(king_offsets); i++) {
		const struct offset o = king_offsets[i];
		to = OFFSET(sq, o.rows, o.cols);
		if (to == (pos_t) -1)
			continue;
		piece = BOARD_GET(board, to);
		if (MAKE_PIECE(side, TYPE_KING) == piece)
			return true;
	}
	return false;
}

static inline int pawn_add_move(MoveList *list, move_t move)
{
	const pos_t to = MOVE_TO(move);
	/* if the pawn reached the end */
	if (to / BOARD_WIDTH == 0 || to / BOARD_WIDTH == BOARD_HEIGHT - 1) {
		if (movelist_add(list, move | (TYPE_KNIGHT <<
					MOVE_PROMOTION_SHIFT)) < 0)
			return -1;
		if (movelist_add(list, move | (TYPE_BISHOP <<
					MOVE_PROMOTION_SHIFT)) < 0)
			return -1;
		if (movelist_add(list, move | (TYPE_ROOK <<
					MOVE_PROMOTION_SHIFT)) < 0)
			return -1;
		if (movelist_add(list, move | (TYPE_QUEEN <<
					MOVE_PROMOTION_SHIFT)) < 0)
			return -1;
	} else {
		if (movelist_add(list, move) < 0)
			return -1;
	}
	return 0;
}

static int pawn_generate_moves(Board *board, pos_t from, MoveList *list)
{
	pos_t to;
	piece_t target;

	const pos_t enp = EN_PASSANT(board);
	const piece_t pawn = BOARD_GET(board, from);
	const piece_t side = SIDE(pawn);
	const int dir = DIRECTION(side);
	to = from + dir * BOARD_WIDTH;
	target = BOARD_GET(board, to);
	if (target == 0) {
		if (pawn_add_move(list, MAKE_MOVE(from, to) | pawn) < 0)
			return -1;

		/* double move if on home row */
		const pos_t home = side == SIDE_WHITE ? 1 : BOARD_HEIGHT - 2;
		if (from / BOARD_WIDTH == home) {
			to += dir * BOARD_WIDTH;
			target = BOARD_GET(board, to);
			if (target == 0)
				if (pawn_add_move(list, MAKE_MOVE(from, to) |
							pawn) < 0)
					return -1;
			to -= dir * BOARD_WIDTH;
		}
	}

	/* takes left */
	if (to % BOARD_WIDTH != 0) {
		to--;
		target = BOARD_GET(board, to);
		if ((target != 0 && ENEMY(target) == side) ||
					(enp != 0 && to == enp))
			if (pawn_add_move(list, MAKE_MOVE(from, to) |
						MOVE_TAKES | pawn) < 0)
				return -1;
		to++;
	}
	/* takes right */
	if ((to + 1) % BOARD_WIDTH != 0) {
		to++;
		target = BOARD_GET(board, to);
		if ((target != 0 && ENEMY(target) == side) ||
					(enp != 0 && to == enp))
			if (pawn_add_move(list, MAKE_MOVE(from, to) |
						MOVE_TAKES | pawn) < 0)
				return -1;
	}
	return 0;
}

static inline int generate_moves(Board *board, pos_t from, MoveList *list,
		const struct offset *offsets, size_t numOffsets, bool single)
{
	pos_t to;
	move_t move;

	const piece_t piece = BOARD_GET(board, from);
	for (size_t i = 0; i < numOffsets; i++) {
		const struct offset o = offsets[i];
		to = from;
		while (to = OFFSET(to, o.rows, o.cols), to != (pos_t) -1) {
			const piece_t p = BOARD_GET(board, to);
			move = MAKE_MOVE(from, to) | piece;
			if (p != 0) {
				if (SIDE(piece) == SIDE(p))
					break;
				move |= MOVE_TAKES;
			}
			if (movelist_add(list, move) < 0)
				return -1;
			if (p != 0 || single)
				break;
		}
	}
	return 0;
}

static int knight_generate_moves(Board *board, pos_t from, MoveList *list)
{
	return generate_moves(board, from, list,
			knight_offsets, ARRLEN(knight_offsets), true);
}

static int bishop_generate_moves(Board *board, pos_t from, MoveList *list)
{
	return generate_moves(board, from, list,
			bishop_offsets, ARRLEN(bishop_offsets), false);
}

static int rook_generate_moves(Board *board, pos_t from, MoveList *list)
{
	return generate_moves(board, from, list,
			rook_offsets, ARRLEN(rook_offsets), false);
}

static int queen_generate_moves(Board *board, pos_t from, MoveList *list)
{
	return generate_moves(board, from, list,
			king_offsets, ARRLEN(king_offsets), false);
}

static int king_generate_moves(Board *board, pos_t from, MoveList *list)
{
	const piece_t king = BOARD_GET(board, from);
	if (!(board->flags & CHECK)) {
		if ((board->flags & (CASTLE_SHORT_WHITE << SIDE(king))) &&
				BOARD_GET(board, from + 1) == 0 &&
				BOARD_GET(board, from + 2) == 0) {
			if (!board_is_attacked(board, from + 1, ENEMY(king)))
				if (movelist_add(list, MOVE_CASTLE_SHORT) < 0)
					return -1;
		}

		if ((board->flags & (CASTLE_LONG_WHITE << SIDE(king))) &&
				BOARD_GET(board, from - 1) == 0 &&
				BOARD_GET(board, from - 2) == 0 &&
				BOARD_GET(board, from - 3) == 0) {
			if (!board_is_attacked(board, from - 1, ENEMY(king)))
				if (movelist_add(list, MOVE_CASTLE_LONG) < 0)
					return -1;
		}
	}
	return generate_moves(board, from, list,
			king_offsets, ARRLEN(king_offsets), true);
}

MoveList *board_generate_moves(Board *board)
{
	static int (*const generators[])(Board *board, pos_t index, MoveList *list) = {
		[TYPE_PAWN] = pawn_generate_moves,
		[TYPE_KNIGHT] = knight_generate_moves,
		[TYPE_BISHOP] = bishop_generate_moves,
		[TYPE_ROOK] = rook_generate_moves,
		[TYPE_QUEEN] = queen_generate_moves,
		[TYPE_KING] = king_generate_moves,
	};
	MoveList *list;
	size_t numMoves;
	move_t *moves;
	pos_t kingPos;
	piece_t king;
	struct cache_board *newCachedBoards;

	/* check if the board is cached */
	/* TODO: implement a hash table */
	for (size_t i = 0; i < num_cached_boards; i++)
		if (memcmp(&cached_boards[i].board, board, sizeof(*board)) == 0)
			return cached_boards[i].moves;

	list = malloc(sizeof(*list));
	if (list == NULL)
		return NULL;
	memset(list, 0, sizeof(*list));
	for (pos_t i = 0; i < BOARD_WIDTH * BOARD_HEIGHT; i++) {
		const piece_t piece = BOARD_GET(board, i);
		if (TURN(board) != SIDE(piece))
			continue;
		if (TYPE(piece) != 0)
			if (generators[TYPE(piece)](board, i, list) < 0) {
				free(list->moves);
				free(list);
				return NULL;
			}
	}
	/* sanitize the moves */
	/* TODO: use the additional free bits in the board
	 * to store the number of attacks on a square and
	 * dynamically update that when moving
	 */

	/* find the king */
	king = MAKE_PIECE(TURN(board), TYPE_KING);
	for (pos_t i = 0; i < BOARD_WIDTH * BOARD_HEIGHT; i++)
		if (BOARD_GET(board, i) == king) {
			kingPos = i;
			break;
		}
	for (numMoves = list->numMoves, moves = list->moves; numMoves != 0;
			moves++, numMoves--) {
		UndoData ud;

		const move_t move = *moves;
		if (MOVE_TYPE(move) == TYPE_KING) {
			if (board_is_attacked(board, MOVE_TO(move), ENEMY(king))) {
				memmove(moves, moves + 1, sizeof(*moves) * numMoves);
				list->numMoves--;
				moves--;
			}
			continue;
		}
		board_play_move(board, move, &ud);
		if (board_is_attacked(board, kingPos, ENEMY(king))) {
			memmove(moves, moves + 1, sizeof(*moves) * numMoves);
			list->numMoves--;
			moves--;
		}
		board_unplay_move(board, &ud);
	}

	newCachedBoards = realloc(cached_boards, sizeof(*cached_boards) *
			(num_cached_boards + 1));
	if (newCachedBoards == NULL)  {
		free(list->moves);
		free(list);
		return NULL;
	}
	cached_boards = newCachedBoards;
	cached_boards[num_cached_boards++] = (struct cache_board) {
		*board, list
	};
	return list;
}
