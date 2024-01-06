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

typedef struct node {
	UndoData data;
	struct node *parent;
	struct node **children;
	size_t numChildren;
} Node;

typedef struct history {
	Node *first;
	Node *cur;
} History;

int history_add(History *hist, const UndoData *ud);
int history_undo(History *hist, Board *board);

typedef struct game_data {
	GameContext *context;
	size_t numContext;
	History history;
} GameData;

int gamedata_input(GameData *data, FILE *fp);
int gamedata_output(GameData *data, FILE *fp);

typedef struct game {
	GameData data;
	Board board;
} Game;

int game_init(Game *game);
void game_uninit(Game *game);
