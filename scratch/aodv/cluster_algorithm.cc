// cluster_algorithm.cc
//

#include "cluster_algorithm.h"
#define LZZ_INLINE inline
ClusterAlg::ClusterAlg ()
  : tau (0.1), FEATURE_LENGTH (8), w(0.1)
                                  {}
ClusterAlg::ClusterAlg (double threshold, uint32_t vector_length, double max_width)
                                                  {
    tau = threshold;
    FEATURE_LENGTH = vector_length;
    w = max_width;
  }

void ClusterAlg::Stats (Sample s, Traffic & mean, Traffic & stddev)
                                                        {
    //go through the "position"th element of all the std::vectors and get the mean and push it onto the mean std::vector
    //similarly for double, through all positions
    //put it on the end of the ithElement array
    double ithElements[FEATURE_LENGTH];
    Sample::iterator s_itr;

    for (uint32_t position = 0; position < FEATURE_LENGTH; position++) {
      for (s_itr = s.begin(); s_itr != s.end(); s_itr++) {
        Traffic traffic = *s_itr;
        ithElements[position] = traffic[position];
      }
      assert (i == size);

      double single_mean = gsl_stats_mean(ithElements, 1, FEATURE_LENGTH);
      mean.push_back(single_mean);
      double single_sd = gsl_stats_sd(ithElements, 1, FEATURE_LENGTH);
      stddev.push_back(single_sd);
    }
}
Sample ClusterAlg::Normalization (Sample s)
                                {
  num_samples = s.size();

  Traffic mean, stddev;
  Stats(s, mean, stddev);

  //go through all traffic std::vectors and create map with normalized traffic
  Sample norm_s;

  Sample::iterator s_itr;
  for (s_itr = s.begin(); s_itr != s.end(); s_itr++) {
    Traffic norm_traffic(FEATURE_LENGTH);
    Traffic traffic = *s_itr;

    for (uint32_t i = 0; i < traffic.size(); i++) {
      norm_traffic[i] = (traffic[i] - mean[i])/stddev[i];
    }

    norm_s.push_back(norm_traffic);
  }

  assert (norm_s.size() == s.size());
  return norm_s;
}
Clusters ClusterAlg::FormClusters (Sample s)
                                 {
  Clusters cs;
  //go through all traffic 
  for (Sample::iterator sitr = s.begin(); sitr != s.end(); sitr++) {
    Traffic t = *sitr;
    Cluster c;
    //first traffic becomes centroid of cluster
    if (cs.empty()) {
      c = Cluster(t);
      cs.push_back(c);
    } else {
      //If a sample is in range of a cluster, add it to that cluster.
      Clusters to_add;
      for (Clusters::iterator cs_itr = cs.begin(); cs_itr != cs.end(); cs_itr++) {
        //otherwise, create a new cluster with it
        if (!(cs_itr->add(t))) {
          c = Cluster(t);
          to_add.push_back(c);
          break;
        }
      }
      for (Clusters::iterator to_add_itr = to_add.begin(); to_add_itr != to_add.end(); to_add_itr++) {
        cs.push_back(*to_add_itr);
      }
      to_add.clear();
    }
  }
  return cs;
  }
void ClusterAlg::LabelClusters (Clusters& cs)
                                     {
  //original code operated on cs directly, which doesn't work
  //because a copy is created at some point.

  // create a vector of cluster pointers, so our changes will count                                    
  std::vector<Cluster*> vpc;
  for (Clusters::iterator cs_itr = cs.begin(); cs_itr != cs.end(); cs_itr++) {
    vpc.push_back(&*cs_itr);
  }
  std::vector<Cluster*>::iterator vpc_itr;
  for (vpc_itr = vpc.begin(); vpc_itr != vpc.end(); vpc_itr++) {
    Cluster* p_c = *vpc_itr;
    //to avoid integer division
    double size = p_c->size();
    double sample_count = num_samples;
    p_c->criteria = size/sample_count;

    // std::cout << "cluster sample count: " << c.size() << std::endl;
    // std::cout << "num samples: " << num_samples << std::endl;
    std::cout << "criteria: " << p_c->criteria << std::endl;

    if (p_c->criteria < tau) {
      p_c->anomalous = true;
    } else {
      p_c->anomalous = false;
    }
  }

  for (Clusters::iterator cs_itr = cs.begin(); cs_itr != cs.end(); cs_itr++) {
    std::cout << "Criteria: " << cs_itr->criteria << std::endl;
   } 

  // return cs;
}

#undef LZZ_INLINE
