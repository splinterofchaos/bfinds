
#include <string.h>

#include "find.h"

Node::Node(const char *path) : child(NULL), path(path)
{
}

/* Edges members */
Edges::Edges()
{
  top = bot = NULL;
}

Edges::~Edges()
{
  while (bot) {
    // Destruct called with items left.
    // Destroying the paths becomes our job.
    delete [] pop();
  }
}

bool Edges::empty()
{
  return bot == NULL;
}

const char *Edges::pop()
{
  const char *ret = NULL;
  if (!empty()) {
    ret = bot->path;
    Node *old = bot;
    bot = bot->child;
    delete old;
  }

  return ret;
}

void Edges::push(const char *path)
{
  Node *e = new Node(path);

  // bot will be null if every edge has been popped.
  if (!bot) {
    bot = top = e;
  } else {
    top->child = e;
    top = e;
  }
}

Find::Find() : target(NULL), path(NULL), d(NULL)
{
}

Find::~Find()
{
  delete [] path;
}

void Find::startpoint(const char *b)
{
  unexplored.push(strdup(b));
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
  if (path == NULL)
    if ((path = unexplored.pop()) == NULL)
      return false;

  const char *p = NULL;
  for (; !p && path; path = unexplored.pop())
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
    unexplored.push(path_append(path, ent->d_name));

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
