// cluster.h
//

#ifndef LZZ_cluster_h
#define LZZ_cluster_h
#define LZZ_INLINE inline
struct Cluster
{
  static uint32_t const FEATURE_LENGTH;
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
std::ostream & operator << (std::ostream & out, Cluster const c);
std::ostream & operator << (std::ostream & out, Clusters const cs);
std::ostream & operator << (std::ostream & out, Traffic const t);
#undef LZZ_INLINE
#endif
