/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

// Default Network Topology
//
// Number of wifi or csma nodes can be increased up to 250
//                          |
//                 Rank 0   |   Rank 1
// -------------------------|----------------------------
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");


class MyApp : public Application 
{
public:

  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0), 
    m_peer (), 
    m_packetSize (0), 
    m_nPackets (0), 
    m_dataRate (0), 
    m_sendEvent (), 
    m_running (false), 
    m_packetsSent (0)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void 
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void 
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void 
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

// Variables for calculating throughput
uint32_t udp_count = 0;
uint32_t tcp_count = 0;
uint32_t udp_data_in_window[100] = { 0 };
uint32_t tcp_data_in_window[100] = { 0 };
double udp_time_window[100] = { 0.0 };
double tcp_time_window[100] = { 0.0 };

// Callback function for UDP RX variable
static void
CalcThru_udp(Ptr<const Packet> pkt, const Address& addr) {
  double throughput = 0; 

  // Save Nowtime and Packet size
  udp_data_in_window[udp_count % 100] = pkt->GetSize();
  udp_time_window[udp_count % 100] = Simulator::Now ().GetSeconds();

  // No sufficient data for throughput
  if (udp_count < 100) {
    for (unsigned int i = 0; i <= udp_count % 100; i++) {
      throughput += udp_data_in_window[i];
    }
  }
  // Sum up all data in window
  else {
    for (unsigned int i = 0; i < 100; i++) {
      throughput += udp_data_in_window[i];
    }
  }

  // caculation of throughput 
  throughput = throughput / (udp_time_window[udp_count % 100] - udp_time_window[(udp_count + 1) % 100]) * 8.0 / 1000000.0;

  NS_LOG_UNCOND (Simulator::Now ().GetSeconds() << "\t" << throughput << "\t" << "UDP");

  udp_count++;
  return;
}

// This has same behaviour with UDP 
static void
CalcThru_tcp(Ptr<const Packet> pkt, const Address& addr) {
  double throughput = 0; 

  tcp_data_in_window[tcp_count % 100] = pkt->GetSize();
  tcp_time_window[tcp_count % 100] = Simulator::Now ().GetSeconds();

  if (tcp_count < 100) {
    for (unsigned int i = 0; i <= tcp_count % 100; i++) {
      throughput += tcp_data_in_window[i];
    }
  }
  else {
    for (unsigned int i = 0; i < 100; i++) {
      throughput += tcp_data_in_window[i];
    }
  }

  throughput = throughput / (tcp_time_window[tcp_count % 100] - tcp_time_window[(tcp_count + 1) % 100]) * 8.0 / 1000000.0;

  NS_LOG_UNCOND (Simulator::Now ().GetSeconds() << "\t" << throughput << "\t" << "TCP");

  tcp_count++;
  return;
}

// RTS failed
uint32_t rtsFailed = 0;
static void
MacTxRtsFailed (Mac48Address addr) {
    rtsFailed++;
}


