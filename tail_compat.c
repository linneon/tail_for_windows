/* tail -- output the last part of file(s)
   Copyright (C) 1989-2026 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Modified for Windows/MSVC support by Ethan H., 2026. */

#include "tail_compat.h"
#include "getopt.h"
#ifdef _WIN32
#include <share.h>
#endif

const char *program_name = "tail";

void initialize_main (int *argc, char ***argv)
{
  (void) argc;
  (void) argv;
}

void set_program_name (const char *argv0)
{
  if (!argv0 || !*argv0)
    {
      program_name = "tail";
      return;
    }

  const char *base = argv0;
  const char *slash = strrchr (argv0, '/');
  const char *bslash = strrchr (argv0, '\\');
  if (slash && bslash)
    base = (slash > bslash) ? slash + 1 : bslash + 1;
  else if (slash)
    base = slash + 1;
  else if (bslash)
    base = bslash + 1;

  program_name = base;
}

void error (int status, int errnum, const char *format, ...)
{
  va_list ap;
  fprintf (stderr, "%s: ", program_name ? program_name : "tail");
  va_start (ap, format);
  vfprintf (stderr, format, ap);
  va_end (ap);

  if (errnum)
    fprintf (stderr, ": %s", strerror (errnum));

  fputc ('\n', stderr);

  if (status)
    exit (status);
}

void xalloc_die (void)
{
  error (EXIT_FAILURE, 0, "memory exhausted");
}

void *xmalloc (size_t size)
{
  void *p = malloc (size ? size : 1);
  if (!p)
    xalloc_die ();
  return p;
}

void *xrealloc (void *ptr, size_t size)
{
  void *p = realloc (ptr, size ? size : 1);
  if (!p)
    xalloc_die ();
  return p;
}

void *ximalloc (idx_t size)
{
  if (size < 0)
    xalloc_die ();
  return xmalloc ((size_t) size);
}

void *xinmalloc (idx_t n, size_t s)
{
  if (n < 0 || s == 0)
    return xmalloc (1);
  if ((size_t) n > SIZE_MAX / s)
    xalloc_die ();
  return xmalloc ((size_t) n * s);
}

void *xirealloc (void *ptr, idx_t size)
{
  if (size < 0)
    xalloc_die ();
  return xrealloc (ptr, (size_t) size);
}

void *xpalloc (void *ptr, idx_t *n, idx_t n_incr, idx_t n_max, size_t s)
{
  idx_t n0 = *n;
  idx_t n1 = n0 + (n0 < n_incr ? n_incr : n0);
  if (n1 < n0)
    xalloc_die ();
  if (n1 > n_max)
    n1 = n_max;
  if (n1 == n0)
    xalloc_die ();
  *n = n1;
  return xrealloc (ptr, (size_t) n1 * s);
}

int xset_binary_mode (int fd, int mode)
{
#ifdef _WIN32
  return _setmode (fd, mode) == -1 ? -1 : 0;
#else
  (void) fd;
  (void) mode;
  return 0;
#endif
}

static bool parse_suffix_multiplier (const char *suffix, intmax_t max,
                                      intmax_t *mul_out)
{
  if (!suffix || !*suffix)
    {
      *mul_out = 1;
      return true;
    }

  char s1 = suffix[0];
  char s2 = suffix[1];
  if (suffix[2] != '\0')
    return false;

  if (s1 == 'b' && s2 == '\0')
    {
      *mul_out = 512;
      return true;
    }

  bool decimal = false;
  if (s2 == 'B' || s2 == 'b')
    decimal = true;
  else if (s2 != '\0')
    return false;

  char prefix = s1;
  int exp = 0;
  switch (prefix)
    {
    case 'k': case 'K': exp = 1; break;
    case 'm': case 'M': exp = 2; break;
    case 'g': case 'G': exp = 3; break;
    case 't': case 'T': exp = 4; break;
    case 'p': case 'P': exp = 5; break;
    case 'e': case 'E': exp = 6; break;
    case 'z': case 'Z': exp = 7; break;
    case 'y': case 'Y': exp = 8; break;
    case 'r': case 'R': exp = 9; break;
    case 'q': case 'Q': exp = 10; break;
    default:
      return false;
    }

  intmax_t base = decimal ? 1000 : (prefix >= 'A' && prefix <= 'Z' ? 1024 : 1000);
  intmax_t mul = 1;
  for (int i = 0; i < exp; i++)
    {
      if (mul > max / base)
        return false;
      mul *= base;
    }

  *mul_out = mul;
  return true;
}

