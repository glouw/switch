CC = gcc
#CFLAGS = -Og -g -fsanitize=address -fsanitize=undefined

SW = switch
TEST = factorial fibonacci tests
TEMP = $(TEST:=.c)

all: $(TEST)

% : %.c
	$(CC) $(CFLAGS) $< -o $@
	@./$@
	@echo -e ${GREEN}OKAY${RESET} $@

%.c : %.sw $(SW)
	./$(SW) $< $@

$(SW) : $(SW).c
	$(CC) $(CFLAGS) $< -o $(SW)

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
