
#include <dirent.h>

#include <deque>
#include <string>
#include <memory>

#include "match.h"

#ifndef FIND_H
#define FIND_H

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
  const char *target = nullptr;  ///< File to search for.
  Edges unexplored;              ///< Paths to explore.

  Find();
  ~Find();

  /// Sets the match generator. (By default, match::prefix.)
  void match_by(match::Gen);

  /// Adds a starting point to 'unexplored'.
  void startpoint(const char *);

  /// Returns true if 'unexplored' is not empty.
  bool has_startpoint();

  /// Executes the search.
  /// Call repeatedly to get more results.
  bool next(std::string &);

private:
  match::Gen matchGenerator = match::prefix;
  match::Matcher check;  ///< Initialized by next().

  // The state of the search:
  const char *path = nullptr;  ///< The path being expanded.
  DIR *d = nullptr;            ///< path's directory.

  const char *in_path();  ///< Increments `d`.
  const char *in_ent(struct dirent *);
};

/// Checks for "." and "..".
bool is_dot(const char *);

const char *path_tail(const char *p);
char *path_tail(char *p);

char *path_append(const char *from, const char *to);

#endif

