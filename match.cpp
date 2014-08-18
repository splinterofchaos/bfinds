
#include "match.h"

namespace match {

Matcher eq(const char *target) {
  return [=](const char *fname) {
    return strncmp(target, fname, strlen(target)) == 0;
  };
}

Matcher prefix(const char *target) {
  return [=](const char *fname) {
    return strcmp(target, fname) == 0;
  };
}

Matcher contains(const char *target) {
  return [=](const char *fname) {
    return strstr(fname, target);
  };
}

Matcher regex(const char *target) {
  auto m = std::regex(target, std::regex::optimize);
  return [=](const char *fname) {
    return std::regex_match(fname, m);
  };
}

}
