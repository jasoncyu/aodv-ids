// cluster.cc
//

#include "cluster.h"

#define LZZ_INLINE inline
Cluster::Cluster ()
  : centroid (), count (0), w (1.0) 
                           {}
Cluster::Cluster(Traffic t, double max_width) 
  : count(1)
{
  centroid = t;
  w = max_width;
}
bool Cluster::add (Traffic t)
{
  assert (count > 0);
  std::cout << "Traffic t: " << t << std::endl;
  std::cout << "centroid: " << centroid << std::endl;
  if (Distance(t, centroid) < w) {
    count += 1;
    return true;
  }  
  return false;
}
uint32_t Cluster::size ()
{
  return count;
}
double Cluster::Distance (Traffic t1, Traffic t2) {
 Traffic::iterator itr1 = t1.begin();
 Traffic::iterator itr2 = t2.begin(); 

 assert (t1.size() == t2.size());

 double distance = 0;

 while (itr1 != t1.end() && itr2 != t2.end()) {
   distance += pow( (*itr1 - *itr2), 2);
   itr1++;
   itr2++;
 }
 
 distance = sqrt(distance);
 return distance;
}
#undef LZZ_INLINE
