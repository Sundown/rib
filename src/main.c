#include "rib.h"

int main(int argc, char** argv) {
	return argc > 1 ? rib_load_file(argv[1]),
	       0	: fprintf(stderr, "rib: error, need a filename\n"), 1;
}
