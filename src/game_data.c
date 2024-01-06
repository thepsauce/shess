#include "shess.h"

typedef struct parser {
	FILE *fp;
	int prevc;
	int c;
	int numErrors;
	piece_t side;
	char *mem;
	size_t szMem;
	size_t numMem;
	GameContext gameContext;
	move_t move;
} Parser;

static int parser_getc(Parser *parser)
{
	parser->prevc = parser->c;
	parser->c = fgetc(parser->fp);
	return parser->c;
}

/* Puts a character in the temporary memory storage */
static int parser_put(Parser *parser, int c)
{
	char *newMem;

	if (parser->numMem + 1 > parser->szMem) {
		parser->szMem = parser->szMem * 2 + 1;
		newMem = realloc(parser->mem, parser->szMem);
		if (newMem == NULL)
			return -1;
		parser->mem = newMem;
	}
	parser->mem[parser->numMem++] = c;
	return 0;
}

static void parser_skip_space(Parser *parser)
{
	while (isspace(parser->c))
		parser_getc(parser);
}

static size_t parser_read_number(Parser *parser, size_t max)
{
	size_t num;

	/* enter this function with parser->c being a digit */
	num = parser->c - '0';
	while (isdigit(parser_getc(parser))) {
		num *= 10;
		num += parser->c - '0';
		if (num > max) {
			parser->numErrors++;
			num = max;
		}
	}
	return num;
}

/* NOTE:
 * Functions are described in their header using regex but with an extension:
 * If something is inside of {...} it means that the content can be there but an
 * error is collected, {!...} is the opposite, meaning what is inside can be
 * omitted but not without an error being collected. The error accumulator
 * is parser->numErrors.
 */

/* Reads a game context: [[ ]*id[ ]*value[ ]*{!]}
 * where id: [^ "]+
 * and value: [^ \]]* or "[^"]*"
 */
static int parser_read_game_context(Parser *parser)
{
	char *id;
	char *value;

	/* enter this function with parser->c being '[' */
	parser_skip_space(parser);

	/* read id */
	parser->numMem = 0;
	while (parser_getc(parser), !isspace(parser->c) && parser->c != '\"' &&
			parser->c != EOF) {
		if (parser_put(parser, parser->c) < 0)
			return -1;
	}
	if (parser->numMem == 0)
		return -1;
	parser_put(parser, '\0');
	id = strdup(parser->mem);
	if (id == NULL)
		return -1;

	/* read value */
	parser_skip_space(parser);
	parser->numMem = 0;
	if (parser->c == '\"') {
		/* quoted value */
		while (parser_getc(parser),
				parser->c != '\"' &&
				parser->c != EOF)
			if (parser_put(parser, parser->c) < 0)
				return -1;
		if (parser->c == '\"')
			parser_getc(parser);
	} else {
		/* unquoted value */
		while (parser->c != '[' && parser->c != ']' &&
				parser->c != EOF) {
			if (parser_put(parser, parser->c) < 0)
				return -1;
			parser_getc(parser);
		}
	}
	if (parser->numMem != 0) {
		while (isspace(parser->mem[parser->numMem - 1]))
			parser->numMem--;
		parser_put(parser, '\0');
		value = strdup(parser->mem);
		if (value == NULL)
			return -1;
	} else {
		value = NULL;
	}

	parser_skip_space(parser);
	if (parser->c != ']')
		parser->numErrors++;
	else
		parser_getc(parser);

	parser->gameContext.id = id;
	parser->gameContext.value = value;
	return 0;
}

/* Read a move, these are all possible formats:
 *           SOURCE      MOVES/TAKES      DESTINATION PROMOTION  CHECK/CHECKMATE
 * 1. [PNBRQK]?[a-h][1-8]   [-x]           [a-h][1-8] (=[PNBRQK])?    [+#]?
 * 2. ([PNBRQKa-h][a-h]?|[a-h])[1-8]?[-x]? [a-h][1-8] (=[PNBRQK])?    [+#]?
 *
 * 3. [Oo]-[Oo](-[Oo])?[+#]?
 *
 * Putting spaces anywhere is forbidden, it creates too much ambiguouity!
 * For example: Nb3d4 vs Nb3 d4
 * Is it a knight moving from b3 to d4 or a knight moving to b3 and
 * a pawn moving to d4?
 * The first notation is needed for example if there are four knights,
 * one on b3, a second one on f3, a third on b5 and a fourth on f5
 * (they all target d4).
 */
