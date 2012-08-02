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

#include "cluster.h"
#define LZZ_INLINE inline
// using namespace Cluster;

uint32_t const Cluster::FEATURE_LENGTH = 2;

Cluster::Cluster ()
  : centroid (), samples () {}

void Cluster::add (Sample & sample)
                           {
      samples.insert (sample);
      updateCentroid();
  }

void Cluster::updateCentroid ()
                        {

    if (samples.size() == 1) {
      Samples::iterator samples_iterator = samples.begin();
      centroid = samples_iterator->second;
      return;
    }

    //aggregates traffic from each sample into one std::vector
    TrafficList allTraffic;

    Samples::iterator itr;
    for (itr = samples.begin(); itr != samples.end(); itr++) {
      allTraffic.push_back(itr->second);
    }

    //std::vector of all zeros
    Traffic zero;
    for (uint32_t i = 0; i < FEATURE_LENGTH; i++) {
      zero.push_back(0.0);
    }

    Traffic sum = zero;
    for (TrafficList::iterator i = allTraffic.begin(); i != allTraffic.end(); ++i)
    {
      // std::cout << "all traffic for loop\n";
      sum = addTraffic(sum, *i); 
    }

    //divide by number of elements
    for (uint32_t i = 0; i < sum.size(); ++i)
    {
      sum[i] /= allTraffic.size();
    }

    centroid = sum; 
  }
Cluster::Traffic Cluster::addTraffic (Traffic x, Traffic y)
                                           {
    Traffic::iterator itr1 = x.begin();
    Traffic::iterator itr2 = y.begin();

    assert (x.size() == y.size());

    Traffic sum;
    while (itr1 != x.end() && itr2 != y.end()) {
      sum.push_back(*itr1 + *itr2);
      itr1++;
      itr2++;
    }

    return sum;
  }
Cluster::Sample Cluster::outermost ()
                     {
    Samples::iterator samples_itr = samples.begin();
    Sample farthest_sample = *samples_itr; 
    Traffic farthest_traffic = samples_itr->second;
    double farthest_distance = Distance(farthest_traffic, *this);

    for (samples_itr = samples.begin(); samples_itr != samples.end(); samples_itr++) {
      Traffic traffic = samples_itr->second;
      double distance = Distance(traffic, *this);
      if (distance > farthest_distance) {
        farthest_sample = *samples_itr;
        farthest_distance = distance;
      }
    }

    return farthest_sample;
  }
uint32_t Cluster::size ()
                  {
    return samples.size();
  }
double Cluster::Distance (Traffic traffic, Cluster c)
                                                     {
   std::vector<double>::iterator itr1 = traffic.begin();
   std::vector<double>::iterator itr2 = c.centroid.begin(); 

   assert (traffic.size() == c.centroid.size());

   double distance = 0;
   while (itr1 != traffic.end() && itr2 != c.centroid.end()) {
     distance += pow( (*itr1 - *itr2), 2);
     itr1++;
     itr2++;
   }
   
   distance = sqrt(distance);
   return distance;
  }
Cluster::Samples Cluster::Normalization (Samples sample, uint32_t size, std::ostringstream & os)
                                                                                      {
    //normalization

    //calculate mean and std dev
    Traffic mean, stddev;
    //go through the "position"th element of all the std::vectors and get the mean and push it onto the mean std::vector
    //similarly for double, through all positions
    //put it on the end of the ithElement array
    double ithElements[size];
    Samples::iterator sample_itr;

    for (uint32_t position = 0; position < Cluster::FEATURE_LENGTH; position++) {
      uint32_t i = 0;
      for (sample_itr = sample.begin(); sample_itr != sample.end(); sample_itr++) {
        Traffic traffic = sample_itr->second;

        ithElements[i++] = traffic[position];
      }
      assert (i == size);

      double single_mean = gsl_stats_mean(ithElements, 1, Cluster::FEATURE_LENGTH);
      mean.push_back(single_mean);
      double single_sd = gsl_stats_sd(ithElements, 1, Cluster::FEATURE_LENGTH);
      stddev.push_back(single_sd);
    }

    //log means and sd's
    os << "Means: ";

    for (uint32_t i = 0; i < mean.size(); ++i)
    {
      os << mean[i] << "\t ";
    }

    os << "\nStd Devs: ";

    for (uint32_t i = 0; i < stddev.size(); ++i)
    {
      os << stddev[i] << "\t ";
    }
    os <<"\n";

    //go through all traffic std::vectors and create map with normalized traffic
    Samples norm_samples;

    for (sample_itr = sample.begin(); sample_itr != sample.end(); sample_itr++) {
      Traffic norm_traffic(Cluster::FEATURE_LENGTH);
      int num = sample_itr->first;
      Traffic traffic = sample_itr->second;

      for (uint32_t i = 0; i < traffic.size(); i++) {
        norm_traffic[i] = (traffic[i] - mean[i])/stddev[i];
      }

      norm_samples.insert(Sample(num, norm_traffic));
    }

    return norm_samples;
  }
