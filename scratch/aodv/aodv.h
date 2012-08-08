#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/v4ping-helper.h"
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
#include "cluster_algorithm.h"
#include "table.h"

using namespace ns3;
class AodvExample 
{
public:
  AodvExample ();
  /// Configure script parameters, \return true on successful configuration
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();

private:
  ///\name parameters
  //\{
  /// Number of nodes
  uint32_t size;
  /// Distance between nodes, meters
  double step;
  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  /// Print routes if true
  bool printRoutes;
  double rss;  // -dBm
  uint32_t packetSize; // bytes
  uint32_t numPackets;
  double interval; // seconds
  bool verbose;
  bool malicious;
  bool trace;

  //simulation parameters
  double threshold;

  double w;
  double tau;

  //The only sample in this file, the sample
  //from our ONE monitor node
  Sample sample;
  Clusters labelled_clusters;
  //\}

  ///\name network
  //\{
  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
  //\}

private:
  void w_cluster_table(vector<double> x, vector<int> y, vector<int> z);
  /// Uses training_data to generate training_clusters
  void Training();
  // Attack scenario tested.
  void Testing();
  void Log(std::ostringstream& os, std::string name = "AODV_LOG");
  void AggregateTraffic(Ptr<Node> node);
  void TrainingDataTable(Sample s);

  void CreateNodes ();
  void CreateDevices ();
  //Set monitor and malicious nodes
  //Set callback needed to get data from monitors  
  void InstallInternetStack ();
  //install traffic apps. 
  void InstallApplications ();
};