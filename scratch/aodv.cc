/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
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
  void Report (std::ostream & os);

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
  test.Report (std::cout);
  return 0;
}

//-----------------------------------------------------------------------------
AodvExample::AodvExample () :
  size (25),
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
  trace(true)
{
}

bool
AodvExample::Configure (int argc, char **argv)
{
  // Enable AODV logs by default. Comment this if too noisy
  LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_FUNCTION);
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

  ofstream outfile;
  // outfile << "sample text " << std::endl;
  outfile.open("aodv.report", ios::app);
  if (!outfile.is_open ()) {
    std::cout << "ERROR: could not open file" << std::endl;
  }
  Report (outfile);
}


void
AodvExample::Report (std::ostream & report)
{ 
  float meanRreqSent = 0, meanRreqReceived = 0, meanRreqDropped = 0;
  float meanRrepSent = 0, meanRrepForwarded = 0, meanRrepReceived = 0;
  float meanRerrSent = 0, meanRerrReceived = 0;
  
  for (NodeContainer::Iterator itr = nodes.Begin(); itr != nodes.End(); ++itr) {
    // Ptr<Node> node = nodes.Get (itr);
    Ptr<Node> node = *itr;
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ptr<aodv::RoutingProtocol> routing = ipv4->GetObject<aodv::RoutingProtocol> ();
     
    meanRreqSent += routing->GetRreqSent();
    meanRreqReceived += routing->GetRreqReceived();
    meanRreqDropped += routing->GetRreqDropped();
    meanRrepSent += routing->GetRrepSent();
    meanRrepForwarded += routing->GetRrepForwarded();
    meanRrepReceived += routing->GetRrepReceived();
    meanRerrSent += routing->GetRerrSent();
    meanRerrReceived += routing->GetRerrReceived();

  }
  // meanRreqSent /= size;
  // meanRreqReceived /= size;
  // meanRreqDropped /= size;
  // meanRrepSent /= size;
  // meanRrepForwarded /= size;
  // meanRrepReceived /= size;
  // meanRerrSent /= size;
  // meanRerrReceived /= size;

  report << "Mean RREQ sent: " << meanRreqSent << std::endl;
  report << "Mean RREQ received: " << meanRreqReceived << std::endl;
  report << "Mean RREQ dropped: " << meanRreqDropped << std::endl;
  report << "Mean RREP sent: " << meanRrepSent << std::endl;
  report << "Mean RREP forwarded: " << meanRrepForwarded << std::endl;
  report << "Mean RREP received: " << meanRrepReceived << std::endl;
  report << "Mean RERR sent: " << meanRerrSent << std::endl;
  report << "Mean RERR received: " << meanRerrReceived << std::endl;
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
  V4PingHelper ping (interfaces.GetAddress (size - 1));
  ping.SetAttribute ("Verbose", BooleanValue (true));
  ping.SetAttribute ("Interval", TimeValue (Seconds (interval)));

  ApplicationContainer p = ping.Install (nodes.Get (0));
  p.Start (Seconds (0));
  p.Stop (Seconds (totalTime) - Seconds (0.001));


  //randomized lots of traffic
  //every 0.5 seconds, two nodes are randomly selected and ping traffic is sent between them.
  //Traffic goes for 5 seconds before stopping.

  UniformVariable r = UniformVariable(0, size - 1);

  for (double startTime = 0; startTime <= totalTime; startTime += 0.5) {
    uint32_t destNode = r.GetInteger(0, size - 1);
    V4PingHelper ping (interfaces.GetAddress (destNode));
    ping.SetAttribute ("Verbose", BooleanValue (true));
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