Cluster::Clusters Cluster::FormClusters (Samples norm_samples, double w)
                                                               {
    Clusters clusters;
    Clusters::iterator clusters_itr;
    Samples::iterator norm_samples_itr;

    for (norm_samples_itr = norm_samples.begin(); norm_samples_itr != norm_samples.end(); norm_samples_itr++) {
      Sample sample = *norm_samples_itr; 

      if (clusters.empty()) {
        //if sample is first cluster, then we add it to the cluster set
        Cluster c = Cluster();
        c.add(sample);
        clusters.push_back(c);
      } else {
        Traffic traffic = norm_samples_itr->second;
        Cluster closest_cluster = clusters[0];
        double closest_cluster_distance = Cluster::Distance(traffic, closest_cluster);
        std::cout << "initial closest cluster distance: " << closest_cluster_distance << std::endl;

        //find the nearest cluster to the sample
        for (clusters_itr = clusters.begin(); clusters_itr != clusters.end(); clusters_itr++) {
          double new_distance = Cluster::Distance(traffic, *clusters_itr);
          if (new_distance < closest_cluster_distance) {
            closest_cluster = *clusters_itr;
            closest_cluster_distance = Cluster::Distance(traffic, closest_cluster);
          }
        }

        // std::cout << "cluster distance: " << closest_cluster_distance << std::endl;
        if (closest_cluster_distance < w) {
          //add sample to this cluster
          closest_cluster.add(sample);
        } else {
          //make a new cluster with the new sample
          Cluster c = Cluster();
          c.add(sample);
          clusters.push_back(c);
        }
      }
    }

    return clusters;
  }
Cluster::Clusters Cluster::LabelClusters (Clusters clusters, double threshold, uint32_t size, std::ostringstream & os)
                                                                                                            {
    Clusters::iterator clusters_itr;

    for (clusters_itr = clusters.begin(); clusters_itr != clusters.end(); clusters_itr++) {
      //c_max
      Sample outermost_sample = clusters_itr->outermost();
      Traffic outermost_sample_traffic = outermost_sample.second;
      //w_k
      //TODO: Figure out what this is used for
      // double outermost_sample_width = Cluster::Distance(outermost_sample_traffic, *clusters_itr);

      //If the number of samples in a cluster over the total number of samples is less than the
      //threshold, we label as anomalous

      uint32_t cluster_size = clusters_itr->size();
      double criteria = cluster_size/size;

      if (criteria < threshold) {
        clusters_itr->anomalous = true;
      } else {
        clusters_itr->anomalous = false;
      }
    }


    return clusters;
  }
std::ostream & operator << (std::ostream & out, Cluster const  c)
                                                              {
  std::ostringstream oss;
  oss << "Number of samples: " << c.samples.size() << std::endl;

  Samples::const_iterator sitr = c.samples.begin();
  while (sitr != c.samples.end()) {
    oss << "Node " << sitr->first << ": " << sitr->second;
  }

  oss << std::endl;

  out << oss.str();
  return out;
}
std::ostream & operator << (std::ostream & out, Clusters cs)
                                                                {
  std::ostringstream oss;
  Clusters::iterator csitr = cs.begin();

  int i = 0;
  while (csitr != cs.end())
  {
    oss << "Cluster " << i << " size: " << csitr->size() << std::endl;
    i++;
  }

  out << oss.str();
  return out;
}
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
#undef LZZ_INLINE
