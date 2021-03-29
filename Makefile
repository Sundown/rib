CFLAGS += -Wall -W -pedantic -march=native -std=c11 -lm -ofast
SRC = src/gen.c src/lex.c src/main.c src/util.c src/parse.c
TARGET = rib
.PHONY: all clean

rib:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	$(RM) $(TARGET)
