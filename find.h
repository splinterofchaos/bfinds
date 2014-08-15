
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

  bool empty();

  const char *pop();
  void push(const char *path);
};

/*
 * Search engine and context.
 * Usage:
 *    Find f;
 *    f.startpoint(".");
 *    f.target = "foo";
 *    f.run();
 */
struct Find
{
  const char *target;  ///< File to search for.
  Edges unexplored;    ///< Paths to explore.
  size_t count;        ///< Stop after finding this many matches.

  Find();

  /// Adds a starting point to 'unexplored'.
  void startpoint(const char *);

  /// Returns true if 'unexplored' is not empty.
  bool has_startpoint();

  /// Executes the search, printing results to stdout.
  void run();
};

/// Checks if a file matches the target.
bool check(const char *, const char *fname);

/// Checks for "." and "..".
bool is_dot(const char *);

const char *path_tail(const char *p);
char *path_tail(char *p);

char *path_append(const char *from, const char *to);

