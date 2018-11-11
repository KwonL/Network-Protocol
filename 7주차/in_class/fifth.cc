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

#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FifthScriptExample");

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                5 Mbps, 2 ms
//
//
// We want to look at changes in the ns-3 TCP congestion window.  We need
// to crank up a flow and hook the CongestionWindow attribute on the socket
// of the sender.  Normally one would use an on-off application to generate a
// flow, but this has a couple of problems.  First, the socket of the on-off 
// application is not created until Application Start time, so we wouldn't be 
// able to hook the socket (now) at configuration time.  Second, even if we 
// could arrange a call after start time, the socket is not public so we 
// couldn't get at it.
//
// So, we can cook up a simple version of the on-off application that does what
// we want.  On the plus side we don't need all of the complexity of the on-off
// application.  On the minus side, we don't have a helper, so we have to get
// a little more involved in the details, but this is trivial.
//
// So first, we create a socket and do the trace connect on it; then we pass 
// this socket into the constructor of our simple application which we then 
// install in the source node.
// ===========================================================================
//
static uint32_t udp_count = 0;
static uint32_t tcp_count = 0;
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

static void
CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
}

static void
CalcThru_udp(Ptr<const Packet> pkt, const Address& addr) {
  if (Simulator::Now().GetSeconds() > 8 && Simulator::Now().GetSeconds() < 18) {
    udp_count++;
  }
  else if (Simulator::Now().GetSeconds() > 18) {
    NS_LOG_UNCOND( "Now throughput of udp is : " << 1040 * udp_count / 10 << "bytes");
  }
}

static void
CalcThru_tcp(Ptr<const Packet> pkt, const Address& addr) {
  if (Simulator::Now().GetSeconds() > 8 && Simulator::Now().GetSeconds() < 18) {
    tcp_count++;
  }
  else if (Simulator::Now().GetSeconds() > 18) {
    NS_LOG_UNCOND( "Now throughput of tcp is : " << 1040 * tcp_count / 10 << "bytes");
  }
}

// static void
// RxDrop (Ptr<const Packet> p)
// {
//   NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
// }

int 
main (int argc, char *argv[])
{
  std::string udp_rate = "2Mbps";
  uint32_t rcv_buf = 10000;

  CommandLine cmd;
  cmd.AddValue ("udp_rate", "udp rate", udp_rate);
	cmd.AddValue ("rcv_buf", "recv buffer size", rcv_buf);
  cmd.Parse (argc, argv);
  
  NodeContainer nodes;
  nodes.Create (4);

  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(rcv_buf));
  Config::SetDefault("ns3::UdpSocket::RcvBufSize", UintegerValue(rcv_buf));

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices_1_3, devices_2_3, devices_3_4;
  devices_1_3 = pointToPoint.Install (nodes.Get(0), nodes.Get(2));
  devices_2_3 = pointToPoint.Install (nodes.Get(1), nodes.Get(2));
  devices_3_4 = pointToPoint.Install (nodes.Get(2), nodes.Get(3));

  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  devices_3_4.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces_1_3 = address.Assign (devices_1_3);
  address.SetBase ("10.1.2.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces_2_3 = address.Assign (devices_2_3);
  address.SetBase ("10.1.3.0", "255.255.255.252");
  Ipv4InterfaceContainer interfaces_3_4 = address.Assign (devices_3_4);


  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (interfaces_3_4.GetAddress (1), sinkPort));
  uint16_t sinkPort_udp = 9;
  Address sinkAddress_udp (InetSocketAddress (interfaces_3_4.GetAddress (1), sinkPort_udp));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (3));
  PacketSinkHelper packetSinkHelper_udp ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort_udp));
  ApplicationContainer sinkApps_udp = packetSinkHelper_udp.Install (nodes.Get (3));
  sinkApps_udp.Start (Seconds (0.));
  sinkApps_udp.Stop (Seconds (30.));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (30.));
  Config::ConnectWithoutContext("NodeList/3/ApplicationList/0/$ns3::PacketSink/Rx",MakeCallback(&CalcThru_tcp));
  Config::ConnectWithoutContext("NodeList/3/ApplicationList/1/$ns3::PacketSink/Rx",MakeCallback(&CalcThru_udp));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));
  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (nodes.Get (0), UdpSocketFactory::GetTypeId ());

  // TCP app
  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 1040, 100000, DataRate ("2Mbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (5.));
  app->SetStopTime (Seconds (20.));

  // UDP app
  Ptr<MyApp> app_udp = CreateObject<MyApp> ();
  app_udp->Setup (ns3UdpSocket, sinkAddress_udp, 1040, 100000, DataRate (udp_rate));
  nodes.Get (1)->AddApplication (app_udp);
  app_udp->SetStartTime (Seconds (1.));
  app_udp->SetStopTime (Seconds (30.));

  // devices.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (20));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

