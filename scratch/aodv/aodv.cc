/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS *
 * This program is free software; you can redistribute it and/or modify * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This is an example script for AODV manet routing protocol. 
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>
 */

 //TODO: refactor to use the typedefs in cluster

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
#include "table.h"
using namespace ns3;
void ReceivePacket (Ptr<Socket> socket)
{
  NS_LOG_UNCOND ("Received one packet!");
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, 
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
  {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic, 
                           socket, pktSize,pktCount-1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}


// static void WhatTimeIsIt() {
//   std::cout << "It is now " << Simulator::Now().GetSeconds () << " seconds " << std::endl; 
// }  


/**
 * \brief Test script.
 * 
 * This script creates 1-dimensional grid topology and then ping last node from the first one: *
 * [10.0.0.1] <-- step --> [10.0.0.2] <-- step --> [10.0.0.3] <-- step --> [10.0.04]
 * 
 * ping 10.0.0.4
 */
class AodvExample 
{
public:
  AodvExample ();
  /// Configure script parameters, \return true on successful configuration
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /// Report results
  Samples Stats ();
  void w_cluster_table(vector<double> x, vector<int> y, vector<int> z);
  void Training(std::map<int, vector<double> > result);
  void Log(std::ostringstream& os, std::string name = "aodv.report");
  void LogTraffic(std::map<int, vector<double> > result);
  void Testing();
  void AggregateTraffic(Ptr<aodv::RoutingProtocol> routing, TrafficList traffic_list);
  void TrainingDataTable();

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

  double w_max;
  double w_step;

  //traffic samples after being extracted
  Samples samples;
  //labelled clusters resulting from normal traffic sim
  Clusters training_clusters;
  // map of (nodeID, trafficlist)
  TrainingData training_data;
  //\}

  ///\name network
  //\{
  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
  //\}

private:
  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
};

int main (int argc, char **argv)
{
  AodvExample test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();

  // Cluster c;
  // vector<double> sample;
  // sample.push_back(0.0);
  // sample.push_back(1.0);

  // pair<int, vector<double> > entry;
  // entry.first = 0;
  // entry.second = sample;

  // c.add(entry);
  return 0;
}

//-----------------------------------------------------------------------------
AodvExample::AodvExample () :
  size (5),
  //100 is too large, all packets dropped
  step (50),
  totalTime (100),
  pcap (true),
  printRoutes (true),
  rss(-80),
  packetSize(1000),
  numPackets(300),
  interval(0.5),
  verbose(false),
  malicious(false),
  trace(true),
  threshold(0.10),
  w_max(4),
  w_step(0.1)
{
}

bool
AodvExample::Configure (int argc, char **argv)
{
  // Enable AODV logs by default. Comment this if too noisy
  // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_FUNCTION);
  // LogComponentEnable("V4Ping", LOG_LEVEL_ALL);

  SeedManager::SetSeed (12345);
  CommandLine cmd;

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
  cmd.AddValue ("size", "Number of nodes.", size);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("step", "Grid step, m", step);
  cmd.AddValue ("mal", "Run simulation with second-to-last node maliciuos", malicious);

  cmd.Parse (argc, argv);
  return true;
}

void
AodvExample::Run ()
{
//  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  Simulator::Destroy ();

  //generates the training data needed
  samples = Stats();
  
  //Saves training data to training 
  Training (samples);
  TrainingDataTable();
  // Testing();
}

void 
AodvExample::Testing() {

  // //total number of observations
  // int obs_count = 0;
  // //number of samples on which the monitor detects an anomaly
  // int anomalous_count = 0;

  // for (double w = 0; w <= w_max; w += w_step) {
  //   RelativeCluster closest_relcluster = ClosestCluster(sample, clusters);

  //   double distance = closest_relcluster.first;

  //   if (distance < w) {
  //     anomalous_count++;
  //   }
  }



  // while (multiplier <= total_obs) {  
    // Simulator::Schedule(Secon)
  // }
  //every monitor_step seconds,
  //extract data from monitor node and reset monitor node data 
  //label as same label as closest cluster if it is less than w away
  //otherwise, label as anomalous

  //did we detect attack traffic?

void 
AodvExample::Log(std::ostringstream& oss, std::string name)
{
  std::ofstream report;
  report.open(name.c_str(), ios::app);
  if (!report.is_open ()) {
    std::cout << "ERROR: could not open file" << std::endl;
  }
  report << oss.str() << std::endl; 
  report.close();
}

//returns (node #, traffic vector (vector<double))

Samples
AodvExample::Stats()
{ 
  Samples samples;
  
  for (NodeContainer::Iterator itr = nodes.Begin(); itr != nodes.End(); ++itr) {
    Ptr<Node> node = *itr;
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ptr<aodv::RoutingProtocol> routing = ipv4->GetObject<aodv::RoutingProtocol> ();
    BooleanValue monitor;
    routing->GetAttribute("Monitor", monitor);
    if (!monitor)
      continue;

    Traffic traffic;
    routing->GetMonitoredData(traffic);

    samples.insert(std::pair<int, Traffic>(node->GetId (), traffic));
  }
  return samples;
}
//takes a sample
void
AodvExample::LogTraffic(std::map<int, vector<double> > result)
{
  std::ostringstream os;

  std::map<int, vector<double> >::iterator result_itr;
  for (result_itr = result.begin(); result_itr != result.end(); result_itr++) {
    int num = result_itr->first;
    vector<double> traffic = result_itr->second;

    std::vector<double>::iterator traffic_itr;
    os << "Node " << num << ": "; 

    for (traffic_itr = traffic.begin(); traffic_itr != traffic.end(); traffic_itr++) {
      os << *traffic_itr;
      if (traffic_itr + 1 != traffic.end()) {
       os << "\t "; 
      } 

    }

    os << "\n";
  }

  Log(os);
}


void
AodvExample::TrainingDataTable() {
  vector<std::string> headers;
  headers.push_back("RREQ_RECEIVED");
  headers.push_back("RREQ_SENT");
  headers.push_back("RREP_SENT");
  headers.push_back("RREP_FORWARDED");
  headers.push_back("RREP_RECEIVED");
  headers.push_back("RERR_SENT");
  headers.push_back("RERR_RECEIVED");
  headers.push_back("HELLO_SENT");

  Table::CreateTables("aodv.training", headers, training_data);
}
//cluster algorithm
void
AodvExample::Training(Samples samples) {
  LogTraffic(samples);

  std::ostringstream oss;

  std::cout << "Number of samples should be 1. Actual: " << samples.size() << std::endl;
  map<int, vector<double> > norm_samples = Cluster::Normalization(samples, size, oss);
  LogTraffic(norm_samples);

  std::ofstream report;
  //wipe out old report
  report.open("aodv.clusters");
  report.close();

  vector<double> w_values;
  vector<int> numCluster, numAnom;

  for (double w = 0; w <= w_max; w += w_step) {
    vector<Cluster> clusters = Cluster::FormClusters(norm_samples, w);
    vector<Cluster> labelled_clusters = Cluster::LabelClusters(clusters, threshold, size, oss);

    training_clusters = labelled_clusters;

    int numberAnomClusters = 0;
    vector<Cluster>::iterator clusters_itr;
    for (clusters_itr = labelled_clusters.begin(); clusters_itr != labelled_clusters.end(); clusters_itr++) {
      if (clusters_itr->anomalous) {
        numberAnomClusters++;
        std::map<int, vector<double> >::iterator samples_itr;
        std::map<int, vector<double> > samples = clusters_itr->samples;

        // for (samples_itr = samples.begin(); samples_itr != samples.end(); samples_itr++) {
        //   os << "Node " << samples_itr->first << " is anomalous" << std::endl;
        // }
      }
    }

    oss << "Number of anomalous clusters: " << numberAnomClusters << std::endl;

    w_values.push_back(w);
    numCluster.push_back(clusters.size());
    numAnom.push_back(numberAnomClusters);
  }

  w_cluster_table(w_values, numCluster, numAnom);
  Log(oss);
}

//plots w|clusters_total|clusters_anon
//plots the vectors it's given in a table
void 
AodvExample::w_cluster_table(vector<double> x, vector<int> y, vector<int> z)
{
  ostringstream oss;

  //comment character for gnuplot
  oss << "#";
  vector<string> headers; 
  std::string header1, header2, header3;
  header1 = "w";
  header2 = "# of clusters";
  header3 = "# of anon clusters" ;

  headers.push_back(header1);  
  headers.push_back(header2);  
  headers.push_back(header3);  

  //create table headers

  vector<string>::iterator itr = headers.begin();
  while (itr != headers.end()) {
    oss << setw(5) << *itr;
    itr++;
    if (itr != headers.end())
      oss << " | ";
  }

  oss << "\n";

  // int columns = headers.size();
  vector<double>::iterator x_itr = x.begin();
  vector<int>::iterator y_itr = y.begin();
  vector<int>::iterator z_itr = z.begin();

  while (x_itr != x.end()) {
    oss << setw(header1.size()) << *x_itr;
    oss << setw(header2.size()) << *y_itr;
    oss << setw(header3.size()) << *z_itr << std::endl;

    x_itr++;
    y_itr++;
    z_itr++;
  }

  ofstream report;

  report.open("aodv.clusters", ios::app);
  if (!report.is_open ()) {
    std::cout << "ERROR: could not open file" << std::endl;
  }

  report << oss.str() << std::endl; 
  report.close();

}
void
AodvExample::CreateNodes () {
  std::cout << "Creating " << (unsigned)size << " nodes " << step << " m apart.\n";
    nodes.Create (size);
  // Name nodes
  for (uint32_t i = 0; i < size; ++i)
    {
      std::ostringstream os;
      os << "node-" << i;
      Names::Add (os.str (), nodes.Get (i));
    }
  // Create static grid
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (step),
                                 "DeltaY", DoubleValue (step),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  // mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
    "Bounds", RectangleValue (Rectangle (-1.0 * step, 6.0 * step, -1.0*step, 6.0*step)), 
    "Speed", RandomVariableValue (UniformVariable (2.0, 55.0)));
  mobility.Install (nodes);
}