intmax_t xnumtoimax (const char *str, int base, intmax_t min, intmax_t max,
                     const char *suffixes, const char *errmsg,
                     int flags, int quiet)
{
  (void) suffixes;
  (void) flags;
  (void) quiet;

  if (!str || !*str)
    error (EXIT_FAILURE, 0, "%s", errmsg);

  errno = 0;
  char *end = NULL;
#ifdef _WIN32
  intmax_t val = _strtoi64 (str, &end, base);
#else
  intmax_t val = strtoimax (str, &end, base);
#endif

  if (end == str || errno == ERANGE)
    error (EXIT_FAILURE, 0, "%s", errmsg);

  intmax_t mul = 1;
  if (!parse_suffix_multiplier (end, max, &mul))
    error (EXIT_FAILURE, 0, "%s", errmsg);

  if (val < 0)
    error (EXIT_FAILURE, 0, "%s", errmsg);

  if (mul != 1)
    {
      if (val > max / mul)
        error (EXIT_FAILURE, 0, "%s", errmsg);
      val *= mul;
    }

  if (val < min || val > max)
    error (EXIT_FAILURE, 0, "%s", errmsg);

  return val;
}

double cl_strtod (const char *nptr, char **endptr)
{
  return strtod (nptr, endptr);
}

double dtimespec_bound (double seconds, int err)
{
  (void) err;
  return seconds;
}

int xnanosleep (double seconds)
{
  if (seconds <= 0)
    return 0;
#ifdef _WIN32
  double ms_f = seconds * 1000.0;
  if (ms_f < 0)
    ms_f = 0;
  DWORD ms = (DWORD) (ms_f + 0.5);
  Sleep (ms);
  return 0;
#else
  struct timespec ts;
  ts.tv_sec = (time_t) seconds;
  ts.tv_nsec = (long) ((seconds - (double) ts.tv_sec) * 1000000000.0);
  return nanosleep (&ts, NULL);
#endif
}

struct timespec get_stat_mtime (const struct stat *st)
{
  struct timespec ts;
  ts.tv_sec = st->st_mtime;
  ts.tv_nsec = 0;
  return ts;
}

int timespec_cmp (struct timespec a, struct timespec b)
{
  if (a.tv_sec < b.tv_sec)
    return -1;
  if (a.tv_sec > b.tv_sec)
    return 1;
  if (a.tv_nsec < b.tv_nsec)
    return -1;
  if (a.tv_nsec > b.tv_nsec)
    return 1;
  return 0;
}

const char *quote (const char *s)
{
  return s ? s : "";
}

const char *quotef (const char *s)
{
  return quote (s);
}

const char *quoteaf (const char *s)
{
  return quote (s);
}

const char *proper_name (const char *s)
{
  return s ? s : "";
}

void emit_try_help (void)
{
  fprintf (stderr, "Try '%s --help' for more information.\n",
           program_name ? program_name : "tail");
}

void emit_stdin_note (void)
{
  fputs ("With no FILE, or when FILE is -, read standard input.\n", stdout);
}

void emit_mandatory_arg_note (void)
{
  fputs ("Mandatory arguments to long options are mandatory for short options too.\n",
         stdout);
}

void emit_ancillary_info (const char *program)
{
  (void) program;
}

void close_stdout (void)
{
  if (fflush (stdout) == EOF)
    error (EXIT_FAILURE, errno, "standard output");
}

void write_error (void)
{
  error (EXIT_FAILURE, errno, "write error");
}

int iopoll (int fd, int fd2, bool writing)
{
  (void) fd;
  (void) fd2;
  (void) writing;
  return IOPOLL_OK;
}

int isapipe (int fd)
{
#ifdef _WIN32
  intptr_t h = _get_osfhandle (fd);
  if (h == -1)
    return 0;
  DWORD t = GetFileType ((HANDLE) h);
  return (t == FILE_TYPE_PIPE) ? 1 : 0;
#else
  (void) fd;
  return 0;
#endif
}

int issymlink (const char *path)
{
  (void) path;
  return 0;
}

