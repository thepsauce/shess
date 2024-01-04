#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ARRLEN(a) (sizeof(a)/sizeof*(a))

#include "board.h"

typedef struct game_context {
	char *id;
	char *value;
} GameContext;

typedef struct game_data {
	GameContext *context;
	size_t numContext;
	MoveList moves;
} GameData;

int gamedata_input(GameData *data, FILE *fp);
int gamedata_output(GameData *data, FILE *fp);

typedef struct game {
	GameData data;
	Board board;
} Game;
