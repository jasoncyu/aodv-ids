#include <cmath>
#include <vector>
#include <map>

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
  
};
