#include <ctype.h>

#include <errno.h>

#include <sys/mman.h>
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

void print_contents(char *buffer, size_t buffer_size, size_t offset)
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
        if (line_size == 0) {
             break;
        }

        print_offset (i * LINE_WIDTH_ELEMENTS + offset);

        print_hex (
            &buffer[i * LINE_WIDTH_ELEMENTS],
            line_size,
            LINE_WIDTH_ELEMENTS);

        align_delimiter (line_size, LINE_WIDTH_ELEMENTS);

        print_chars (&buffer[i * LINE_WIDTH_ELEMENTS], line_size);

        printf ("|\n");
        remaining_buffer_size -= line_size;
    }
}

int read_print_file(const char *path)
{
    int result = 1;
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        fprintf (stderr, "Couldn't open file: %s\n", strerror (errno));
        goto Return;
    }
    struct stat file_info = { };
    fstat (fd, &file_info);

    size_t remaining_file_size = file_info.st_size;
    enum { PAGE_SIZE = 4096 };
    size_t offset = 0;
    while (remaining_file_size > 0) {
        size_t map_size =
              remaining_file_size > PAGE_SIZE ? PAGE_SIZE : remaining_file_size;
        char *buffer = mmap (
             NULL,
             remaining_file_size,
             PROT_READ,
             MAP_PRIVATE,
             fd,
             offset);
        if (buffer == MAP_FAILED) {
            fprintf (stderr, "Couldn't read file: %s\n", strerror (errno));
            goto Close;
        }

        print_contents (buffer, map_size, offset);

        remaining_file_size -= map_size;
        offset += map_size;
    }
    print_offset (file_info.st_size);
    printf ("\n");

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
