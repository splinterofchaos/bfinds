
#include <dirent.h>

#include <deque>
#include <string>

typedef std::deque<const char *> Edges;

/*
 * Search engine and context.
 * Usage:
 *    Find f;
 *    f.startpoint(".");
 *    f.startpoint("/dir");
 *    f.target = "foo";
 *    std::string match;
 *    while(f.next(match))
 *      ...
 */
struct Find
{
  const char *target;  ///< File to search for.
  Edges unexplored;    ///< Paths to explore.

  Find();
  ~Find();

  /// Adds a starting point to 'unexplored'.
  void startpoint(const char *);

  /// Returns true if 'unexplored' is not empty.
  bool has_startpoint();

  /// Executes the search.
  /// Call repeatedly to get more results.
  bool next(std::string &);

  /// Overload of next() that returns "" instead of false.
  std::string next();

private:
  // The state of the search:
  const char *path;  ///< The path being expanded.
  DIR *d;            ///< path's directory.

  const char *in_path();  ///< Increments `d`.
  const char *in_ent(struct dirent *);
};

/// Checks if a file matches the target.
bool check(const char *, const char *fname);

/// Checks for "." and "..".
bool is_dot(const char *);

const char *path_tail(const char *p);
char *path_tail(char *p);

char *path_append(const char *from, const char *to);

