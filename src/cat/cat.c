#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  char b;
  char e;  // +v
  char n;
  char s;
  char t;  // +v
  char v;
} FLAGS;

extern char *optarg;

void parsing(int argc, char *args[], FLAGS *flags, int *parser_flag);
void simple_output_from_console();
void switch_parsing(int value, FLAGS *flags, int *flag_for_parsing);
void output(char args[], FLAGS flags, int *i, int *num_of_files);
void flag_v(int *symbol);

int main(int argc, char *args[]) {
  FLAGS flags = {0};
  int counter_b_n = 1, parser_flag = 1;
  parsing(argc, args, &flags, &parser_flag);
  if (argc == 1) {
    parser_flag = 0;
    simple_output_from_console();
  }
  if (parser_flag) {
    if (flags.b) flags.n = 0;
    int num_of_files = 0;
    for (int i = 1; i < argc; ++i) {
      if (args[i][0] != '-') {
        output(args[i], flags, &counter_b_n, &num_of_files);
        num_of_files++;
      }
    }
  }
  return 0;
}

void parsing(int argc, char *args[], FLAGS *flags, int *parser_flag) {
  static struct option long_options[] = {
      {"number-nonblank", 0, 0, 'b'},  // -b
      {"number", 0, 0, 'n'},           // -n
      {"squeeze-blank", 0, 0, 's'},    // -s
      {0, 0, 0, 0}  // окончание данной структуры
                    // 2 значение определяет, принимает ли опция аргумент (0 -
                    // нет) 3 значение - указатель на переменную, которой будет
                    // присваиваться значение, 0 - значение просто возвращается
  };
  int value, long_value = 0;  // ???
  while ((value = getopt_long(argc, args, "beEnstTv", long_options,
                              &long_value)) != -1) {
    switch_parsing(value, flags, parser_flag);
    if (!parser_flag) break;
  }
}

void switch_parsing(int value, FLAGS *flags, int *flag_for_parsing) {
  switch (value) {
    case 'b':
      flags->b = 1;
      break;
    case 'e':
      flags->e = 1;
      flags->v = 1;
      break;
    case 'E':
      flags->e = 1;
      break;
    case 'n':
      flags->n = 1;
      break;
    case 's':
      flags->s = 1;
      break;
    case 't':
      flags->t = 1;
      flags->v = 1;
      break;
    case 'T':
      flags->t = 1;
      break;
    case 'v':
      flags->v = 1;
      break;
    default:
      *flag_for_parsing = 0;
  }
}

void output(char args[], FLAGS flags, int *i, int *num_of_files) {
  FILE *stream = fopen(args, "r");
  int enters_for_s = 0, flag_for_dir = 0;
  DIR *dir = opendir(args);
  if (dir) {
    printf("cat: %s: Is a directory\n", args);
    closedir(dir);
    if (stream) fclose(stream);
    flag_for_dir = 1;
  }
  if (!stream || flag_for_dir) {
    if (!flag_for_dir) printf("cat: %s: No such file or directory\n", args);
    (*num_of_files)--;
  } else {
    char enter = '\0';
    if (flags.e) enter = '$';
    int symbol, num_of_string = 0;

    while ((symbol = fgetc(stream)) != EOF) {
      if (flags.s && symbol == '\n' && enters_for_s)
        continue;  // s
      else if (flags.s && symbol == '\n')
        enters_for_s = 1;  // s
      if ((*num_of_files) == 0 || num_of_string != 0) {
        if (flags.b && symbol != '\n')
          printf("%6d\t", (*i)++);  // b
        else if (flags.n)
          printf("%6d\t", (*i)++);  // n
      }
      if (symbol != '\n') enters_for_s = 0;  // обнуление для s
      while (symbol != '\n' && symbol != EOF) {
        if (symbol == '\t' && (flags.t)) {  // v and t
          printf("^I");
        } else {
          if (flags.v) flag_v(&symbol);  // v
          printf("%c", symbol);
        }
        symbol = fgetc(stream);
      }

      if (symbol == 10) {
        if (flags.e)
          printf("%c%c", enter, symbol);
        else
          printf("%c", symbol);
        num_of_string++;
      }
    }
    fclose(stream);
  }
}

void flag_v(int *symbol) {
  if (*symbol < 32 && *symbol != '\t' && *symbol != '\n') {
    printf("^");
    *symbol += 64;
  } else if (*symbol > 127 && *symbol < 160) {
    printf("M-^");
    *symbol -= 64;
  } else if (*symbol >= 160) {
    printf("M-");
    *symbol -= 128;
  } else if (*symbol == 127) {
    printf("^");
    *symbol = 63;
  }
}

void simple_output_from_console() {
  while (1) {
    char *ptr = malloc(200);
    char *tmp_ptr = ptr;
    char c;
    while ((c = getchar()) != '\n') *tmp_ptr++ = c;
    puts(ptr);
    free(ptr);
  }
}