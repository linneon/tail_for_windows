#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

struct option
{
  const char *name;
  int has_arg;
  int *flag;
  int val;
};

enum
{
  no_argument = 0,
  required_argument = 1,
  optional_argument = 2
};

int getopt_long (int argc, char *const argv[], const char *optstring,
                 const struct option *longopts, int *longindex);

#ifdef __cplusplus
}
#endif
