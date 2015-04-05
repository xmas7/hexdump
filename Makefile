CFLAGS = -Wall -Wextra -g -std=gnu99

ALL = hexdump

all: $(ALL)

.PHONY: clean
clean:
	rm -f $(ALL)
