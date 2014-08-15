
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
