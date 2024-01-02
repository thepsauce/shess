#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8

#define BIT_MASK(nbits) ((1<<(nbits))-1)

/**
 * Piece composition
 **/
typedef uint8_t piece_t;

/* side of the piece */
#define SIDE_SHIFT	0
#define SIDE_MASK	BIT_MASK(1)

#define SIDE_WHITE	0x0
#define SIDE_BLACK	0x1

#define SIDE(piece) (((piece)>>SIDE_SHIFT)&SIDE_MASK)

/* type of the piece */
#define TYPE_SHIFT	1
#define TYPE_MASK	BIT_MASK(3)

#define TYPE_PAWN	0x1
#define TYPE_KNIGHT	0x2
#define TYPE_BISHOP	0x3
#define TYPE_ROOK	0x4
#define TYPE_QUEEN	0x5
#define TYPE_KING	0x6

#define TYPE(piece) (((piece)>>TYPE_SHIFT)&TYPE_MASK)

#define MAKE_PIECE(side, type) (((side)<<SIDE_SHIFT)|((type)<<TYPE_SHIFT))

#define SIDE_TO_CHAR(side) ("WB"[side])
#define TYPE_TO_CHAR(type) ("?PNBRQK"[type])

#define CHAR_TO_SIDE(side) ((side)=='W'?SIDE_WHITE:SIDE_BLACK)
#define CHAR_TO_TYPE(type) ({ \
	const char _t = (type); \
	_t == 'P' ? TYPE_PAWN : _t == 'N' ? TYPE_KNIGHT : \
	_t == 'B' ? TYPE_BISHOP : _t == 'R' ? TYPE_ROOK : \
	_t == 'Q' ? TYPE_QUEEN : _t == 'K' ? TYPE_KING : '?'; \
})

/**
 * Move composition (b stands for bit/bits):
 *   1b    3b    7b   7b      3b         1b            1b        1b
 * [side][type][from][to][promotion][castle short][castle long][takes]
 *         1b             1b           3b
 * [confused row][confused column][end condition]
 */

typedef uint32_t move_t;

#define MOVE_SIDE_SHIFT 0

#define MOVE_SIDE(move) (((move)>>MOVE_SIDE_SHIFT)&SIDE_MASK)

#define MOVE_TYPE_SHIFT 1

#define MOVE_TYPE(move) (((move)>>MOVE_TYPE_SHIFT)&TYPE_MASK)

#define MOVE_FROM_SHIFT 4

#define MOVE_FROM(move) (((move)>>MOVE_FROM_SHIFT)&BIT_MASK(7))

#define MOVE_TO_SHIFT 12

#define MOVE_TO(move) (((move)>>MOVE_TO_SHIFT)&BIT_MASK(7))

#define MOVE_PROMOTION_SHIFT 18

#define MOVE_PROMOTION(move) (((move)>>MOVE_PROMOTION_SHIFT)&TYPE_MASK)

#define MOVE_CASTLE_SHORT	0x00200000
#define MOVE_CASTLE_LONG	0x00400000

#define MOVE_TAKES		0x00800000

#define MOVE_CONFUSED_ROW	0x01000000
#define MOVE_CONFUSED_COL	0x02000000
#define MOVE_CONFUSED (MOVE_CONFUSED_ROW|MOVE_CONFUSED_COL)
#define MOVE_IS_CONFUSED	0x04000000

#define MOVE_CHECK		0x08000000
#define MOVE_CHECKMATE		0x10000000

#define MOVE_RESIGNATION	0x1
#define MOVE_STALEMATE		0x2
#define MOVE_DRAW		0x3

#define MOVE_END_CONDITION_SHIFT 29
#define MOVE_END_CONDITION_MASK BIT_MASK(2)

#define MOVE_END_CONDITION(move) (((move)>>MOVE_END_CONDITION_SHIFT)&MOVE_END_CONDITION_MASK)

/**
 * Flags
 **/
/* first bit indicates the playing side */

/* castling */
#define CASTLE_SHORT_WHITE	0x0002
#define CASTLE_LONG_WHITE	0x0004
#define CASTLE_SHORT_BLACK	0x0008
#define CASTLE_LONG_BLACK	0x0010

/* whether a side is in check */
#define CHECK_WHITE		0x0020
#define CHECK_BLACK		0x0040

/* the current en passant square (0 indicates that there is none) */
#define EN_PASSANT_SHIFT	7
#define EN_PASSANT_MASK		BIT_MASK(7)

#define EN_PASSANT(flags) (((flags)>>EN_PASSANT_SHIFT)&EN_PASSANT_MASK)

#define HALF_TURN_SHIFT		14
#define HALF_TURN_MASK		BIT_MASK(7)

#define HALF_TURN(flags) (((flags)>>HALF_TURN_SHIFT)&HALF_TURN_MASK)

#define FULL_TURN_SHIFT		21
#define FULL_TURN_MASK		BIT_MASK(11)

#define FULL_TURN(flags) (((flags)>>FULL_TURN_SHIFT)&FULL_TURN_MASK)

typedef struct board {
	uint32_t flags;
	piece_t data[BOARD_WIDTH * BOARD_HEIGHT];
} Board;

#define BOARD_GET(board, at) ({ \
	const Board *const _b = (board); \
	const uint32_t _at = (at); \
	_b->data[_at]; \
})

#define BOARD_SET(board, at, piece) ({ \
	Board *const _b = (board); \
	const uint32_t _at = (at); \
	const piece_t _p = (piece); \
	_b->data[_at] = _p; \
})

void board_setup_starting(Board *board);
int board_setup_fen(Board *board, const char *fen, const char **pEnd);
void board_play_move(Board *board, const move_t *move);
void board_play_moves(Board *board, const move_t *moves, size_t numMoves);
void board_neat_output(const Board *board, FILE *fp);
