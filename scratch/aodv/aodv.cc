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

 //TODO: mess with random seed

#include "aodv.h"

using namespace std;

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

//-----------------------------------------------------------------------------
AodvExample::AodvExample () :
  size (5),
  //100 is too large, all packets dropped
  step (50),
  totalTime (50),
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
  w(1)
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
  cmd.AddValue ("w", "Width value for cluster sim", w);

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
  
  //Saves training data to training 
  Training(); 
  // Testing();
}

// void 
// AodvExample::Testing() {
//   ostringstream oss;

//   //total number of observations
//   int obs_count = 0;
//   //number of samples on which the monitor detects an anomaly
//   int anomalous_count = 0;

//   //go through all the monitor nodes. Right now, we have only one,
//   //so this should run once
//   TrafficList traffic_list = training_data.second;

//   TrafficList::iterator tlitr = traffic_list.begin();

//   //go through all the traffic of this monitor node
//   //traffic is taken at regular intervals
//   while (tlitr != traffic_list.end()) {
//     Traffic traffic = *tlitr;

//     RelativeCluster closest_relcluster = Cluster::ClosestRelCluster(traffic, training_clusters);

//     double distance = closest_relcluster.first;
//     Cluster closest_cluster = closest_relcluster.second;

//     obs_count++;
//     //if the closest cluster is less than w away, we label
//     //the sample with the same label
//     if (distance < w) {
//       if (closest_cluster.anomalous) { anomalous_count++; }
//       //otherwise, we consider as not anomalous
//     }

//     tlitr++;
//   }

//   assert (obs_count != 0);


//   oss << "Anomaloust count: " << anomalous_count << std::endl;
//   oss << "Total count: " << obs_count << std::endl;
//   double detection_rate = anomalous_count/obs_count;
//   oss << "Detection rate: " << detection_rate << std::endl;

//   Log(oss);
// }



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


void
AodvExample::TrainingDataTable(Sample s) {
  vector<std::string> headers;
  headers.push_back("RREQ_RECEIVED");
  headers.push_back("RREQ_SENT");
  headers.push_back("RREP_SENT");
  headers.push_back("RREP_FORWARDED");
  headers.push_back("RREP_RECEIVED");
  headers.push_back("RERR_SENT");
  headers.push_back("RERR_RECEIVED");
  headers.push_back("HELLO_SENT");

  Table::CreateTables("TrainingDataTable", headers, s);
}

//cluster algorithm
void
AodvExample::Training() {
  std::ostringstream oss;

  //tau, feature length, w
  ClusterAlg ca = ClusterAlg(0.2, 8, w);
  // labelled_clusters = ca.RunAlgorithm();

  Sample norm_sample = ca.Normalization(sample);

  vector<Cluster> clusters = ca.FormClusters(norm_sample);
  // oss << "Number of clusters: " << clusters.size() << endl;

  // ca.LabelClusters(clusters);
  ca.LabelClusters (clusters);
  Clusters labelled_clusters = clusters;
  // cout << "Criteria \t Anomalous?" << std::endl;

  int numberAnomClusters = 0;
  vector<Cluster>::iterator clusters_itr;
  for (clusters_itr = clusters.begin(); clusters_itr != clusters.end(); clusters_itr++) {
    Cluster c = *clusters_itr;
    // cout << "Criteria: " << c.criteria << "\t" << c.anomalous << std::endl;
    if (clusters_itr->anomalous) {
      numberAnomClusters++;
    }
  }

  oss << w << "\t " << labelled_clusters.size() << "\t "<< numberAnomClusters << std::endl;
  // oss << endl;
  // oss << "Number of anom clusters: " << numberAnomClusters << endl;
  // w_cluster_table(w_values, numCluster, numAnom);
  Log(oss, "RESULT");
}

void
AodvExample::Testing() 
{


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

  Ipv4AddressHelper address;

  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);

  //get monitor data
  for (NodeContainer::Iterator nitr = nodes.Begin(); nitr != nodes.End(); nitr++ ) {

    //get data starting from 10 seconds, every obs_interval seconds
    Ptr<Node> node = *nitr;
    BooleanValue monitor;
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ptr<aodv::RoutingProtocol> routing = ipv4->GetObject<aodv::RoutingProtocol> ();
    
    routing->GetAttribute("Monitor", monitor);

    if (monitor) {
      double obs_interval = 10.0;
      double current_time = 10.0;

      while (current_time <= totalTime) {
        Time Tcurrent_time = Seconds (current_time);
        Simulator::Schedule(Tcurrent_time, &AodvExample::AggregateTraffic, this, node);
        // Simulator::Schedule(Tcurrent_time, &WhatTimeIsIt);
        current_time += obs_interval;
      }
    }
  }

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

  // move node away
  // Ptr<Node> node = nodes.Get (size/2);
  // Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
  // Simulator::Schedule (Seconds (totalTime/3), &MobilityModel::SetPosition, mob, Vector (1e5, 1e5, 1e5));
}

void
AodvExample::AggregateTraffic(Ptr<Node> node) {
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ptr<aodv::RoutingProtocol> routing = ipv4->GetObject<aodv::RoutingProtocol> ();

  Traffic traffic;
  routing->GetMonitoredData(traffic); 

  sample.push_back(traffic);
}

int main (int argc, char **argv)
{
  AodvExample test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();

  // Cluster c; // vector<double> sample; // sample.push_back(0.0);
  // sample.push_back(1.0);

  // pair<int, vector<double> > entry;
  // entry.first = 0;
  // entry.second = sample;

  // c.add(entry);
  return 0;
}
