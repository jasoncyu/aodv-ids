#include <iostream>
#include <cmath>
#include <vector>
#include <functional>
#include <numeric>
#include <cassert>
#include <iomanip>
//I made these
#include "common.h"
#include "cluster.h"
// #include "cluster_algorithm.h"

using namespace std;

// Clusters clusters;
// Clusters*
// RunAlg(double tau, int ftr_len, double w, Sample sample) {
//   //tau, feature length, w
//   ClusterAlg ca = ClusterAlg(tau, ftr_len, w);
//   // labelled_clusters = ca.RunAlgorithm();

//   Sample norm_sample = ca.Normalization(sample);
//   vector<Cluster> clusters = ca.FormClusters(norm_sample);
//   cout << "Number of clusters: " << clusters.size() << endl;

//   // ca.LabelClusters(clusters);
//   ca.LabelClusters (clusters);
//   return &clusters;
//   //one sample centroid should be sole traffic
// }

int main(int argc, char const *argv[])
{
  // double w = 2;
  // double threshold = 0.3;

  vector<double> x, y, z;
  x.push_back(0);
  x.push_back(0);
  y.push_back(1);
  y.push_back(1);
  z.push_back(3);
  z.push_back(3);

  Sample sample;
  sample.push_back(x);
  cout << x << endl;
  cout << sample << endl; 
  // RunAlg(0.2, 8, 1.0, sample); 

  // cout << "One-sample cluster should have centroid as sole traffic vector\n"
  //      << "Expected: <1 1>\n" 
  //      << "Actual: ";
  // cout << endl; 
  // cout << endl; 

  // cout << "Clusters: " << clusters << endl;
  // std::cout << "Distance between x and y\n "
  //           << "Expected: 1.414\n"
  //           << "Actual: " << Cluster::Distance(x, c) ;

  // cout << endl; 
  // cout << endl; 


  // cout << "Adding x should result in a cluster with two elements\n"
  //      << "Expected: 2\n"
  //      << "Actual: " << c.centroid.size() << endl;

  // cout << endl; 
  // cout << endl; 

  // std::pair<int, vector<double> > x_sample = std::pair<int, vector<double> >(0, x);
  // result.insert(x_sample);
  // std::pair<int, vector<double> > z_sample = std::pair<int, vector<double> >(1, z); 
  // result.insert(z_sample);

  // // map<int, vector<double> > norm_result = Cluster::Normalization(result, size, os);

  // vector<Cluster> clusters = Cluster::FormClusters(result, w);
  // // vector<Cluster> labelled_clusters = Cluster::LabelClusters(clusters, threshold, size, os);
  // cout << "Adding z should result in two clusters\n"
  //      << "Expected: 2" << "clusters\n"
  //      << "Actual: " << clusters.size() << " clusters";

  // cout << endl; 
  // cout << endl; 

  return 0; 
};