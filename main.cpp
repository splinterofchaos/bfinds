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

  if (argc == 1)
    return 1;

  // TODO: Allow more arguments.
  if (argc == 2)
    target = argv[1];

  if (target == NULL)
    return 1;

  char *cwd;
  if (!(cwd = get_current_dir_name())) {
    perror("cwd");
    return 1;
  }

  Edges unexplored;
  unexplored.push(strdup(cwd));

  const char *path;
  while((path = unexplored.pop()))
  {
    if (chdir(path) == -1) {
      // Probably can't cd here because it's not a dir.
      continue;
    }

    DIR *d = opendir(".");

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

  if (chdir(cwd) == -1)
    perror("couldn't cd back");

  delete [] cwd;

  return 0;
}

