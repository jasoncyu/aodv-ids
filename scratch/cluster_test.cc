#include "cluster.h"

using namespace std;

int main() {
  uint32_t size = 3; 
  double w = 2;
  double threshold = 0.3;

  vector<double> x, y, z;
  x.push_back(0);
  x.push_back(0);
  y.push_back(1);
  y.push_back(1);
  z.push_back(3);
  z.push_back(3);

  std::map<int, vector<double> > result;
  std::pair<int, vector<double> > y_sample = std::pair<int, vector<double> >(1, y);
  result.insert(y_sample);

  Cluster c;
  c.add(y_sample);

  //one sample centroid should be sole traffic
  cout << "One-sample cluster should have centroid as sole traffic vector\n"
       << "Expected: <1 1>\n" 
       << "Actual: ";
  
  vector<double>::iterator itr ;
  for (itr = c.centroid.begin(); itr != c.centroid.end(); itr++) {
    cout << *itr << " ";
  } 

  cout << endl; 
  cout << endl; 


  std::cout << "Distance between x and y\n "
            << "Expected: 1.414\n"
            << "Actual: " << Cluster::Distance(x, c) ;

  cout << endl; 
  cout << endl; 


  cout << "Adding x should result in a cluster with two elements\n"
       << "Expected: 2\n"
       << "Actual: " << c.centroid.size() << endl;

  cout << endl; 
  cout << endl; 

  std::pair<int, vector<double> > x_sample = std::pair<int, vector<double> >(0, x);
  result.insert(x_sample);
  std::pair<int, vector<double> > z_sample = std::pair<int, vector<double> >(1, z); 
  result.insert(z_sample);

  // map<int, vector<double> > norm_result = Cluster::Normalization(result, size, os);

  vector<Cluster> clusters = Cluster::FormClusters(result, w);
  // vector<Cluster> labelled_clusters = Cluster::LabelClusters(clusters, threshold, size, os);
  cout << "Adding z should result in two clusters\n"
       << "Expected: 2" << "clusters\n"
       << "Actual: " << clusters.size() << " clusters";

  cout << endl; 
  cout << endl; 

  return 0; 
}