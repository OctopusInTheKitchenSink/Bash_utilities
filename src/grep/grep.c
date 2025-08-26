#include <dirent.h>
#include <getopt.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  char e;  // pattern ("...$", "^...", "\<"... = "\b...", "\>..." = "d\...",
           // ".", "[]")
  char i;  // Ignore uppercase vs. lowercase.
  char v;  // Invert match (without)
  char c;  // Output count
  char l;  // Output matching files there you found patterns
  char n;  // Precede each matching line with a line number.
  char h;
  char s;
  char f;
  char o;
} FLAGS;

void parsing(int argc, char *args[], FLAGS *flags, int *parser_flag);
int getopt(int argc, char *const argv[], const char *optstring);
void switch_flags(int value, FLAGS *flags, int *parser_flag);
void works_with_flags(int argc, char *args[], FLAGS flags, int count_files);

void read_file_name(int argc, char *args[], char *name_file, int *k);
char read_patterns_from_file(int *count_of_patterns,
                             char ***ptr_patterns_from_file,
                             int *len_of_patterns, FILE *patterns_file,
                             int *tmp_of_patterns);
void read_patterns_from_string(int argc, char *args[],
                               char ***ptr_patterns_from_file,
                               int *tmp_of_patterns, int *count_of_patterns,
                               int *len_of_patterns);
void we_found_match(char args[], int *counter, int another_flags, FLAGS flags,
                    int *counter_for_c, int strings_counter, char *ptr,
                    int counter_files);
void we_didnt_found_matches(char args[], int another_flags, int *counter_for_v,
                            int *counter_for_cv, FLAGS flags,
                            int strings_counter, char *ptr, int *counter_for_c,
                            int count_of_patterns, int counter_files);
char work_in_file_for_search(char *arg, FILE *stream, FLAGS flags,
                             char **ptr_patterns_from_file, int another_flags,
                             int *strings_counter, int count_of_patterns,
                             int count_files);
void memory_error_1d(char **ptr_patterns_from_file, char *flag);
void memory_error_2d(char **ptr_patterns_from_file, int tmp_of_patterns,
                     char *flag);
void memory_error_without_free(char *flag);
void memory_error_with_free(char *flag, char *ptr);
void flag_o(char *ptr, regex_t *rx, int count_of_patterns, FLAGS flags,
            int *counter_for_c, char args[], int strings_counter,
            int counter_files);
void searching(int count_of_patterns, char **ptr_patterns_from_file,
               int flag_register_for_i, FLAGS flags, char *ptr,
               int *counter_for_c, int another_flags, char *arg,
               int strings_counter, int *counter_for_cv, int counter_files);
int is_dir(char name_file[], FILE *patterns_file);

void count_of_file_for_search(char *args[], int argc, int *count_files);

extern char *optarg;
extern int optind, opterr, optopt;

int main(int argc, char *args[]) {
  FLAGS flags = {0};
  int parser_flag = 1;
  int count_files = 0;
  parsing(argc, args, &flags, &parser_flag);
  count_of_file_for_search(args, argc, &count_files);
  if (argc > 2) {
    if (parser_flag)
      works_with_flags(argc, args, flags, count_files);
    else
      printf("grep: invalid option");
  }
  return 0;
}

void parsing(int argc, char *args[], FLAGS *flags, int *parser_flag) {
  int value;
  while ((value = getopt(argc, args, "e:vilcnhsf:o")) != -1) {
    switch_flags(value, flags, parser_flag);
  }
}

void switch_flags(int value, FLAGS *flags, int *parser_flag) {
  switch (value) {
    case 'e':
      flags->e += 1;
      break;
    case 'v':
      flags->v += 1;
      break;
    case 'i':
      flags->i += 1;
      break;
    case 'l':
      flags->l += 1;
      break;
    case 'c':
      flags->c += 1;
      break;
    case 'n':
      flags->n += 1;
      break;
    case 'h':
      flags->h += 1;
      break;
    case 's':
      flags->s += 1;
      break;
    case 'f':
      flags->f += 1;
      break;
    case 'o':
      flags->o = 1;
      break;
    default:
      *parser_flag = 0;
  }
}

int is_dir(char name_file[], FILE *patterns_file) {
  DIR *dir = opendir(name_file);
  int flag = 0;
  if (dir) {
    closedir(dir);
    if (patterns_file) fclose(patterns_file);
    flag = 1;
  }
  return flag;
}