int getpagesize (void)
{
#ifdef _WIN32
  SYSTEM_INFO si;
  GetSystemInfo (&si);
  return (int) si.dwPageSize;
#else
  return (int) sysconf (_SC_PAGESIZE);
#endif
}

int win_open (const char *path, int oflag)
{
#ifdef _WIN32
  int fd = -1;
  int shflag = _SH_DENYNO;
  int pmode = _S_IREAD | _S_IWRITE;
  if (_sopen_s (&fd, path, oflag, shflag, pmode) != 0)
    return -1;
  return fd;
#else
  return open (path, oflag);
#endif
}

void *memrchr (const void *s, int c, size_t n)
{
  const unsigned char *p = (const unsigned char *) s;
  for (size_t i = 0; i < n; i++)
    {
      const unsigned char *cur = p + (n - 1 - i);
      if (*cur == (unsigned char) c)
        return (void *) cur;
    }
  return NULL;
}

void *rawmemchr (const void *s, int c)
{
  const unsigned char *p = (const unsigned char *) s;
  for (;; p++)
    {
      if (*p == (unsigned char) c)
        return (void *) p;
    }
}

const char *bindtextdomain (const char *domainname, const char *dirname)
{
  (void) domainname;
  (void) dirname;
  return "";
}

const char *textdomain (const char *domainname)
{
  (void) domainname;
  return "";
}

/* Minimal getopt_long implementation for this project */

char *optarg = NULL;
int optind = 1;
int opterr = 1;
int optopt = 0;

static const char *optstring_scan = NULL;
static int optpos = 0;

static const struct option *find_long_option (const struct option *longopts,
                                             const char *name,
                                             int *match_index)
{
  for (int i = 0; longopts && longopts[i].name; i++)
    {
      if (strcmp (longopts[i].name, name) == 0)
        {
          if (match_index)
            *match_index = i;
          return &longopts[i];
        }
    }
  return NULL;
}

int getopt_long (int argc, char *const argv[], const char *optstring,
                 const struct option *longopts, int *longindex)
{
  optarg = NULL;

  if (optind >= argc)
    return -1;

  const char *arg = argv[optind];

  if (optpos == 0)
    {
      if (strcmp (arg, "--") == 0)
        {
          optind++;
          return -1;
        }

      if (arg[0] != '-' || arg[1] == '\0')
        return -1;

      if (arg[1] == '-')
        {
          const char *eq = strchr (arg + 2, '=');
          char namebuf[128];
          const char *name = arg + 2;
          if (eq)
            {
              size_t n = (size_t) (eq - name);
              if (n >= sizeof namebuf)
                n = sizeof namebuf - 1;
              memcpy (namebuf, name, n);
              namebuf[n] = '\0';
              name = namebuf;
            }

          int match_index = -1;
          const struct option *opt = find_long_option (longopts, name, &match_index);
          if (!opt)
            {
              optind++;
              optopt = 0;
              return '?';
            }

          if (longindex)
            *longindex = match_index;

          if (opt->has_arg == required_argument)
            {
              if (eq)
                optarg = (char *) (eq + 1);
              else if (optind + 1 < argc)
                optarg = argv[++optind];
              else
                {
                  optind++;
                  optopt = opt->val;
                  return '?';
                }
            }
          else if (opt->has_arg == optional_argument)
            {
              if (eq)
                optarg = (char *) (eq + 1);
              else
                optarg = NULL;
            }

          optind++;
          if (opt->flag)
            {
              *opt->flag = opt->val;
              return 0;
            }
          return opt->val;
        }

      optpos = 1;
      optstring_scan = optstring;
    }

  char c = arg[optpos++];
  if (arg[optpos] == '\0')
    {
      optind++;
      optpos = 0;
    }

  const char *p = strchr (optstring_scan, c);
  if (!p)
    {
      optopt = (unsigned char) c;
      return '?';
    }

  if (p[1] == ':')
    {
      if (optpos != 0 && arg[optpos] != '\0')
        {
          optarg = (char *) &arg[optpos];
          optind++;
          optpos = 0;
        }
      else if (optind < argc)
        {
          optarg = argv[optind++];
          optpos = 0;
        }
      else
        {
          optopt = (unsigned char) c;
          return '?';
        }
    }

  return (int) c;
}
