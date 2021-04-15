CFLAGS += -Wall -W -pedantic -march=native -std=c11 -lm -ofast `llvm-config --cflags`
LDFLAGS += `llvm-config --ldflags --libs core executionengine analysis native bitwriter --system-libs`
SRC =  $(shell find ./src -name "*.c")
CC=clang++

TARGET = rib
.PHONY: rib clean

rib:
	$(CC) $(CFLAGS) -x c $(SRC) $(LDFLAGS) -o $(TARGET)

run: rib
	./rib

clean:
	$(RM) -r $(TARGET) output/
