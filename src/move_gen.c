#include "shess.h"

struct cache_board {
	Board board;
	MoveList *moves;
} *cached_boards;
size_t num_cached_boards;

static inline int pawn_add_move(MoveList *list, move_t move)
{
	const move_t to = MOVE_TO(move);
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

static int pawn_generate_moves(Board *board, move_t from, MoveList *list)
{
	move_t to;
	piece_t target;

	const move_t enp = EN_PASSANT(board);
	const piece_t pawn = BOARD_GET(board, from);
	const piece_t side = SIDE(pawn);
	const int dir = DIRECTION(side);
	to = from + dir * BOARD_WIDTH;
	target = BOARD_GET(board, to);
	if (target == 0) {
		if (pawn_add_move(list, MAKE_MOVE(from, to) | pawn) < 0)
			return -1;

		/* double move if on home row */
		const move_t home = side == SIDE_WHITE ? 1 : BOARD_HEIGHT - 2;
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

static int knight_generate_moves(Board *board, move_t from, MoveList *list)
{
	(void) board;
	(void) from;
	(void) list; /* TODO: */
	return 0;
}

static int bishop_generate_moves(Board *board, move_t from, MoveList *list)
{
	(void) board;
	(void) from;
	(void) list; /* TODO: */
	return 0;
}

static int rook_generate_moves(Board *board, move_t from, MoveList *list)
{
	(void) board;
	(void) from;
	(void) list; /* TODO: */
	return 0;
}

static int queen_generate_moves(Board *board, move_t from, MoveList *list)
{
	(void) board;
	(void) from;
	(void) list; /* TODO: */
	return 0;
}

static int king_generate_moves(Board *board, move_t from, MoveList *list)
{
	(void) board;
	(void) from;
	(void) list; /* TODO: */
	return 0;
}

MoveList *board_generate_moves(Board *board)
{
	static int (*const generators[])(Board *board, move_t index, MoveList *list) = {
		[TYPE_PAWN] = pawn_generate_moves,
		[TYPE_KNIGHT] = knight_generate_moves,
		[TYPE_BISHOP] = bishop_generate_moves,
		[TYPE_ROOK] = rook_generate_moves,
		[TYPE_QUEEN] = queen_generate_moves,
		[TYPE_KING] = king_generate_moves,
	};
	MoveList *list;
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
	for (move_t i = 0; i < BOARD_WIDTH * BOARD_HEIGHT; i++) {
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
