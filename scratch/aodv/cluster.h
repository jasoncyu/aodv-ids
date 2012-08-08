// cluster.h
//
#ifndef LZZ_cluster_h
#define LZZ_cluster_h
#define LZZ_INLINE inline

#include "common.h"
#include <iostream>
#include <stdint.h>
struct Cluster
{
  Traffic centroid;
  uint32_t count;
  double w;
  bool anomalous;

  //used for inspection afterwards;
  double criteria;

  uint32_t size ();
  bool add (Traffic t);
  Cluster ();
  Cluster(Traffic t);
  static double Distance (Traffic t1, Traffic t2);
};
#undef LZZ_INLINE

#endif