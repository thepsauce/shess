#include "shess.h"

int game_init(Game *game)
{
	History *hist;
	Node *node;

	memset(game, 0, sizeof(*game));
	hist = &game->data.history;
	node = malloc(sizeof(*node));
	if (node == NULL)
		return -1;
	memset(node, 0, sizeof(*node));
	hist->first = node;
	hist->cur = node;
	return 0;
}

void game_uninit(Game *game)
{
	History *hist;
	Node *node, *parent;

	hist = &game->data.history;
	node = hist->first;
	while (1) {
		while (node->numChildren > 0)
			node = node->children[node->numChildren - 1];
		parent = node->parent;
		free(node);
		node = parent;
		if (node == NULL)
			break;
		node->numChildren--;
	}
}

int history_add(History *hist, const UndoData *ud)
{
	Node *node;
	Node **newChildren;

	if (hist->cur != NULL) {
		for (size_t i = 0; i < hist->cur->numChildren; i++) {
			Node *const child = hist->cur->children[i];
			if (memcmp(&child->data, ud, sizeof(*ud)) == 0) {
				hist->cur = child;
				return 0;
			}
		}
	}
	node = malloc(sizeof(*node));
	if (node == NULL)
		return -1;
	node->data = *ud;
	node->children = NULL;
	node->numChildren = 0;
	newChildren = realloc(hist->cur->children,
			sizeof(*hist->cur->children) *
			(hist->cur->numChildren + 1));
	if (newChildren == NULL) {
		free(node);
		return -1;
	}
	hist->cur->children = newChildren;
	hist->cur->children[hist->cur->numChildren++] = node;
	node->parent = hist->cur;
	hist->cur = node;
	return 0;
}

int history_undo(History *hist, Board *board)
{
	if (hist->cur->parent == NULL)
		return 1;
	board_unplay_move(board, &hist->cur->data);
	hist->cur = hist->cur->parent;
	return 0;
}
