#include "rib.h"

int main(int argc, char** argv) {
	argc < 2 ? (void)fprintf(stderr, "rib: error: no input files\n")
		 : llvm_start(read(argv[1]));

	return 0;
}
