#include <ctype.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

void print_offset(size_t position)
{
    printf("%08lx  ", position);
}

void print_hex(char *buffer, size_t buffer_size, size_t line_width)
{
    for (size_t j = 0; j < buffer_size; ++ j) {
        printf ("%02hhx ", buffer[j]);
        if (j == line_width / 2 - 1) {
            printf (" ");
        }
    }
}

void align_delimiter(size_t line_size_current, size_t line_size_full)
{
    for (size_t j = line_size_current ; j < line_size_full; ++ j) {
        printf ("   ");
    }

    if (line_size_current < line_size_full / 2) {
        printf (" ");
    }

    printf (" |");
}

void print_chars(char *buffer, size_t buffer_size)
{
    for (size_t j = 0; j < buffer_size; ++ j) {
        char c = buffer[j];
        if (isprint(c)) {
            printf ("%c", c);
        } else {
            printf (".");
        }
    }
}

void print_contents(char *buffer, size_t buffer_size)
{
    if (buffer_size == 0) {
        return;
    }

    enum { LINE_WIDTH_ELEMENTS = 16 };
    size_t num_lines = buffer_size / LINE_WIDTH_ELEMENTS + 1;
    size_t remaining_buffer_size = buffer_size;
    for (size_t i = 0; i < num_lines; ++ i) {
        size_t line_size =
            remaining_buffer_size > LINE_WIDTH_ELEMENTS
          ? LINE_WIDTH_ELEMENTS
          : remaining_buffer_size;

        print_offset (i * LINE_WIDTH_ELEMENTS);

        print_hex (
            &buffer[i * LINE_WIDTH_ELEMENTS],
            line_size,
            LINE_WIDTH_ELEMENTS);

        align_delimiter (line_size, LINE_WIDTH_ELEMENTS);

        print_chars (&buffer[i * LINE_WIDTH_ELEMENTS], line_size);

        printf ("|\n");
        remaining_buffer_size -= line_size;
    }
    print_offset (buffer_size);
    printf ("\n");
}

int read_print_file(const char *path)
{
    int result = 1;
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        fprintf (stderr, "Couldn't open file: %s\n", strerror (errno));
        goto Return;
    }

    while (1) {
        char buffer[BUFSIZ];
        ssize_t count = read (fd, buffer, sizeof (buffer) / sizeof (buffer[0]));
        if (count == -1) {
            fprintf (stderr, "Couldn't read file: %s\n", strerror (errno));
            goto Close;
        }

        print_contents (buffer, count);

        if (count < BUFSIZ) {
            break;
        }
    }

    result = 0;

Close:
    close(fd);
Return:
    return result;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: hexdump <file>\n");
        return 1;
    }
    if (read_print_file (argv[1])) {
        return 1;
    }
    return 0;
}