void works_with_flags(int argc, char *args[], FLAGS flags, int count_files) {
  int count_of_patterns = 5, len_of_patterns = 30, tmp_of_patterns = 0,
      another_flags = flags.c + flags.i + flags.l + flags.n + flags.v +
                      flags.h + flags.s + flags.o;
  char total_flag = 1;
  char **ptr_patterns_from_file =
      (char **)malloc(sizeof(char *) * count_of_patterns);
  memset(ptr_patterns_from_file, 0, sizeof(char *) * count_of_patterns);
  if (flags.f || flags.e) {
    if (flags.f && ptr_patterns_from_file) {
      int k = 1;
      for (int i = 0; i < flags.f; ++i) {
        char name_file[50] = {0};
        read_file_name(argc, args, name_file, &k);
        FILE *patterns_file = fopen(name_file, "r");
        int flag_for_dir = is_dir(name_file, patterns_file);  //////////
        if (patterns_file && !flag_for_dir)
          total_flag *= read_patterns_from_file(
              &count_of_patterns, &ptr_patterns_from_file, &len_of_patterns,
              patterns_file, &tmp_of_patterns);
        else {
          if (!flags.s) {
            if (flag_for_dir)
              printf("grep: %s: Is a directory\n", name_file);
            else
              printf("%s: No such file or directory\n", name_file);
          }
          total_flag = 2;
        }
      }
    }
    if (flags.e)
      read_patterns_from_string(argc, args, &ptr_patterns_from_file,
                                &tmp_of_patterns, &count_of_patterns,
                                &len_of_patterns);
  } else {
    len_of_patterns = strlen(args[1 + another_flags]) + 1;
    ptr_patterns_from_file[0] = malloc(sizeof(char) * (len_of_patterns));
    strcpy(ptr_patterns_from_file[0], args[1 + another_flags]);
    tmp_of_patterns = 1;
  }
  if (total_flag == 1) {
    for (int i = argc - count_files; i < argc; ++i) {
      FILE *stream = fopen(args[i], "r");
      int flag_for_dir = is_dir(args[i], stream);
      int strings_counter = 1;
      if (stream && !flag_for_dir) {
        total_flag = work_in_file_for_search(
            args[i], stream, flags, ptr_patterns_from_file, another_flags,
            &strings_counter, tmp_of_patterns, count_files);
        fclose(stream);
      } else if (!flags.s) {
        if (flag_for_dir)
          printf("grep: %s: Is a directory\n", args[i]);
        else
          printf("%s: No such file or directory\n", args[i]);
      }
    }
  }
  if (total_flag) {
    for (int q = 0; q < count_of_patterns; ++q) free(ptr_patterns_from_file[q]);
    free(ptr_patterns_from_file);
  }
}

void count_of_file_for_search(char *args[], int argc, int *count_files) {
  char flag = 1;
  for (int i = 2; i < argc && flag; ++i)
    if (args[i][0] != '-' && args[i - 1][0] != '-') {
      flag = 0;
      *count_files = argc - i;
    }
}

void read_file_name(int argc, char *args[], char *name_file, int *k) {
  for (int i = *k; i < argc; ++i) {
    if (args[i][0] == '-' && (args[i][1] == 'f' || args[i][2] == 'f') &&
        i + 1 < argc) {
      strncpy(name_file, args[i + 1], 49);
      name_file[49] = '\0';
      *k = i + 1;
      break;
    }
  }
}

char read_patterns_from_file(int *count_of_patterns,
                             char ***ptr_patterns_from_file,
                             int *len_of_patterns, FILE *patterns_file,
                             int *tmp_of_patterns) {
  int symbol = 1;
  char flag = 1;
  while (symbol != EOF && flag) {
    if (*tmp_of_patterns >= *count_of_patterns) {
      *count_of_patterns += *count_of_patterns + 1;
      char **new_tmp_ptr = (char **)realloc(
          *ptr_patterns_from_file, *count_of_patterns * sizeof(char *));
      if (new_tmp_ptr) {
        *ptr_patterns_from_file = new_tmp_ptr;
      } else {
        memory_error_1d(*ptr_patterns_from_file, &flag);
        fclose(patterns_file);
        break;
      }
    }
    int tmp_of_len = 0;
    (*ptr_patterns_from_file)[*tmp_of_patterns] =
        (char *)malloc(sizeof(char) * *len_of_patterns);
    if ((*ptr_patterns_from_file)[*tmp_of_patterns] && flag) {
      while ((symbol = fgetc(patterns_file)) != '\n' && symbol != EOF && flag) {
        if (tmp_of_len >= *len_of_patterns - 1) {
          *len_of_patterns += *len_of_patterns;
          char *tmp_for_len =
              (char *)realloc((*ptr_patterns_from_file)[*tmp_of_patterns],
                              *len_of_patterns * sizeof(char));
          if (tmp_for_len) {
            (*ptr_patterns_from_file)[*tmp_of_patterns] = tmp_for_len;
          } else {
            memory_error_2d(*ptr_patterns_from_file, *tmp_of_patterns, &flag);
            fclose(patterns_file);
            break;
          }
        }
        (*ptr_patterns_from_file)[*tmp_of_patterns][tmp_of_len] = symbol;
        tmp_of_len++;
      }
      (*ptr_patterns_from_file)[*tmp_of_patterns][tmp_of_len] = '\0';
    } else
      memory_error_without_free(&flag);
    (*tmp_of_patterns)++;
  }
  fclose(patterns_file);
  return flag;
}

