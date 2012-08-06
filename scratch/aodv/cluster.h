#ifndef LZZ_cluster_h
#define LZZ_cluster_h

#include <cmath>
#include <vector>
#include <map>
#include <utility>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_cdf.h>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <cassert>

struct Cluster
{
  static uint32_t const FEATURE_LENGTH = 7;
  typedef std::pair <int, std::vector<double> > Sample;
  typedef std::vector <double> Traffic;
  typedef std::vector <std::vector<double > > TrafficList;
  typedef std::map <int, std::vector<double> > Samples;
  typedef std::vector <Cluster> Clusters;
  Traffic centroid;
  Samples samples;
  bool anomalous;
  Cluster ();
  void add (Sample & sample);
  void updateCentroid ();
  Traffic addTraffic (Traffic x, Traffic y);
  Sample outermost ();
  uint32_t size ();
  static double Distance (Traffic traffic, Cluster c);
  static Samples Normalization (Samples sample, uint32_t size, std::ostringstream & os);
  static Clusters FormClusters (Samples norm_samples, double w);
  static Clusters LabelClusters (Clusters clusters, double threshold, uint32_t size, std::ostringstream & os);
};


typedef std::pair <int, std::vector<double> > Sample;
typedef std::vector <double> Traffic;
typedef std::vector <std::vector<double > > TrafficList;
typedef std::map <int, std::vector<double> > Samples;
typedef std::vector <Cluster> Clusters;

std::ostream & operator << (std::ostream & out, Cluster const c);
//Clusters
std::ostream & operator << (std::ostream & out, std::vector <Cluster> cs);
//Traffic
std::ostream & operator << (std::ostream & out, std::vector <double> const t);
std::ostream & operator << (std::ostream & out, Sample const s);
#endif
