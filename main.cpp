#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <utility>
#include <algorithm>

#include "find.h"

int main(int argc, char **argv)
{
  const char *target = NULL;  ///< File to search for.
  Edges unexplored;           ///< Paths to explore.

  if (argc == 1)
    return 1;

  int argi = 1;

  for (char **arg = argv + 1; arg < argv + argc; arg++)
  {
    // Consider the first non-option argument the target,
    // fallowed by directories to search.
    if ((*arg)[0] == '-') {
      // TODO: options
    } else {
      if (target == NULL)
        target = *arg;
      else
        unexplored.push(strdup(*arg));
    }
  }

  if (target == NULL) {
    fprintf(stderr, "usage: bfinds [FILE] [PATHS...]");
    return 1;
  }

  if (unexplored.empty())
    unexplored.push(strdup("."));

  const char *path;
  while((path = unexplored.pop()))
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

      char *next = path_append(path, ent->d_name);

      if (check(target, ent->d_name)) {
        bool rel = (strncmp(next, "./", 2) == 0);
        puts(rel ? next + 2 : next);
      }

      if (ent->d_type == DT_DIR)
        unexplored.push(next);
      else
        delete [] next;
    }

    closedir(d);
    delete [] path;
  }


  return 0;
}