void
AodvExample::CreateDevices ()
{
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  devices = wifi.Install (wifiPhy, wifiMac, nodes); 

  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("aodv"));
    }
}

void
AodvExample::InstallInternetStack ()
{
  AodvHelper aodv;
  Ptr<Node> monitor_node = nodes.Get (size - 3);
  monitor_node->SetAttribute ("Monitor", BooleanValue (true));
  // you can configure AODV attributes here using aodv.Set(name, value)
  if (malicious) {
    Ptr<Node> mal_node = nodes.Get (size - 2);
    mal_node->SetAttribute ("Malicious", BooleanValue (true));
  }
  
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv); // has effect on the next Install ()
  stack.Install (nodes);

  // Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  // Ptr<Ipv4RoutingProtocol> ipv4Routing = m_routing->Create (node);
  Ipv4AddressHelper address;

  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);

  if (printRoutes)
    {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
      aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);
    }
}


void
AodvExample::InstallApplications ()
{
  // ApplicationContainer p = ping.Install (nodes.Get (0));
  // p.Start (Seconds (0));
  // p.Stop (Seconds (totalTime) - Seconds (0.001));


  //randomized lots of traffic
  //every 0.5 seconds, two nodes are randomly selected and ping traffic is sent between them.
  //Traffic goes for 5 seconds before stopping.

  UniformVariable r = UniformVariable(0, size - 1);
  for (double startTime = 0; startTime <= totalTime; startTime += 0.5) {
    uint32_t destNode = r.GetInteger(0, size - 1);
    V4PingHelper ping (interfaces.GetAddress (destNode));
    // ping.SetAttribute ("Verbose", BooleanValue (true));
    ping.SetAttribute ("Interval", TimeValue (Seconds (interval)));

    uint32_t sourceNode = r.GetInteger(0, size - 1);

    while (sourceNode == destNode) {
      sourceNode = r.GetInteger(0, size - 1);
    } 

    ApplicationContainer p = ping.Install (nodes.Get (sourceNode));
    p.Start (Seconds (0));
    p.Stop (Seconds (totalTime) - Seconds (0.001));
  }

  //schedule getting traffic from monitor node 
  // Time obs_interval = Seconds (interval);

  for (NodeContainer::Iterator nitr = nodes.Begin(); nitr != nodes.End(); nitr++ ) {

    //get data starting from 10 seconds, every obs_interval seconds
    Ptr<Node> node = *nitr;
    BooleanValue monitor;
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ptr<aodv::RoutingProtocol> routing = ipv4->GetObject<aodv::RoutingProtocol> ();
    
    routing->GetAttribute("Monitor", monitor);

    if (monitor) {
      TrafficList traffic_list;
      double obs_interval = 10.0;
      double current_time = 10.0;

      while (current_time <= totalTime) {
        // Time Tcurrent_time = Seconds (current_time);
        // Simulator::Schedule(Tcurrent_time, &GetMonitoredData, routing, traffic_list);
        // Simulator::Schedule(Tcurrent_time, &WhatTimeIsIt);
        current_time += obs_interval;
      }

      TrainingDatum td(node->GetId (), traffic_list);
      training_data.insert(td);
    }
  }
  // move node away
  // Ptr<Node> node = nodes.Get (size/2);
  // Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
  // Simulator::Schedule (Seconds (totalTime/3), &MobilityModel::SetPosition, mob, Vector (1e5, 1e5, 1e5));
}

void
AodvExample::AggregateTraffic(Ptr<aodv::RoutingProtocol> routing, TrafficList traffic_list) {
  Traffic traffic;
  routing->GetMonitoredData(traffic); 
  traffic_list.push_back(traffic);
}
