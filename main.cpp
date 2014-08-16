#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <utility>
#include <algorithm>

#include "find.h"

void usage(int ret, const char *fmt, ...)
    __attribute__ ((noreturn)) __attribute__((format (printf, 2, 3)));

void usage(int ret)
{
  usage(ret, NULL);
}

int main(int argc, char **argv)
{
  Find find;
  size_t count = 10000000;  ///< Stop after finding this many matches.

  for (char **arg = argv + 1; arg < argv + argc; arg++)
  {
    // Consider the first non-option argument the target,
    // fallowed by directories to search.
    if ((*arg)[0] != '-') {
      if (find.target == NULL)
        find.target = *arg;
      else
        find.startpoint(*arg);
    } else {
      char *opt = (*arg) + 1;
      if (isdigit(opt[0])) {
        char *end;
        count = strtoul(opt, &end, 10);
        if (*end != 0)
          usage(1, "unexpected token, '%s', after count (%s)", end, *arg);
      }
    }
  }

  if (find.target == NULL)
    usage(0, "Please enter a file to search for.");

  if (!find.has_startpoint())
    find.startpoint(".");

  std::string match;
  while(find.next(match)) {
    size_t off = 0;
    if (match[0] == '.' && match[1] == '/')
      off = 2;  // I hate that "./" prefix!
    puts(match.c_str() + off);

    if (--count == 0)
      return 0;
  }

  return 0;
}

void usage(int ret, const char *fmt=NULL, ...)
{
  FILE *out = (ret == 0) ? stdout : stderr;

  if (fmt) {
    va_list va;
    va_start(va, fmt);
    vfprintf(out, fmt, va);
    fprintf(out, "\n");
    va_end(va);
  }
  fprintf(out, "usage: bfinds FILE [PATH...]\n");

  exit(ret);
}