void read_patterns_from_string(int argc, char *args[],
                               char ***ptr_patterns_from_file,
                               int *tmp_of_patterns, int *count_of_patterns,
                               int *len_of_patterns) {
  char flag = 1;
  for (int i = 0; i < argc && flag; ++i) {
    if ((args[i][0] == '-' && args[i][1] == 'e')) {
      if (*tmp_of_patterns >= *count_of_patterns) {
        *count_of_patterns += *tmp_of_patterns + 1;
        char **new_tmp_ptr = (char **)realloc(
            *ptr_patterns_from_file, *count_of_patterns * sizeof(char *));
        if (new_tmp_ptr)
          *ptr_patterns_from_file = new_tmp_ptr;
        else
          memory_error_1d(*ptr_patterns_from_file, &flag);
      }
      if (strlen(args[i + 1]) > (size_t)*len_of_patterns) *len_of_patterns *= 2;
      if (flag) {
        char *tmp = (char *)malloc(sizeof(char) * *len_of_patterns);
        if (tmp)
          (*ptr_patterns_from_file)[*tmp_of_patterns] = tmp;
        else
          memory_error_2d(*ptr_patterns_from_file, *count_of_patterns, &flag);
        if (flag)
          strncpy((*ptr_patterns_from_file)[*tmp_of_patterns], args[i + 1],
                  *len_of_patterns - 1);
      }
      *tmp_of_patterns += 1;
    }
  }
}

void memory_error_2d(char **ptr_patterns_from_file, int tmp_of_patterns,
                     char *flag) {
  printf("I don't have enough memory\n");
  for (int q = 0; q < tmp_of_patterns; ++q) free(ptr_patterns_from_file[q]);
  *flag = 0;
}

void memory_error_1d(char **ptr_patterns_from_file, char *flag) {
  printf("I don't have enough memory\n");
  free(ptr_patterns_from_file);
  *flag = 0;
}

char work_in_file_for_search(char *arg, FILE *stream, FLAGS flags,
                             char **ptr_patterns_from_file, int another_flags,
                             int *strings_counter, int count_of_patterns,
                             int counter_files) {
  int counter_for_c = 0, counter_for_cv = 0, symbol = 1,
      flag_register_for_i = (flags.i ? REG_ICASE : 0);
  char flag = 1;
  while (symbol != EOF) {
    unsigned long count = 50, ptr_count = 0;
    char *ptr = (char *)malloc(sizeof(char) * count);
    if (!ptr) {
      memory_error_without_free(&flag);
      break;
    }
    char *tmp_ptr = ptr;
    while ((symbol = fgetc(stream)) != '\n' && symbol != EOF && flag) {
      if (ptr_count < count)
        *tmp_ptr++ = symbol;
      else {
        count += ptr_count + 1;
        char *new_ptr = (char *)realloc(ptr, count);
        if (!new_ptr) {
          memory_error_with_free(&flag, ptr);  /////////////
          break;
        }
        ptr = new_ptr;
        tmp_ptr = ptr + ptr_count;
        *tmp_ptr++ = symbol;
      }
      ptr_count++;
    }
    *tmp_ptr = '\0';
    searching(count_of_patterns, ptr_patterns_from_file, flag_register_for_i,
              flags, ptr, &counter_for_c, another_flags, arg, *strings_counter,
              &counter_for_cv, counter_files);
    (*strings_counter)++;
    free(ptr);
  }
  if (flags.c && !flags.l) {
    if (flags.h || counter_files < 2)
      printf("%d\n", counter_for_c);
    else
      printf("%s:%d\n", arg, flags.v ? counter_for_cv : counter_for_c);
  }
  if (flags.l && counter_for_c && !flags.v) printf("%s\n", arg);
  return flag;
}

