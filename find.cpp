
#include <string.h>

#include "find.h"

static const char *pop(Edges &es) {
  const char *path = nullptr;
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

Find::Find()
{
}

Find::~Find()
{
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

bool Find::next(std::string &ret)
{
  if (!target)
    return false;

  // We may be returning from a previous call to next().
  // If so, don't reset the 'path'.
  if (path == nullptr && unexplored.size())
    path = pop(unexplored);

  const char *p = nullptr;
  for (; path; path = pop(unexplored)) {
    if (p = in_path()) {
      ret = p;
      delete [] p;
      break;  // Don't let the next path be popped!
    }
  }

  return p;
}

std::string Find::next()
{
  std::string ret;
  next(ret);
}

const char *Find::in_path()
{
  // We set 'd' to nullptr every loop so we can tell the difference between
  // starting a search and continuing one.
  if (!d)
    d = opendir(path);

  struct dirent* ent;
  while (d && (ent = readdir(d)))
    if (const char *p = in_ent(ent))
      return p;

  closedir(d);  // Safe even if d is null.
  d = nullptr;

  delete [] path;
  path = nullptr;

  return nullptr;
}

const char *Find::in_ent(struct dirent *ent)
{
  if (ent->d_type == DT_DIR) {
    if (is_dot(ent->d_name))
      return nullptr;
    push(unexplored, path_append(path, ent->d_name));
  }

  if (check(target, ent->d_name))
    return path_append(path, ent->d_name);

  return nullptr;
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
