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

#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifndef _CRT_USE_64BIT_STAT
#define _CRT_USE_64BIT_STAT
#endif
#include <io.h>
#include <fcntl.h>
#include <process.h>
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <locale.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#define open win_open
#define read _read
#define close _close
#define lseek _lseeki64
#define fstat _fstat64
#define stat _stat64
#define isatty _isatty
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif

#ifndef S_ISREG
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef S_ISCHR
#define S_ISCHR(m) (((m) & _S_IFMT) == _S_IFCHR)
#endif
#ifndef S_ISFIFO
#define S_ISFIFO(m) (0)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m) (0)
#endif

#ifndef SIGPIPE
#define SIGPIPE SIGTERM
#endif

#ifndef mode_t
typedef int mode_t;
#endif

#ifndef _SSIZE_T_DEFINED
typedef intptr_t ssize_t;
#define _SSIZE_T_DEFINED
#endif

#ifndef OFF_T_MAX
#define OFF_T_MAX LLONG_MAX
#endif
#ifndef OFF_T_MIN
#define OFF_T_MIN (-(OFF_T_MAX) - 1)
#endif


typedef ptrdiff_t idx_t;
#define IDX_MAX PTRDIFF_MAX

#ifndef PID_T_MAX
#define PID_T_MAX INT_MAX
#endif

#ifdef _WIN32
typedef int pid_t;
#endif

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define TYPE_MINIMUM(t) ((t) ((t) 1 << (sizeof (t) * CHAR_BIT - 1)))

#define _(s) (s)
#define N_(s) (s)
#define gettext(s) (s)

#define streq(a,b) (strcmp ((a), (b)) == 0)
#define FALLTHROUGH ((void) 0)
#define MAYBE_UNUSED
#define affirm(cond) do { if (!(cond)) error (EXIT_FAILURE, 0, "assertion failed: %s", #cond); } while (0)

#define XTOINT_MAX_QUIET 0

extern const char *program_name;

void initialize_main (int *argc, char ***argv);
void set_program_name (const char *argv0);

void error (int status, int errnum, const char *format, ...);
void xalloc_die (void);

void *xmalloc (size_t size);
void *xrealloc (void *ptr, size_t size);
void *ximalloc (idx_t size);
void *xinmalloc (idx_t n, size_t s);
void *xirealloc (void *ptr, idx_t size);
void *xpalloc (void *ptr, idx_t *n, idx_t n_incr, idx_t n_max, size_t s);

int xset_binary_mode (int fd, int mode);

intmax_t xnumtoimax (const char *str, int base, intmax_t min, intmax_t max,
                     const char *suffixes, const char *errmsg,
                     int flags, int quiet);

double cl_strtod (const char *nptr, char **endptr);
double dtimespec_bound (double seconds, int err);
int xnanosleep (double seconds);

struct timespec get_stat_mtime (const struct stat *st);
int timespec_cmp (struct timespec a, struct timespec b);

const char *quote (const char *s);
const char *quotef (const char *s);
const char *quoteaf (const char *s);
const char *proper_name (const char *s);

void emit_try_help (void);
void emit_stdin_note (void);
void emit_mandatory_arg_note (void);
void emit_ancillary_info (const char *program);
void close_stdout (void);
void write_error (void);

int iopoll (int fd, int fd2, bool writing);
#define IOPOLL_OK 0
#define IOPOLL_BROKEN_OUTPUT 1

int isapipe (int fd);
int issymlink (const char *path);

int getpagesize (void);

void *memrchr (const void *s, int c, size_t n);
void *rawmemchr (const void *s, int c);

int win_open (const char *path, int oflag);

/* getopt compatibility is provided by local getopt.h */

/* Additional helpers used by tail.c */
#define STP_BLKSIZE(st) (0)
#define usable_st_size(st) (false)
#define SAME_INODE(f, s) ((f).st_ino == (s).st_ino && (f).st_dev == (s).st_dev)

/* i18n stubs */
const char *bindtextdomain (const char *domainname, const char *dirname);
const char *textdomain (const char *domainname);

static inline void main_exit (int status)
{
  exit (status);
}
