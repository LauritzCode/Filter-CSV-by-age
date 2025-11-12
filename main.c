#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX 100

char *ifile, *ofile;
unsigned filter_age_max;
FILE *istream, *ostream;

static void print_usage(const char *prog) {
  fprintf(stderr,
          "Filters CSV rows, keeping only those with provided maximum age\n%s "
          "max-age [input-file] [output-file]\nExample:\n%s 18 input.csv "
          "output.csv\n%s input.csv (output to stdout)\n%s 18 (input from "
          "stdin, output to stdout)\n",
          prog, prog, prog,
          prog); // Helper output if wrong arguments are written.
}

static void filter_stream(FILE *in, FILE *out) {
  size_t kept = 0, skipped = 0;
  size_t line_number = 0;
  char line[LINE_MAX];

  while (fgets(line, sizeof(line), in)) {
    line_number++;
    char parsebuffer[LINE_MAX]; // buffer for line to be copied in, so we can
                                // safely modify the data.
    char *p = parsebuffer;      // pointer to buffer.

    strncpy(parsebuffer, line, sizeof(parsebuffer));
    parsebuffer[sizeof parsebuffer - 1] =
        '\0'; // Copies each line to the buffer and skips \n and replaces it
              // with the null terminator.

    size_t n = strlen(parsebuffer);

    // removes new line and carriage return from the end of the line if any are
    // found.

    if (n && (parsebuffer[n - 1] == '\n' || parsebuffer[n - 1] == '\r'))
      parsebuffer[--n] = '\0';
    if (n && (parsebuffer[n - 1] == '\r'))
      parsebuffer[--n] = '\0';

    // skips whitespace or horizontal tab escape sequence and moves the pointer
    // forward

    while (*p == ' ' || *p == '\t')
      p++;
    char *name = p;
    if (*p == '\0') {
      // if all the white space in the line are skipped, and the first thing we
      // run into is a null terminator, we clearly have an empty line, thus:
      fprintf(stderr, "Line %zu: Empty line -> skipped.\n", line_number);
      skipped++;
      continue;
    }

    if (*p == '#') {
      fprintf(stderr, "Line %zu: Comment detected -> skipped.\n", line_number);
      skipped++;
      continue;
    }

    if (*p == '/' && *(p + 1) == '/') {
      fprintf(stderr, "Line %zu: Comment detected -> skipped.\n", line_number);
      skipped++;
      continue;
    }

    char *comma = strchr(parsebuffer, ',');

    if (!comma) {
      fprintf(stderr, "Line %zu: No comma -> skipped: %s\n", line_number, line);
      skipped++;
      continue;
    }
    *comma = '\0';

    // declare a pointer to the age, which is found after the comma. Sequence to
    // skip whitespace or carriage comes later:
    char *age_str = comma + 1;

    size_t m = strlen(name);

    while (m && name[m - 1] == ' ' || name[m - 1] == '\t') {
      name[--m] = '\0';
    }

    while (*age_str == ' ' || *age_str == '\r')
      age_str++;

    unsigned age;

    // Same as missing line. If we keep skipping whitespace after the comma,
    // and no age is detected, and we run into the null terminator, age is
    // missing.

    if (*age_str == '\0') {
      fprintf(stderr, "Line %zu: Age missing -> skipped: %s\n", line_number,
              line);
      skipped++;
      continue;
    }

    // if sscanf returns anything other than one, an error was detected reading
    // the age as an unsigned, thus the age is malformed.
    if (sscanf(age_str, "%u", &age) != 1) {
      fprintf(stderr, "Line %zu: Bad age -> skipped: %s\n", line_number, line);
      skipped++;
      continue;
    }

    if (age <= filter_age_max) {
      fprintf(out, "%s, %u\n", name, age);
      kept++;
    } else {
      skipped++;
    }
  }

  // Good to have as this checks if a real input/output error happened
  // while reading the file and will actually print a clear system error message
  // instead of failing silently.
  if (ferror(in)) {
    perror("I/O error while reading\n");
  }

  fprintf(stderr, "Done. Kept %zu line(s), skipped %zu.\n", kept, skipped);
}

int main(int argc, char *argv[]) {
  ifile = NULL;
  ofile = NULL;
  istream = NULL;
  ostream = NULL;

  switch (argc) {
  case 4:
    ofile = argv[3];
  case 3:
    ifile = argv[2];
  case 2:

  {
    unsigned tmp = 0;
    if (sscanf(argv[1], "%u", &tmp) != 1) {
      fprintf(stderr, "First argument is not a valid number.\n");
      print_usage(argv[0]);
      return EXIT_FAILURE;
    }
    filter_age_max = tmp;
  } break;
  default:
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  if (ifile) {
    istream = fopen(ifile, "r");
    if (!istream) {
      fprintf(stderr, "Failed to open input file: %s\n", ifile);
      return EXIT_FAILURE;
    }
  } else {
    istream = stdin; // no file provided, so move on to stdin //
  }

  if (ofile) {
    ostream = fopen(ofile, "w");
    if (!ostream) {
      fprintf(stderr, "Failed to open output file: %s", ofile);
      if (istream && istream != stdin)
        fclose(istream);
      return EXIT_FAILURE;
    }
  } else {
    ostream = stdout; // no output file provided, so move on to stdout //
  }

  filter_stream(istream, ostream);

  if (istream && istream != stdin)
    fclose(istream);
  if (ostream && ostream != stdout)
    fclose(ostream);

  return EXIT_SUCCESS;
}