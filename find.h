
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>

#include <list>
#include <deque>
#include <string>

typedef std::pair<const char *, const char *> Leaf;

typedef std::deque<const char *> Edges;
typedef std::list<Leaf>         Waiting;

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
  Waiting toMatch;     ///< Nodes waiting to be matched.
  std::list<const char *> paths;

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

  static int *search(Find *);
  bool searchOver;
  sem_t sToMatch;
  pthread_cond_t startCond;
  pthread_mutex_t lock;
  pthread_t searcher;
};

/// Checks if a file matches the target.
bool check(const char *, const char *fname);

/// Checks for "." and "..".
bool is_dot(const char *);

const char *path_tail(const char *p);
char *path_tail(char *p);

char *path_append(const char *from, const char *to);

