
/**
 * Matchers for the find module.
 *
 * A generator takes in a target value and produces a predicate function taking
 * a c-string.
 */

#include <functional>
#include <regex>

#include <string.h>

namespace match {

using Matcher = std::function<bool(const char *)>;
using Gen = Matcher(*)(const char *);  // Function that generates a matcher.

// Generators.
Matcher eq(const char *target);
Matcher prefix(const char *target);
Matcher contains(const char *target);
Matcher regex(const char *target);

}
