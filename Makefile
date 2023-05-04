CC=clang
CFLAGS=-Wall -Wextra -Wstrict-prototypes -Werror -pedantic -Wno-unused-function
LDFLAGS=-fsanitize=address,undefined,leak
OBJS=example.o

.PHONY: clean format debug

all: example

example: $(OBJS)
	$(CC) -o example $(OBJS)

%.o: %.c region_allocator.h
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS+=-g \
	-fsanitize=address,undefined,leak \
	-fno-omit-frame-pointer \
	-fno-common \
	-Wnull-dereference \
	-Wwrite-strings \
	-Wswitch-enum

debug: $(OBJS) example-debug
	@# run the program
	ASAN_OPTIONS=debug=true:detect_stack_use_after_return=true:detect_invalid_pointer_pairs=1 ./example

example-debug: $(OBJS)
	$(CC) $(LDFLAGS) -o example $(OBJS)

clean:
	rm -f example $(OBJS)

format:
	clang-format -i -style=file *.[ch]
