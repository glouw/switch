ASAN = 0

ifeq ($(ASAN),1)
CFLAGS = -g -fsanitize=address -fsanitize=undefined
else
CFLAGS = -O3 -mtune=native -march=native
endif

WFLAGS = -ansi -pedantic -Wall -Wextra -W -Wpedantic -Wshadow

CC = gcc

SW = switch
TEST = factorial fibonacci tests string putint qsort
TEMP = $(TEST:=.c)

all: $(TEST)

% : %.c
	$(CC) $(CFLAGS) $(WFLAGS) $< -o $@
	./$@
	objdump -d $@ > $*.asm
	@echo -e ${GREEN}OKAY${RESET} $@

%.c : %.sw $(SW)
	./$(SW) $< $@

$(SW) : $(SW).c
	$(CC) $(CFLAGS) $(WFLAGS) $< -o $(SW)

clean:
	rm -f $(SW)
	rm -f $(TEST)
	rm -f $(TEMP)

love:
	@echo "and war"

# Keep intermediate C files for debugging purposes
.SECONDARY: $(TEMP)

# Pretty colors
GREEN = '\033[0;32m'
RESET = '\033[0m'
