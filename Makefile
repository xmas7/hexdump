CFLAGS = -Wall -Wextra -g -std=c99

ALL = hexdump

all: $(ALL)

.PHONY: clean
clean:
	rm -f $(ALL)
