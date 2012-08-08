#ifndef common_h
#define common_h 

#include <vector>
#include <map>
#include <cmath>
#include <cassert>
#include <sstream>

//forward definition needed for below
class Cluster;

typedef std::vector<double> Traffic;
typedef std::vector <std::vector<double > > Sample;
typedef std::vector <Cluster> Clusters;
typedef std::pair<double,Cluster> RelativeCluster;

std::ostream & operator << (std::ostream & out, Traffic const t);
std::ostream & operator << (std::ostream & out, Cluster const  c);
std::ostream & operator << (std::ostream & out, Clusters cs);
std::ostream & operator << (std::ostream & out, Traffic const t);
std::ostream & operator << (std::ostream & out, Sample const s);
#endif