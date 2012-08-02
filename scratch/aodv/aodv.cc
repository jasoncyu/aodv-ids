/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
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
//I made these
#include "cluster.h"
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

  

/**
 * \brief Test script.
 * 
 * This script creates 1-dimensional grid topology and then ping last node from the first one:
 * 
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
  std::map<int, vector<double> > Stats ();
  void Process(std::map<int, vector<double> > result);
  void Log(std::ostringstream& os);
  void LogTraffic(std::map<int, vector<double> > result);

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
  //threshold
  double w;
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
  totalTime (10),
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
  w(1000.0)
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

  std::map<int, vector<double> > samples = Stats();
  Process (samples);
}

void 
AodvExample::Log(std::ostringstream& os) 
{
  std::ofstream report;
  report.open("aodv.report", ios::app);
  if (!report.is_open ()) {
    std::cout << "ERROR: could not open file" << std::endl;
  }
  report << os.str() << std::endl; 
  report.close();
}

//returns (node #, traffic vector (vector<double))
std::map<int, vector<double> >
AodvExample::Stats()
{ 
  double meanRreqSent = 0, meanRreqReceived = 0, meanRreqDropped = 0;
  double meanRrepSent = 0, meanRrepForwarded = 0, meanRrepReceived = 0;
  double meanRerrSent = 0, meanRerrReceived = 0;
  std::map<int, vector<double> > result;
  
  for (NodeContainer::Iterator itr = nodes.Begin(); itr != nodes.End(); ++itr) {
    double rreqSent = 0, rreqReceived = 0, rreqDropped = 0;
    double rrepSent = 0, rrepForwarded = 0, rrepReceived = 0;
    double rerrSent = 0, rerrReceived = 0;
    vector<double> traffic;

    Ptr<Node> node = *itr;
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ptr<aodv::RoutingProtocol> routing = ipv4->GetObject<aodv::RoutingProtocol> ();

    rreqSent      = routing->GetRreqSent();
    rreqReceived  = routing->GetRreqReceived();
    rreqDropped   = routing->GetRreqDropped();
    rrepSent      = routing->GetRrepSent();
    rrepForwarded = routing->GetRrepForwarded();
    rrepReceived  = routing->GetRrepReceived();
    rerrSent      = routing->GetRerrSent();
    rerrReceived  = routing->GetRerrReceived();

    traffic.push_back(rreqSent     );
    traffic.push_back(rreqReceived );
    traffic.push_back(rreqDropped  );
    traffic.push_back(rrepSent     );
    traffic.push_back(rrepForwarded);
    traffic.push_back(rrepReceived );
    traffic.push_back(rerrSent     );
    traffic.push_back(rerrReceived );

    int id = node->GetId ();
    result.insert(pair<int, vector<double> >(id, traffic));

    meanRreqSent      += rreqSent;
    meanRreqReceived  += rreqReceived;
    meanRreqDropped   += rreqDropped;
    meanRrepSent      += rreqSent;
    meanRrepForwarded += rrepForwarded;
    meanRrepReceived  += rrepReceived;
    meanRerrSent      += rerrSent;
    meanRerrReceived  += rerrReceived;
  }
  meanRreqSent /= size;
  meanRreqReceived /= size;
  meanRreqDropped /= size;
  meanRrepSent /= size;
  meanRrepForwarded /= size;
  meanRrepReceived /= size;
  meanRerrSent /= size;
  meanRerrReceived /= size;

  ostringstream os;


  os << "Mean RREQ sent: " << meanRreqSent << std::endl
     << "Mean RREQ received: " << meanRreqReceived << std::endl
     << "Mean RREQ dropped: " << meanRreqDropped << std::endl
     << "Mean RREP sent: " << meanRrepSent << std::endl
     << "Mean RREP forwarded: " << meanRrepForwarded << std::endl
     << "Mean RREP received: " << meanRrepReceived << std::endl
     << "Mean RERR sent: " << meanRerrSent << std::endl
     << "Mean RERR received: " << meanRerrReceived << std::endl;

  Log(os);

  return result;
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

//cluster algorithm
void
AodvExample::Process(std::map<int, vector<double> > result) {
  LogTraffic(result);

  std::ostringstream os;

  map<int, vector<double> > norm_result = Cluster::Normalization(result, size, os);
  LogTraffic(norm_result);
  
  vector<Cluster> clusters = Cluster::FormClusters(norm_result, w);
  
  std::cout << "clusters: " << std::endl;
  // std::cout << clusters << std::endl;

  vector<Cluster> labelled_clusters = Cluster::LabelClusters(clusters, threshold, size, os);

  //report on anomalous clusters
  os << "number of clusters: " << labelled_clusters.size() << std::endl;

  vector<Cluster>::iterator clusters_itr;
  for (clusters_itr = labelled_clusters.begin(); clusters_itr != labelled_clusters.end(); clusters_itr++) {
    if (clusters_itr->anomalous) {
      std::map<int, vector<double> >::iterator samples_itr;
      std::map<int, vector<double> > samples = clusters_itr->samples;

      for (samples_itr = samples.begin(); samples_itr != samples.end(); samples_itr++) {
        os << "Node: " << samples_itr->first << " anomalous" << std::endl;
      }
    }
    os << "not anomalous\n";
  }

  Log(os);
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
  // new traffic
  // TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  // Ptr<Socket> recvSink = Socket::CreateSocket (nodes.Get (0), tid);
  // InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  // recvSink->Bind (local);
  // recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  // Ptr<Socket> source = Socket::CreateSocket (nodes.Get (size - 1), tid);
  // InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
  // source->SetAllowBroadcast (true);
  // source->Connect (remote);

  // // NS_LOG_UNCOND ("Testing " << numPackets  << " packets sent with receiver rss " << rss );

  // Time interPacketInterval = Seconds (interval);

  // Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
  //                                 Seconds (2.0), &GenerateTraffic, 
  //                                 source, packetSize, numPackets, interPacketInterval);

  //mobility
  // Ptr<Node> node = nodes.Get (size/2);
  // Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
  // Simulator::Schedule (Seconds (1.0), &MobilityModel::SetPosition, mob, Vector(100, 50, 0));


  //original traffic
  // V4PingHelper ping (interfaces.GetAddress (size - 1));
  // ping.SetAttribute ("Verbose", BooleanValue (true));
  // ping.SetAttribute ("Interval", TimeValue (Seconds (interval)));

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
  // move node away
  // Ptr<Node> node = nodes.Get (size/2);
  // Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
  // Simulator::Schedule (Seconds (totalTime/3), &MobilityModel::SetPosition, mob, Vector (1e5, 1e5, 1e5));
}

