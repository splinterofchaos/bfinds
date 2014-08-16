
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "find.h"

static const char *pop(Edges &es) {
  const char *path = NULL;
  if (es.size()) {
    path = es.front();
    es.pop_front();
  }
  return path;
}

static void push(Edges &es, const char *path)
{
  es.push_back(path);
}

int *Find::search(Find *self)
{
  const char *path = pop(self->unexplored);
  for (; path; path = pop(self->unexplored))
  {
    DIR *d = opendir(path);

    fprintf(self->fl[1], "\t%s\n", path);

    struct dirent* ent;
    while (d && (ent = readdir(d))) {
      fprintf(self->fl[1], "%s\n", ent->d_name);

      if (ent->d_type == DT_DIR && !is_dot(ent->d_name))
        push(self->unexplored, path_append(path, ent->d_name));
    }

    closedir(d);
    delete [] path;
  }

  close(self->fd[1]);  // Signal the parent thread to stop reading.
  self->threadOk = 0;
}

Find::Find() : target(NULL), path(NULL), d(NULL)
{
  threadOk = (pipe(fd) >= 0);

  if (threadOk) {
    fl[0] = fdopen(fd[0], "r");
    fl[1] = fdopen(fd[1], "w");
    if (! (fl[0] && fl[1]))
      perror("fdopen");
  }
}

Find::~Find()
{
  pthread_cancel(searcher);
  delete [] path;
}

void Find::startpoint(const char *b)
{
  push(unexplored, strdup(b));
}

bool Find::has_startpoint()
{
  return !unexplored.empty();
}

size_t read_line(FILE *f, char buf[PATH_MAX])
{
  size_t max = PATH_MAX;
  size_t size = getline(&buf, &max, f);
  buf[size] = 0;
  if (buf[--size] == '\n')
    buf[size] = 0;
  return size;
}

bool Find::next(std::string &ret)
{
  if (threadOk == 1) {
    pthread_cond_init(&startSearch, NULL);

    void * (*vf)(void *) = (void *(*)(void *))search;
    if (pthread_create(&searcher, NULL, vf, this) == 0)
      threadOk = 2;
  }

  size_t read;
  char *line = new char[PATH_MAX];
  size_t len = 0;
  while (threadOk == 2 && (read = read_line(fl[0], line)) != -1)
  {
    if (! read || is_dot(line))
      continue;

    if (line[0] == '\t') {
      path = strdup(line + 1);
      continue;
    }

    if (check(target, line)) {
    //puts(path_tail(line));
    //if (check(target, path_tail(line))) {
      ret = path;
      ret.push_back('/');
      ret += line;
      return true;
    }
  }

  delete [] line;

  return false;

  if (!target)
    return false;

  // We may be returning from a previous call to next().
  // If so, don't reset the 'path'.
  if (path == NULL && unexplored.size())
    path = pop(unexplored);

  const char *p = NULL;
  for (; !p && path; path = pop(unexplored))
    p = in_path();

  if (p) {
    ret = p;
    delete [] p;
  }

  return p != NULL;
}

std::string Find::next()
{
  std::string ret;
  next(ret);
}

const char *Find::in_path()
{
  // We set 'd' to NULL every loop so we can tell the difference between
  // starting a search and continuing one.
  if (!d)
    d = opendir(path);

  struct dirent* ent;
  while (d && (ent = readdir(d)))
    if (const char *p = in_ent(ent))
      return p;

  closedir(d);  // Safe even if d is null.
  d = NULL;

  delete [] path;
  path = NULL;

  return NULL;
}

const char *Find::in_ent(struct dirent *ent)
{
  if (is_dot(ent->d_name))
    return NULL;

  if (ent->d_type == DT_DIR)
    push(unexplored, path_append(path, ent->d_name));

  if (check(target, ent->d_name))
    return path_append(path, ent->d_name);

  return NULL;
}

bool is_dot(const char *path)
{
  return path[0] == '.' && 
    (path[1] == 0 || (path[1] == '.' && path[2] == 0));
}

bool check(const char *a, const char *b)
{
  // TODO: Matching is more than simple equality.
  return strcmp(a, b) == 0;
}

const char *path_tail(const char *p)
{
  const char *tail = p;
  while (*p) {
    if (*(p++) == '/')
      tail = p;
  }
  return tail;
}

char *path_tail(char *p)
{
  return (char *) path_tail((const char *) p);
}

char *path_append(const char *from, const char *to)
{
  size_t fromLen = strlen(from);

  // Allocate enough space in case we need to add a path separator.
  size_t size = fromLen + strlen(to) + 2;
  char *buf = new char[size];
  buf[0] = 0;

  strcat(buf, from);
  if (buf[fromLen - 1] != '/') {
    buf[fromLen++] = '/';
    buf[fromLen] = 0;
  }
  strcat(buf + fromLen, to);

  return buf;
}
