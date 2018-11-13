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

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

uint32_t tcppkt = 0;

uint32_t udppkt = 0;
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
CalcThru (Ptr<const Packet> p, const Address &address){
    //NS_LOG_UNCOND (pkt*1040*8/10000000);
    if(Simulator::Now ().GetSeconds ()>8 && Simulator::Now ().GetSeconds ()<18){
        //NS_LOG_UNCOND ("test");
        tcppkt++;
    }
}

static void
CalcThru2 (Ptr<const Packet> p, const Address &address){
    //NS_LOG_UNCOND (pkt*1040*8/10000000);
    if(Simulator::Now ().GetSeconds ()>8 && Simulator::Now ().GetSeconds ()<18){
        //NS_LOG_UNCOND ("test");
        udppkt++;
    }
}

    static void
CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
}

    static void
RxDrop (Ptr<const Packet> p)
{
    NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
}

    int 
main (int argc, char *argv[])
{

    //uint32_t rcvBufSize = 10000;
    std::string udpDataRate = "0.5Mbps";

    NodeContainer nodes;
    nodes.Create (4);
    //CommandLine cmd;
    //cmd.AddValue ("rcvBufSize", "Receiver Buffer Size", rcvBufSize);
    //cmd.AddValue ("updDataRate", "UDP Data Rate", udpDataRate);

    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(10000));
    Config::SetDefault("ns3::UdpSocket::RcvBufSize", UintegerValue(10000));

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NodeContainer nodes13 = NodeContainer (nodes.Get(0), nodes.Get(2));
    NodeContainer nodes23 = NodeContainer (nodes.Get(1), nodes.Get(2));
    NodeContainer nodes34 = NodeContainer (nodes.Get(2), nodes.Get(3));

    NetDeviceContainer devices13;
    NetDeviceContainer devices23;
    NetDeviceContainer devices34;
    devices13 = pointToPoint.Install (nodes13);
    devices23 = pointToPoint.Install (nodes23);
    devices34 = pointToPoint.Install (nodes34);

    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
    //devices13.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
    //devices23.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
    devices34.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address13;
    address13.SetBase ("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces13 = address13.Assign (devices13);

    Ipv4AddressHelper address23;
    address23.SetBase ("10.1.2.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces23 = address23.Assign (devices23);

    Ipv4AddressHelper address34;
    address34.SetBase ("10.1.3.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces34 = address34.Assign (devices34);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    uint16_t sinkPort = 8080;
    Address sinkAddress (InetSocketAddress (interfaces34.GetAddress (1), sinkPort));
    PacketSinkHelper packetSinkHelper1 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps1 = packetSinkHelper1.Install (nodes34.Get (1));
    sinkApps1.Get(0)->TraceConnectWithoutContext ("Rx", MakeCallback (&CalcThru));
    sinkApps1.Start (Seconds (5));
    sinkApps1.Stop (Seconds (20));

    Address sinkAddress2 (InetSocketAddress (interfaces34.GetAddress (1), 9));
    PacketSinkHelper packetSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), 9)); 
    ApplicationContainer sinkApps2 = packetSinkHelper2.Install (nodes34.Get(1));
    sinkApps2.Get(0)->TraceConnectWithoutContext ("Rx", MakeCallback (&CalcThru2));
    sinkApps2.Start (Seconds (1));
    sinkApps2.Stop (Seconds (30));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
    ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));

    Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (nodes.Get (1), UdpSocketFactory::GetTypeId ());
    //ns3UdpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));

    Ptr<MyApp> app1 = CreateObject<MyApp> ();
    app1->Setup (ns3TcpSocket, sinkAddress, 1040, 1000000, DataRate ("2Mbps"));
    nodes.Get (0)->AddApplication (app1);
    app1->SetStartTime (Seconds (5));
    app1->SetStopTime (Seconds (20));

    Ptr<MyApp> app2 = CreateObject<MyApp> ();
    app2->Setup (ns3UdpSocket, sinkAddress2, 1040, 100000, DataRate (udpDataRate));
    nodes.Get (1)->AddApplication (app2);
    app2->SetStartTime (Seconds (1));
    app2->SetStopTime (Seconds (30));

    devices34.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));

    //std::cout<<udpDataRate<<"\t"<<rcvBufSize<<std::endl;

    Simulator::Stop (Seconds (30));
    Simulator::Run ();
    Simulator::Destroy ();

    NS_LOG_UNCOND("tcp_throughput = "<<(double)tcppkt*1040*8.0/10000000);
    NS_LOG_UNCOND("udp_throughput = "<<(double)udppkt*1040*8.0/10000000);

    return 0;
}

