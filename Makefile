CC = gcc -fsanitize=address -fsanitize=undefined
SW = switch

PROGS = factorial fibonacci
TEMP = $(PROGS:=.c)

all: $(PROGS)

% : %.c
	$(CC) $< -o $@

%.c : %.sw $(SW)
	./$(SW) $< $@

$(SW) : switch.c
	$(CC) switch.c -o $(SW)

clean:
	rm -f $(TEMP)
	rm -f $(PROGS)
	rm -f $(SW)

.SECONDARY: $(TEMP)
