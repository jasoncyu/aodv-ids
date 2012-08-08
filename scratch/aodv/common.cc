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


// std::ostream & operator << (std::ostream & out, Cluster const c)
//                                                               {
//   std::ostringstream oss;
//   oss << "Number of samples: " << c.samples.size() << std::endl;

//   Samples::const_iterator sitr = c.samples.begin();
//   while (sitr != c.samples.end()) {
//     oss << "Node " << sitr->first << ": " << sitr->second;
//     sitr++;
//   }

//   oss << std::endl;

//   out << oss.str();
//   return out;
// }
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

// std::ostream & operator << (std::ostream & out, Sample const s)
// {
//   std::ostringstream oss;

//   int num = s.first;
//   Traffic t = s.second;
  
//   oss << "Node " << num << ": " << t << std::endl;

//   out << oss.str();
//   return out;
// }
