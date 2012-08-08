#include "common.h"

std::ostream & operator << (std::ostream & out, Traffic t)
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

// std::ostream & operator << (std::ostream & out, Clusters cs)
//                                                                 {
//   std::ostringstream oss;
//   Clusters::iterator csitr = cs.begin();

//   int i = 0;
//   while (csitr != cs.end())
//   {
//     oss << "Cluster " << i << ": " << std::endl;
//     oss << *csitr << std::endl;
//     i++;
//     csitr++;
//   }

//   out << oss.str();
//   return out;
// }
// std::ostream & operator << (std::ostream & out, Traffic const t) {
//   std::ostringstream oss;
//   oss << "< ";

//   Traffic::const_iterator titr = t.begin();
//   while (titr != t.end()) {
//     oss << *titr;
//     titr++;
//     if (titr != t.end())
//       oss << " ";
//   }

//   oss << ">" << std::endl;

//   out << oss.str();
//   return out;
// }

std::ostream & operator << (std::ostream & out, Sample s)
{
  std::ostringstream oss;
  oss << "Sample" << std::endl;
  oss << "----------------------------" << std::endl;
  for (Sample::iterator s_itr = s.begin(); s_itr != s.end(); s_itr++) {
    oss << *s_itr << std::endl;
  }
  oss << "----------------------------" << std::endl;
  out << oss.str();
  return out;
}
