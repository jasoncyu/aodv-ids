#include <cmath>
#include <vector>
#include <map>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_cdf.h>
#include <iostream>


struct Cluster{
  const static uint32_t FEATURE_LENGTH = 8;
    //8 elements for the feature vector
  vector<double> centroid;
  std::map<int, vector<double> > samples;
  bool anomalous;

  Cluster() : centroid(), samples() {};

  void add(pair<int, vector<double> >& sample) {
    samples.insert (sample);
    if (samples.size() == 0) { centroid = sample.second; }
    else {
      samples.insert (sample);
      updateCentroid();
    }
  }

  
  void updateCentroid() {
    //aggregates traffic from each sample into one vector
    vector< vector<double> > allTraffic;

    std::map<int, vector<double> >::iterator itr;
    for (itr = samples.begin(); itr != samples.end(); itr++) {
      allTraffic.push_back(itr->second);
    }

    //vector of all zeros
    vector<double> zero;
    for (uint32_t i = 0; i <= FEATURE_LENGTH; i++) {
      zero.push_back(0.0);
    }

    //tried to use accumulate, but no go
    // vector<double> sum = accumulate(allTraffic.begin(), allTraffic.end(), zero, addSamples);

    vector<double> sum = zero;
    for (std::vector<vector<double> >::iterator i = allTraffic.begin(); i != allTraffic.end(); ++i)
    {
      // std::cout << "all traffic for loop\n";
      sum = addSamples(sum, *i); 
    }

    //divide by number of elements
    for (uint32_t i = 0; i < sum.size(); ++i)
    {
      sum[i] /= allTraffic.size();
    }

    centroid = sum; 
  }

  vector<double> addSamples(vector<double> x, vector<double> y) {
    vector<double>::iterator itr1 = x.begin();
    vector<double>::iterator itr2 = y.begin();

    assert (x.size() == y.size());

    vector<double> sum;
    while (itr1 != x.end() && itr2 != y.end()) {
      sum.push_back(*itr1 + *itr2);
      itr1++;
      itr2++;
    }

    return sum;
  }
  
  //returns outermost sample in the cluster
  pair<int, vector<double> > outermost() {
    std::map<int, vector<double> >::iterator samples_itr = samples.begin();
    pair<int, vector<double> > farthest_sample = *samples_itr; 
    vector<double> farthest_traffic = samples_itr->second;
    double farthest_distance = Distance(farthest_traffic, *this);

    for (samples_itr = samples.begin(); samples_itr != samples.end(); samples_itr++) {
      vector<double> traffic = samples_itr->second;
      double distance = Distance(traffic, *this);
      if (distance > farthest_distance) {
        farthest_sample = *samples_itr;
        farthest_distance = distance;
      }
    }

    return farthest_sample;
  } 

  uint32_t size() {
    return samples.size();
  }

  //returns Euclidean distance between a traffic and this cluster
  static double Distance(vector<double> traffic, Cluster c) {
   vector<double>::iterator itr1 = traffic.begin();
   vector<double>::iterator itr2 = c.centroid.begin(); 

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

  static map<int, vector<double> > Normalization(map<int, vector<double> > result, uint32_t size, std::ostringstream& os) {
    //normalization

    //calculate mean and std dev
    vector<double> mean, stddev;
    //go through the "position"th element of all the vectors and get the mean and push it onto the mean vector
    //similarly for double, through all positions
    //put it on the end of the ithElement array
    double ithElements[size];
    std::map<int, vector<double> >::iterator result_itr;

    for (uint32_t position = 0; position < Cluster::FEATURE_LENGTH; position++) {
      uint32_t i = 0;
      for (result_itr = result.begin(); result_itr != result.end(); result_itr++) {
        vector<double> traffic = result_itr->second;

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

    //go through all traffic vectors and create map with normalized traffic
    map<int, vector<double> > norm_results;

    for (result_itr = result.begin(); result_itr != result.end(); result_itr++) {
      vector<double> norm_traffic(Cluster::FEATURE_LENGTH);
      int num = result_itr->first;
      vector<double> traffic = result_itr->second;

      for (uint32_t i = 0; i < traffic.size(); i++) {
        norm_traffic[i] = (traffic[i] - mean[i])/stddev[i];
      }

      norm_results.insert(pair<int, vector<double> >(num, norm_traffic));
    }

    return norm_results;
  };

  static vector<Cluster> FormClusters(map<int, vector<double> > norm_results, double w) {
    vector<Cluster> clusters;
    vector<Cluster>::iterator clusters_itr;
    map<int, vector<double> >::iterator norm_results_itr;

    for (norm_results_itr = norm_results.begin(); norm_results_itr != norm_results.end(); norm_results_itr++) {
      pair<int, vector<double> > sample = *norm_results_itr; 

      if (clusters.empty()) {
        //if sample is first cluster, then we add it to the cluster set
        Cluster c = Cluster();
        c.add(sample);
        clusters.push_back(c);
      } else {
        vector<double> traffic = norm_results_itr->second;
        Cluster closest_cluster = clusters[0];
        double closest_cluster_distance = Cluster::Distance(traffic, closest_cluster);

        //find the nearest cluster to the sample
        for (clusters_itr = clusters.begin(); clusters_itr != clusters.end(); clusters_itr++) {
          double new_distance = Cluster::Distance(traffic, *clusters_itr);
          if (new_distance < closest_cluster_distance) {
            closest_cluster = *clusters_itr;
            closest_cluster_distance = Cluster::Distance(traffic, closest_cluster);
          }
        }
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

  static vector<Cluster> LabelClusters(vector<Cluster> clusters, double threshold, uint32_t size, std::ostringstream& os) {
    vector<Cluster>::iterator clusters_itr;

    for (clusters_itr = clusters.begin(); clusters_itr != clusters.end(); clusters_itr++) {
      //c_max
      pair<int, vector<double> > outermost_sample = clusters_itr->outermost();
      vector<double> outermost_sample_traffic = outermost_sample.second;
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

};
