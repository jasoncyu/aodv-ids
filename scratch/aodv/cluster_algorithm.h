// cluster_algorithm.h
//

#ifndef LZZ_cluster_algorithm_h
#define LZZ_cluster_algorithm_h
#define LZZ_INLINE inline

#include "common.h"
#include "cluster.h"

#include <stdint.h>
#include <cmath>
#include <vector>
#include <map>
#include <utility>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_cdf.h>
#include <iostream>
#include <sstream>
#include <cassert>

class ClusterAlg
{
public:
  ClusterAlg ();
  ClusterAlg (double threshold, uint32_t vector_length, double max_width);
  void Stats (Sample s, Traffic & mean, Traffic & stddev);
  Sample Normalization (Sample s);
  Clusters FormClusters (Sample s);
  Clusters& LabelClusters (Clusters& cs);
  Clusters& RunAlgorithm ();
private:
  double tau;
  uint32_t FEATURE_LENGTH;
  double w;
  uint32_t num_samples;
};
#undef LZZ_INLINE
#endif
