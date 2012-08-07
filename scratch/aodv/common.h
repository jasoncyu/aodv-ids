#ifndef common_h
#define common_h 

#include <vector>
#include <map>

class Cluster;

typedef std::pair <int, std::vector<double> > Sample;
typedef std::vector <double> Traffic;
typedef std::vector <std::vector<double > > TrafficList;
typedef std::map <int, std::vector<double> > Samples;
typedef std::vector <Cluster> Clusters;
typedef std::pair<double,Cluster> RelativeCluster;
typedef std::pair <int, TrafficList> TrainingDatum;
typedef std::map <int, TrafficList> TrainingData;

#endif