static int parser_read_move(Parser *parser)
{
	move_t move;

	move = parser->side << MOVE_SIDE_SHIFT;
	if (parser->c == 'O' || parser->c == 'o') {
		if (parser_getc(parser) != '-')
			return -1;
		parser_getc(parser);
		if (parser->c != 'O' && parser->c != 'o')
			return -1;
		if (parser_getc(parser) == '-') {
			move |= MOVE_CASTLE_LONG;
			parser_getc(parser);
			if (parser->c != 'O' && parser->c != 'o')
				return -1;
			parser_getc(parser);
		} else {
			move |= MOVE_CASTLE_SHORT;
		}
	} else {
		move_t col, row;

		switch (parser->c) {
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
			move |= CHAR_TO_TYPE(toupper(parser->c)) <<
				MOVE_TYPE_SHIFT;
			parser_getc(parser);
			break;
		}

		move |= MOVE_CONFUSED;

		if (isdigit(parser->c)) {
			if (parser->c == '0' || parser->c == '9')
				return -1;
			move ^= MOVE_CONFUSED_ROW;
			move |= ((parser->c - '1') * BOARD_WIDTH) <<
				MOVE_FROM_SHIFT;
			parser_getc(parser);
		} else if (isalpha(parser->c) && parser->c != 'x') {
			if (parser->c < 'a' || parser->c > 'h')
				return -1;
			move ^= MOVE_CONFUSED_COL;
			move |= (parser->c - 'a') << MOVE_FROM_SHIFT;
			parser_getc(parser);

			if (isdigit(parser->c)) {
				if (parser->c < '1' || parser->c > '8')
					return -1;
				move ^= MOVE_CONFUSED_ROW;
				move |= ((parser->c - '1') * BOARD_WIDTH) <<
					MOVE_FROM_SHIFT;
				parser_getc(parser);
			}
			if (parser->c != 'x' && parser->c != '-' &&
					!isalpha(parser->c)) {
				move |= MOVE_CONFUSED;
				move |= MOVE_FROM(move) << MOVE_TO_SHIFT;
				goto after;
			}
		} else if (parser->c != 'x' && parser->c != '-') {
			return -1;
		}

		if (parser->c == 'x') {
			move |= MOVE_TAKES;
			parser_getc(parser);
		}
		if (parser->c == '-')
			parser_getc(parser);

		if (parser->c < 'a' || parser->c > 'h')
			return -1;
		col = parser->c - 'a';
		parser_getc(parser);

		if (parser->c < '1' || parser->c > '8')
			return -1;
		row = parser->c - '1';
		parser_getc(parser);

		move |= (row * BOARD_WIDTH + col) << MOVE_TO_SHIFT;

	after:
		if (parser->c == '=') {
			parser_getc(parser);
			switch (parser->c) {
			case 'N':
			case 'B':
			case 'R':
			case 'Q':
			case 'K':
				move |= CHAR_TO_TYPE(parser->c) <<
					MOVE_PROMOTION_SHIFT;
				parser_getc(parser);
				break;
			default:
				return -1;
			}
		}
	}

	if (parser->c == '+' || parser->c == '#') {
		if (parser->c == '+')
			move |= MOVE_CHECK;
		else
			move |= MOVE_CHECKMATE;
		parser_getc(parser);
	}

	parser->move = move;
	return 0;
}

int gamedata_input(GameData *data, FILE *fp)
{
	Parser parser;

	GameContext *newctx;
	size_t lastmove = 0;

	memset(&parser, 0, sizeof(parser));
	parser.fp = fp;
	parser.c = fgetc(fp);
	while (parser.c != EOF) {
		parser_skip_space(&parser);

		/* comment: \{.*\} */
		if (parser.c == '{') {
			while (parser_getc(&parser), parser.c != '}' &&
					parser.c != EOF);
			continue;
		}

		/* read context */
		if (parser.c == '[') {
			if (parser_read_game_context(&parser) < 0)
				goto err;

			newctx = realloc(data->context, sizeof(*data->context) *
					(data->numContext + 1));
			if (newctx == NULL)
				goto err;
			data->context = newctx;
			data->context[data->numContext++] = parser.gameContext;
			memset(&parser.gameContext, 0, sizeof(parser.gameContext));
			continue;
		}

		/* ignore weird characters */
		if (!isalnum(parser.c)) {
			parser.numErrors++;
			continue;
		}

		/* this can either be:
		 * 1. a move number: [0-9]+({[ ]*}.)+
		 * 2. the match result: 0-1 or 1-0 or 1/2-1/2 or ½-½ or (=)
		 */
		if (isdigit(parser.c) || parser.c == 0xc2) {
			size_t move;
			size_t numdots = 0;

			move = parser_read_number(&parser, 1000);
			if (lastmove >= move)
				/* seems like we have a time traveller here */
				return -1;
			if (parser.c != '.') {
				parser.numErrors++;
				parser_skip_space(&parser);
				if (parser.c != '.')
					/* there is just a number floating
					 * around */
					return -1;
				if (parser_getc(&parser) == '.')
					numdots = 1;

			}
			/* optional: .. or [ ]+... */
			/* also possible: ([ ]*.)* (but collects an error) */
			if (parser.c == '.') {
				do {
					parser_skip_space(&parser);
					numdots++;
				} while (parser_getc(&parser) == '.');
				if (numdots != 3)
					parser.numErrors++;
				parser.side = SIDE_BLACK;
			} else {
				parser.side = SIDE_WHITE;
			}
			continue;
		}

		if (isalpha(parser.c)) {
			if (parser_read_move(&parser) < 0)
				goto err;
			if (history_add(&data->history, parser.move, 0) < 0)
				goto err;
			parser.side = SIDE_ENEMY(parser.side);
		}
	}
	return parser.numErrors;

err:
	free(parser.mem);
	free(parser.gameContext.id);
	free(parser.gameContext.value);
	return -1 - parser.numErrors;
}

int gamedata_output(GameData *data, FILE *fp)
{
	int parity = 0;
	Node *node;
	size_t move = 0;

	for (size_t i = 0; i < data->numContext; i++) {
		GameContext *const ctx = &data->context[i];
		fprintf(fp, "[%s \"%s\"]\n", ctx->id, ctx->value);
	}
	for (node = data->history.first; node != NULL; move++) {
		if (move % 2 == 1) {
			if (move > 0)
				fputc(' ', fp);
			fprintf(fp, "%zu.", move / 2 + 1 + parity);
			if (MOVE_SIDE(node->move) == SIDE_BLACK) {
				fprintf(fp, "...");
				parity ^= 1;
			}
		}
		fprintf(fp, " ");
		move_output(node->move, fp);
		if (node->numChildren == 0)
			break;
		node = node->children[0];
	}
	return 0;
}
