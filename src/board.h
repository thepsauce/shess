#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8

typedef int32_t offset_t;
typedef uint32_t pos_t;

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

#define SIDE_ENEMY(side) (((side)^SIDE_MASK))
#define ENEMY(piece) SIDE_ENEMY(SIDE(piece))

#define DIRECTION(side) ((side)==SIDE_WHITE?1:-1)

/* type of the piece */
#define TYPE_SHIFT	1
#define TYPE_MASK	BIT_MASK(3)

#define TYPE_PAWN	0x1
#define TYPE_KNIGHT	0x2
#define TYPE_BISHOP	0x3
#define TYPE_ROOK	0x4
#define TYPE_QUEEN	0x5
#define TYPE_KING	0x6

#define TYPE_SET(piece, type) (((piece)&~(TYPE_MASK<<TYPE_SHIFT))|((type)<<TYPE_SHIFT))

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
 * Flags
 **/
/* first bit indicates the playing side */

#define TURN(board) (((board)->flags>>0)&SIDE_MASK)
#define SWITCH_TURN(board) (((board)->flags^=SIDE_MASK))

/* castling */
/* note: black must come after white because of a bit trick used
 * in src/board.c in board_play_move
 */
#define CASTLE_SHORT_WHITE	0x0002
#define CASTLE_SHORT_BLACK	0x0004
#define CASTLE_SHORT		(CASTLE_SHORT_WHITE|CASTLE_SHORT_BLACK)
#define CASTLE_LONG_WHITE	0x0008
#define CASTLE_LONG_BLACK	0x0010
#define CASTLE_LONG		(CASTLE_LONG_WHITE|CASTLE_LONG_BLACK)

/* whether a side is in check */
#define CHECK			0x0020

/* the current en passant square (0 indicates that there is none) */
#define EN_PASSANT_SHIFT	6
#define EN_PASSANT_MASK		BIT_MASK(7)

#define EN_PASSANT(board) (((board)->flags>>EN_PASSANT_SHIFT)&EN_PASSANT_MASK)

#define HALF_TURN_SHIFT		13
#define HALF_TURN_MASK		BIT_MASK(7)

#define HALF_TURN(board) (((board)->flags>>HALF_TURN_SHIFT)&HALF_TURN_MASK)
#define SET_HALF_TURN(board, ht) ((board)->flags=((board)->flags&~(HALF_TURN_MASK<<HALF_TURN_SHIFT))|((ht)<<HALF_TURN_SHIFT))

#define FULL_TURN_SHIFT		20
#define FULL_TURN_MASK		BIT_MASK(12)

#define FULL_TURN(board) (((board)->flags>>FULL_TURN_SHIFT)&FULL_TURN_MASK)
#define SET_FULL_TURN(board, ft) ((board)->flags=((board)->flags&~(FULL_TURN_MASK<<FULL_TURN_SHIFT))|((ft)<<FULL_TURN_SHIFT))

typedef struct board {
	uint32_t flags;
	piece_t data[BOARD_WIDTH * BOARD_HEIGHT];
} Board;

#define BOARD_GET(board, at) ({ \
	const Board *const _b = (board); \
	const move_t _at = (at); \
	_b->data[_at]; \
})

#define BOARD_SET(board, at, piece) ({ \
	Board *const _b = (board); \
	const move_t _at = (at); \
	const piece_t _p = (piece); \
	_b->data[_at] = _p; \
})

void board_setup_starting(Board *board);
int board_setup_fen(Board *board, const char *fen, const char **pEnd);
void board_neat_output(const Board *board, FILE *fp);
const char *board_to_fen(const Board *board);

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

#define MOVE_TO_SHIFT 11

#define MOVE_TO(move) (((move)>>MOVE_TO_SHIFT)&BIT_MASK(7))

#define MOVE_PROMOTION_SHIFT 18

#define MOVE_PROMOTION(move) (((move)>>MOVE_PROMOTION_SHIFT)&TYPE_MASK)

#define MOVE_CASTLE_SHORT	0x00200000
#define MOVE_CASTLE_LONG	0x00400000
#define MOVE_CASTLE (MOVE_CASTLE_SHORT|MOVE_CASTLE_LONG)

#define MOVE_TAKES		0x00800000

#define MOVE_CONFUSED_ROW	0x01000000
#define MOVE_CONFUSED_COL	0x02000000
#define MOVE_CONFUSED (MOVE_CONFUSED_ROW|MOVE_CONFUSED_COL)

#define MOVE_CHECK		0x04000000
#define MOVE_CHECKMATE		0x08000000

#define MOVE_RESIGNATION	0x1
#define MOVE_STALEMATE		0x2
#define MOVE_DRAW		0x3

#define MOVE_END_CONDITION_SHIFT 28
#define MOVE_END_CONDITION_MASK BIT_MASK(2)

#define MOVE_END_CONDITION(move) (((move)>>MOVE_END_CONDITION_SHIFT)&MOVE_END_CONDITION_MASK)

#define MAKE_MOVE(from, to) (((from)<<MOVE_FROM_SHIFT)|((to)<<MOVE_TO_SHIFT))

#define OFFSET(to, rows, cols) ({ \
	pos_t _result = (pos_t) -1; \
	const pos_t _to = (to); \
	const offset_t _dr = (rows); \
	const offset_t _dc = (cols); \
	const offset_t _r = _to / BOARD_WIDTH + _dr; \
	const offset_t _c = _to % BOARD_WIDTH + _dc; \
	if (_r >= 0 && _c >= 0 && _r < BOARD_HEIGHT && _c < BOARD_WIDTH) \
		_result = _r * BOARD_WIDTH + _c; \
	_result; \
})

typedef struct move_list {
	move_t *moves;
	size_t numMoves;
} MoveList;

void move_output(move_t move, FILE *fp);
int move_input(move_t *move, FILE *fp);
int move_parse(move_t *pMove, piece_t side, const char *str);
void move_getmatching(move_t move, Board *board, MoveList *matching);

int movelist_add(MoveList *list, move_t move);
bool movelist_targets(MoveList *list, pos_t to);

typedef struct undo_data {
	move_t move;
	uint32_t flags;
	piece_t capture;
} UndoData;

void board_play_move(Board *board, move_t move, UndoData *ud);
void board_unplay_move(Board *board, const UndoData *ud);
/* side is the attacking side */
bool board_is_attacked(Board *board, move_t sq, piece_t side);
MoveList *board_generate_moves(Board *board);
