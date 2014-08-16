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
#include <memory>
#include <algorithm>

#include "find.h"

void usage(int ret, const char *fmt, ...)
    __attribute__ ((noreturn)) __attribute__((format (printf, 2, 3)));

void usage(int ret)
{
  usage(ret, NULL);
}

/// Executes a command for "-c cmd".
int command(const char *cmd, const std::string &match);

int main(int argc, char **argv)
{
  Find find;
  size_t count = 10000000;  ///< Stop after finding this many matches.
  const char *cmd = NULL;   ///< Execute this on every file.

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
      } else if (strcmp(opt, "c") == 0 || strcmp(opt, "-command") == 0) {
        if (++arg >= argv + argc || (*arg)[0] == '-')
          usage(1, "-c(ommand) requires an argument");
        cmd = *arg;
      }
    }
  }

  if (find.target == NULL)
    usage(0, "Please enter a file to search for.");

  if (!find.has_startpoint())
    find.startpoint(".");

  std::string match;
  while(find.next(match)) {
    if (match[0] == '.' && match[1] == '/')
      match += 2;  // I hate that "./" prefix!

    // TODO: Do we need to print anything when given a command?
    puts(match.c_str());

    if (cmd)
      command(cmd, match);

    if (--count == 0)
      return 0;
  }

  return 0;
}

int command(const char *cmd, const std::string &match)
{
  size_t cmdLen = strlen(cmd);
  size_t size = cmdLen + match.size() + 3;

  // We can't use std::string /and/ fill it with snprintf, so...
  std::unique_ptr<char[]> exec(new char[size]);

  if (const char *wild = strchr(cmd, '%')) {
    // Expand "prog -a % -b" to "prog -a <match> -b".
    std::string fmt = std::string(cmd, wild) + "%s" + (wild + 1);
    snprintf(exec.get(), size, fmt.c_str(), match.c_str());
  } else {
    snprintf(exec.get(), size, "%s %s", cmd, match.c_str());
  }

  return system(exec.get());
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

