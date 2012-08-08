#include "common.h"

std::ostream & operator << (std::ostream & out, Traffic const t)
{
  std::ostringstream oss;
  oss << "< ";

  Traffic::const_iterator titr = t.begin();
  while (titr != t.end()) {
    oss << *titr;
    titr++;
    if (titr != t.end())
      oss << " ";
  }

  oss << ">" << std::endl;

  out << oss.str();
  return out;
}
