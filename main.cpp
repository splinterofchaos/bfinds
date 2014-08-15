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
  const char *target = NULL;  ///< File to search for.
  Edges unexplored;           ///< Paths to explore.
  size_t count = 10000000;    ///< Stop after finding this many matches.

  for (char **arg = argv + 1; arg < argv + argc; arg++)
  {
    // Consider the first non-option argument the target,
    // fallowed by directories to search.
    if ((*arg)[0] != '-') {
      if (target == NULL)
        target = *arg;
      else
        unexplored.push(strdup(*arg));
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

  if (target == NULL)
    usage(0, "Please enter a file to search for.");

  if (unexplored.empty())
    unexplored.push(strdup("."));

  const char *path;
  while(path = unexplored.pop())
  {
    DIR *d = opendir(path);

    // TODO: Under what situations will we need error reporting?
    // Usually, it's just a boring 'permission denied'.
    if (d == NULL) {
      continue;
    }

    struct dirent* ent;
    while (ent = readdir(d))
    {
      if (is_dot(ent->d_name))
        continue;

      if (check(target, ent->d_name)) {
        // I hate that "./" prefix!
        bool rel = (strncmp(path, "./", 2) == 0);
        const char *p = rel ? path + 2 : path;

        printf("%s/%s\n", p, ent->d_name);
        if (--count == 0)
          return 0;
      }

      if (ent->d_type == DT_DIR)
        unexplored.push(path_append(path, ent->d_name));
    }

    closedir(d);
    delete [] path;
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

