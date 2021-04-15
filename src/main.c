#include "rib.h"

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "rib: error: no input files\n");
		exit(1);
	} else {
		rib_load_file(argv[1]);
	}

	return 0;
}
