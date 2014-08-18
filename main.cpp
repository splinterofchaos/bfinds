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

#include <thread>
#include <future>

#include "find.h"

void usage(int ret, const char *fmt, ...)
    __attribute__ ((noreturn)) __attribute__((format (printf, 2, 3)));

void usage(int ret)
{
  usage(ret, NULL);
}

/// Executes a command for "-c cmd".
int command(const char *cmd, const std::vector<std::string> &matches);

int main(int argc, char **argv)
{
  Find find;
  size_t count = 10000000;  ///< Stop after finding this many matches.
  const char *cmd = NULL;   ///< Execute this per match.
  const char *exec = NULL;  ///< Send every match to this when done.

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
      } else if (strcmp(opt, "x") == 0 || strcmp(opt, "-execute") == 0) {
        if (++arg >= argv + argc || (*arg)[0] == '-')
          usage(1, "%s requires an argument", *arg);
        exec = *arg;
      } else {
        usage(1, "unrecognized option: %s", opt);
      }
    }
  }

  if (find.target == NULL)
    usage(0, "Please enter a file to search for.");

  if (!find.has_startpoint())
    find.startpoint(".");

  // We will run the actual file search in another thread in case the user
  // supplies the -c option; the command can execute while we find the next
  // file.
  std::string futMatch;
  auto do_match = [&]{ return find.next(futMatch); };
  std::future<bool> found = std::async(do_match);

  std::vector<std::string> matches;  // Store matches here.

  while(found.get()) {
    std::string match = futMatch;
    
    found = std::async(std::launch::async, do_match);

    if (match[0] == '.' && match[1] == '/')
      match.erase(0, 2);  // I hate that "./" prefix!

    if (cmd)
      command(cmd, {match});
    else
      puts(match.c_str());

    matches.push_back(match);

    if (--count == 0)
      break;
  }

  if (exec)
    command(exec, matches);

  return 0;
}

std::string concat(const std::vector<std::string> &);

int command(const char *cmd, const std::vector<std::string> &matches)
{
  std::string run;  // The program to run.

  // If the command has a %, expand the matches there; else at the end.
  if (const char *wild = strchr(cmd, '%'))
    run = std::string(cmd, wild) + concat(matches) + (wild + 1);
  else
    run = cmd + (" " + concat(matches));

  return system(run.c_str());
}

std::string concat(const std::vector<std::string> &vs)
{
  std::string ret;
  for (const std::string &s : vs)
    ret += s + ' ';
  if (ret.size() > 1)
    ret.erase(ret.size() - 1);  // Trailing whitespace.
  return std::move(ret);
}

void usage(int ret, const char *fmt=NULL, ...)
{
  if (fmt) {
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    fprintf(stderr, "\n");
    va_end(va);
  }

  puts("usage: bfinds [options] file [path...]");
  puts("");
  puts("options:");
  puts("\t--command <cmd> Executes <cmd> for every instance of <file>.");
  puts("\t  or -c <cmd>   If <cmd> contains a percent (%), the <file> is inserted there.");
  puts("\t                Found files will not be printed. (Useful for piping.)");
  puts("\t-<N>            Stop searching after finding <N> matches.");
  puts("\t--execute <cmd> Like --command, only it is run after the entire search");
  puts("\t  or -x <cmd>   and sends every file all at once.");

  exit(ret);
}

