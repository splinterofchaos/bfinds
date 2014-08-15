
#include <dirent.h>

struct Node
{
  Node *child;       ///< The edge to search after this one.
  const char *path;  ///< What gets searched when expanding this node.

  Node(const char *path);
};

struct Edges
{
  Node *top;  ///< append here
  Node *bot;  ///< pop here

  Edges();
  ~Edges();

  const char *pop();
  void push(const char *path);
};

/// Checks if a file matches the target.
bool check(const char *, const char *fname);

/// Checks for "." and "..".
bool is_dot(const char *);

const char *path_tail(const char *p);
char *path_tail(char *p);

char *path_append(const char *from, const char *to);