void searching(int count_of_patterns, char **ptr_patterns_from_file,
               int flag_register_for_i, FLAGS flags, char *ptr,
               int *counter_for_c, int another_flags, char *arg,
               int strings_counter, int *counter_for_cv, int counter_files) {
  regex_t *rx = (regex_t *)malloc(sizeof(regex_t) * count_of_patterns);
  regmatch_t matches[1];  // для хранения совпадений
  int reg,
      counter = 0,  // не печатает строку несколько раз, если в ней
                    // есть несколько шаблонов
      counter_for_v =
          0;  // чтобы при ненахождении одного шаблона печатал только один раз
  for (int l = 0; l < (count_of_patterns); ++l)
    reg = regcomp(rx + l, (ptr_patterns_from_file[l]),
                  flag_register_for_i);  /////////////////
  if (flags.o) {
    if (!flags.v)
      flag_o(ptr, rx, count_of_patterns, flags, counter_for_c, arg,
             strings_counter, counter_files);
  } else {
    for (int l = 0; l < (count_of_patterns); ++l) {
      if (!(reg = regexec(&rx[l], ptr, 1, matches, 0)) && !counter)
        we_found_match(arg, &counter, another_flags, flags, counter_for_c,
                       strings_counter, ptr, counter_files);
      else {
        we_didnt_found_matches(arg, another_flags, &counter_for_v,
                               counter_for_cv, flags, strings_counter, ptr,
                               counter_for_c, count_of_patterns, counter_files);
      }
    }
    if (flags.f && flags.v && !counter) {
      if ((counter_files > 1)) printf("%s:", arg);
      printf("%s\n", ptr);
    }
  }
  for (int i = 0; i < count_of_patterns; ++i) regfree(&rx[i]);
  free(rx);
}

void flag_o(char *ptr, regex_t *rx, int count_of_patterns, FLAGS flags,
            int *counter_for_c, char args[], int strings_counter,
            int counter_files) {
  int reg = 0, counter = 0;
  char *tmp_ptr = ptr;
  while (!reg) {
    regmatch_t matches[1];
    int start = INT_MAX, end = INT_MIN, found = 0;
    for (int i = 0; i < count_of_patterns; ++i) {
      if (!(reg = regexec(&rx[i], tmp_ptr, 1, matches, 0))) {
        counter++;
        found++;
        if (start == matches->rm_so)
          end = (end > matches->rm_eo ? end : matches->rm_eo);
        if (start > matches->rm_so) {
          start = matches->rm_so;
          end = matches->rm_eo;
        }
      }
    }
    if (!counter) break;
    if (!flags.l && !flags.c) {
      if (found && !flags.h && (counter_files > 1)) printf("%s:", args);
      if (found && flags.n) printf("%d:", strings_counter);
      for (int i = start; i < end; ++i) {
        printf("%c", tmp_ptr[i]);
        if (i == end - 1) printf("\n");
      }
    }
    tmp_ptr += end;
  }
  *counter_for_c += (counter != 0);
}

void we_found_match(char args[], int *counter, int another_flags, FLAGS flags,
                    int *counter_for_c, int strings_counter, char *ptr,
                    int counter_files) {
  (*counter)++;
  if (!flags.v) *counter_for_c += 1;
  if ((counter_files > 1) && !flags.h && !flags.c && !flags.l && !flags.v &&
      !flags.o)
    printf("%s:", args);
  if (flags.n && !flags.l && !flags.c && !flags.o && !flags.v)
    printf("%d:", strings_counter);
  if ((flags.e || flags.i || flags.h || !another_flags || flags.n || flags.f ||
       flags.s) &&
      !flags.v && !flags.o && !flags.l && !flags.c)
    puts(ptr);
}

void we_didnt_found_matches(char args[], int another_flags, int *counter_for_v,
                            int *counter_for_cv, FLAGS flags,
                            int strings_counter, char *ptr, int *counter_for_c,
                            int count_of_patterns, int counter_files) {
  (*counter_for_v)++;
  (*counter_for_cv)++;
  if (flags.c && flags.v) ((*counter_for_c)++);
  if (flags.v && (*counter_for_v == count_of_patterns || flags.f) && !flags.c &&
      !flags.o) {
    if ((flags.l || flags.c) && *counter_for_cv == 1)
      printf("%s\n", flags.h ? "" : args);
    else {
      if ((flags.f || flags.n || flags.e || flags.i || flags.s) &&
          (counter_files > 1))
        if (!flags.h) printf("%s:", args);
      if (flags.n) printf("%d:", strings_counter);
    }
    if (another_flags == flags.v && (counter_files > 1) && !flags.e)
      printf("%s:", args);
    if (!flags.l && !flags.f) printf("%s\n", ptr);
  }
}

void memory_error_without_free(char *flag) {
  *flag = 0;
  puts("I don't have memory\n");
}

void memory_error_with_free(char *flag, char *ptr) {
  puts("I don't have memory\n");
  free(ptr);
  *flag = 0;
}