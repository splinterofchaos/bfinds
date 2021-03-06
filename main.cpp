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
#include <list>
#include <memory>
#include <algorithm>

#include <thread>
#include <condition_variable>
#include <mutex>
#include <future>

#include "find.h"

void usage(int ret, const char *fmt, ...)
    __attribute__ ((noreturn)) __attribute__((format (printf, 2, 3)));

void usage(int ret)
{
  usage(ret, nullptr);
}

/// Executes a command for "-c cmd".
int command(const char *cmd, const std::vector<std::string> &matches);

/// AsyncMatches lets us iterate through matches while finding them in
/// parallel.
struct AsyncMatches 
{
  std::list<std::string> data;
  std::mutex m;
  std::condition_variable cv;
  std::thread worker;
  bool finished = false;
  
  /// An iterator that blocks until the next item has been found.
  struct Iterator
  {
    AsyncMatches &am;
    std::list<std::string>::iterator it;

    Iterator &operator++ ()
    {
      std::unique_lock<std::mutex> l(am.m);
      am.cv.wait(l, [&]{return std::next(it) != std::end(am.data) || am.finished;});
      it++;
    }

    bool operator == (const Iterator &other) {
      return &am == &other.am && it == other.it;
    }

    bool operator != (const Iterator &other) {
      return !(*this == other);
    }

    std::string &operator * () {
      return *it;
    }
  };

  /// `worker`s function.
  static int fill_data(AsyncMatches &self, Find &f) {
    std::string match;
    while (f.next(match)) {
      self.m.lock();
      self.data.push_back(match);
      self.m.unlock();
      self.cv.notify_one();
    }

    // Let the main thread know we're done.
    self.finished = true;
    self.cv.notify_one();

    return 1;
  }

  AsyncMatches(Find &f) : worker(fill_data, std::ref(*this), std::ref(f))
  {
  }

  ~AsyncMatches() {
    worker.join();
  }

  Iterator begin() {
    std::unique_lock<std::mutex> l(m);
    cv.wait(l, [&]{return data.size() || finished;});
    return {*this, std::begin(data)};
  }

  Iterator end() {
    return {*this, std::end(data)};
  }
};

int main(int argc, char **argv)
{
  Find find;

  size_t count = 10000000;     ///< Stop after finding this many matches.
  const char *cmd = nullptr;   ///< Execute this on every file.
  const char *exec = nullptr;  ///< Send every match to this when done.
  bool edit = false;           ///< Optionally open files for editing.

  for (char **arg = argv + 1; arg < argv + argc; arg++)
  {
    // Consider the first non-option argument the target,
    // fallowed by directories to search.
    if ((*arg)[0] != '-') {
      if (find.target == nullptr)
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
      } else if (strcmp(opt, "e") == 0 || strcmp(opt, "-edit") == 0) {
        edit = true;
      } else if (strcmp(opt, "P") == 0 || strcmp(opt, "-pre") == 0) {
        find.match_by(match::prefix);
      } else if (strcmp(opt, "F") == 0 || strcmp(opt, "-full") == 0) {
        find.match_by(match::eq);
      } else if (strcmp(opt, "C") == 0 || strcmp(opt, "-contains") == 0) {
        find.match_by(match::contains);
      } else if (strcmp(opt, "R") == 0 || strcmp(opt, "-regex") == 0) {
        find.match_by(match::regex);
      } else {
        usage(1, "unrecognized option: %s", opt);
      }
    }
  }

  if (find.target == nullptr)
    usage(0, "Please enter a file to search for.");

  if (!find.has_startpoint())
    find.startpoint(".");

  std::vector<std::string> matches;
  for (auto &&match : AsyncMatches(find)) {
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

  if (edit) {
    const char * editor = getenv("VISUAL");
    if (!editor) editor = getenv("EDITOR");
    if (!editor) editor = "vi";  // TODO: We /could/ check if different 
                                 // editors exist.
    command(editor, matches);    // TODO: And we could be smarter about
                                 // the syntax of sending files to edit.
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

void usage(int ret, const char *fmt=nullptr, ...)
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
  puts("\t--pre | -P (defualt) Match by prefix.");
  puts("\t--full | -F     Match by full-name comparison.");
  puts("\t--regex | -R    Match by regex.");
  puts("\t--contains | -C Match if a filename contains the target.");
  puts("\t--command <cmd> Executes <cmd> for every instance of <file>.");
  puts("\t  or -c <cmd>   If <cmd> contains a percent (%), the <file> is inserted there.");
  puts("\t                Found files will not be printed. (Useful for piping.)");
  puts("\t-<N>            Stop searching after finding <N> matches.");
  puts("\t--execute <cmd> Like --command, only it is run after the entire search");
  puts("\t  or -x <cmd>   and sends every file all at once.");

  exit(ret);
}

