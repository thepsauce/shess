#include "shess.h"

int main(int argc, char **argv)
{
	const char *file;
	FILE *fp;

	file = argv[1];
	fp = fopen(file);

	fclose(fp);
	return 0;
}
