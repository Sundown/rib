mkdir -p bit/
make
./rib &&
llvm-dis "bit/out.bc" &&
llc "bit/out.bc"&&
rm -f "bit/out.bc" &&
clang "bit/out.s" -o "bit/out" &&
rm -f "bit/out.s"