int 
main (int argc, char *argv[])
{
  bool tracing = true;
  uint32_t nCsma = 3;
  uint32_t nWifi = 9;
  uint32_t thre = 2000;
  uint32_t protocol = 0;
  
  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("thre", "threshold of CTS/RTS", thre);
  cmd.AddValue ("protocol", "TCP or UDP", protocol);
  cmd.Parse (argc,argv);

  // Check for valid number of csma or wifi nodes
  // 250 should be enough, otherwise IP addresses 
  // soon become an issue
  if (nWifi > 250 || nCsma > 250)
    {
      std::cout << "Too many wifi or csma nodes, no more than 250 each." << std::endl;
      return 1;
    }

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  // P2P channel setting
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("20Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  // CSMA channel setting
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  // WIFI channel setting
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  // Set channel delay, loss model
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel"); 
  channel.AddPropagationLoss("ns3::JakesPropagationLossModel");
  phy.SetChannel (channel.Create ());

  // Set ARF threshold
  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::ArfWifiManager", "RtsCtsThreshold", UintegerValue(thre));

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  // Enable tracing source for RTS failed
  Ptr<WifiRemoteStationManager> st1 = DynamicCast<WifiNetDevice> (apDevices.Get (0))->GetRemoteStationManager ();
  st1->TraceConnectWithoutContext ("MacTxRtsFailed", MakeCallback (&MacTxRtsFailed));

  for (uint32_t i = 0; i < nWifi; i++) 
  {
    Ptr<WifiRemoteStationManager> st1 = DynamicCast<WifiNetDevice> (staDevices.Get (i))->GetRemoteStationManager ();
    st1->TraceConnectWithoutContext ("MacTxRtsFailed", MakeCallback (&MacTxRtsFailed));
  }


  MobilityHelper mobility;

  // Original code
  // mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
  //                                "MinX", DoubleValue (0.0),
  //                                "MinY", DoubleValue (0.0),
  //                                "DeltaX", DoubleValue (5.0),
  //                                "DeltaY", DoubleValue (10.0),
  //                                "GridWidth", UintegerValue (3),
  //                                "LayoutType", StringValue ("RowFirst"));

  // mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
  //                            "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  // mobility.Install (wifiStaNodes);

  // mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // mobility.Install (wifiApNode);

  // Set STA nodes mobility
  MobilityHelper Stamobility;
  Stamobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
          "rho", DoubleValue (30.0));
  Stamobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue(Rectangle(-30,30,-30,30)));
  Stamobility.Install (wifiStaNodes);

  // Set Ap mobility
  MobilityHelper Apmobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  Apmobility.SetPositionAllocator (positionAlloc);
  Apmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  Apmobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);

  // Original code
  // UdpEchoServerHelper echoServer (9);

  // ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
  // serverApps.Start (Seconds (1.0));
  // serverApps.Stop (Seconds (10.0));

  // UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
  // echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  // echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  // echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  // ApplicationContainer clientApps = 
  //   echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  // clientApps.Start (Seconds (2.0));
  // clientApps.Stop (Seconds (10.0));

  // Install TCP sink app
  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (csmaInterfaces.GetAddress (3), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (csmaNodes.Get (3));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (12.));

  // Install UDP sink app
  uint16_t sinkPort_udp = 9;
  Address sinkAddress_udp (InetSocketAddress (csmaInterfaces.GetAddress (2), sinkPort_udp));
  PacketSinkHelper packetSinkHelper_udp ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort_udp));
  ApplicationContainer sinkApps_udp = packetSinkHelper_udp.Install (csmaNodes.Get (2));
  sinkApps_udp.Start (Seconds (0.));
  sinkApps_udp.Stop (Seconds (12.));

  // Enable TCP only or UDP only
  // 0 : TCP
  // 1 : UDP
  if (protocol == 0) 
    sinkApps.Get (0)->TraceConnectWithoutContext ("Rx", MakeCallback (&CalcThru_tcp));
  else
    sinkApps_udp.Get(0)->TraceConnectWithoutContext ("Rx", MakeCallback (&CalcThru_udp));

  // Install TCP traffic app on 1st wifi node
  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (wifiStaNodes.Get (0), TcpSocketFactory::GetTypeId ());
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 400, 100000, DataRate ("5Mbps"));
  wifiStaNodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (10.));

  // Install UDP traffic app on other wifi STA nodes
  for (unsigned int i = 1; i < nWifi; i++) {
    Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (wifiStaNodes.Get (i), UdpSocketFactory::GetTypeId ());
    Ptr<MyApp> app_udp = CreateObject<MyApp> ();
    app_udp->Setup (ns3UdpSocket, sinkAddress_udp, 400, 100000, DataRate ("3Mbps"));
    wifiStaNodes.Get (i)->AddApplication (app_udp);
    app_udp->SetStartTime (Seconds (2.));
    app_udp->SetStopTime (Seconds (7.));
    }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (13.0));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("p2p");
      phy.EnablePcap ("AP", apDevices.Get (0));
      csma.EnablePcap ("CSMA", csmaDevices.Get (0), true);
    }

  Simulator::Run ();
  Simulator::Destroy ();

  std::cout << "RTS failed : " << rtsFailed << std::endl;
  return 0;
}
