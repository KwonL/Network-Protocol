#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/csma-module.h"
#include  <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  bool csma = true;
  std::string node4DataRate = "1Mbps";
  CommandLine cmd;
  cmd.AddValue("csma", "type of channel between node 2 and node 3", csma);
  cmd.AddValue("node4DataRate", "data rate of node 4", node4DataRate);
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (4);

  // For node 1 and node 2
  PointToPointHelper first_2_second;
  first_2_second.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  first_2_second.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer devices_1;
  devices_1 = first_2_second.Install (nodes.Get(0), nodes.Get(1));

  // For node 2 and node 3
  NetDeviceContainer devices_2;
  if (csma == true) {
    CsmaHelper second_2_third;
    second_2_third.SetChannelAttribute("DataRate", StringValue("2Mbps"));
    second_2_third.SetChannelAttribute("Delay", StringValue("1ms")) ;

    NodeContainer csmaContainer(nodes.Get(1), nodes.Get(2));
    devices_2 = second_2_third.Install(csmaContainer);
    second_2_third.EnablePcapAll("csma_channel");
  }
  else {
    PointToPointHelper second_2_third;
    second_2_third.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
    second_2_third.SetChannelAttribute ("Delay", StringValue ("1ms"));

    devices_2 = second_2_third.Install (nodes.Get(1), nodes.Get(2));
    second_2_third.EnablePcapAll("p2p_channel");
  }

  // For node 2 and node 3
  PointToPointHelper third_2_fourth;
  third_2_fourth.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  third_2_fourth.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer devices_3;
  devices_3 = third_2_fourth.Install (nodes.Get(2), nodes.Get(3));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_1 = address.Assign (devices_1);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_2 = address.Assign (devices_2);
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_3 = address.Assign (devices_3);

  // Sink server at node 3
  PacketSinkHelper sinkServer_1("ns3::UdpSocketFactory", Address(InetSocketAddress(interfaces_2.GetAddress(1), 9)));
  ApplicationContainer serverApps_1 = sinkServer_1.Install (nodes.Get (2));
  serverApps_1.Start (Seconds (1.0));

  // Sink server at node 2
  PacketSinkHelper sinkServer_2("ns3::UdpSocketFactory", Address(InetSocketAddress(interfaces_2.GetAddress(0), 9)));
  ApplicationContainer serverApps_2 = sinkServer_2.Install (nodes.Get (1));
  serverApps_2.Start (Seconds (1.0));

  // OnOff application helper, At node 1
  OnOffHelper onOff_1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces_2.GetAddress (1), 9)));
  onOff_1.SetAttribute ("PacketSize", UintegerValue (1472));
  onOff_1.SetAttribute ("DataRate", StringValue ("1Mbps"));
  onOff_1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onOff_1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  // Install App on Node 1
  ApplicationContainer clientApps_1 = onOff_1.Install(nodes.Get(0));
  clientApps_1.Start(Seconds(2.0));
  clientApps_1.Stop(Seconds(5.0));

  // OnOff application helper, At Node 4
  OnOffHelper onOff_2 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces_2.GetAddress (0), 9)));
  onOff_2.SetAttribute ("PacketSize", UintegerValue (1472));
  onOff_2.SetAttribute ("DataRate", StringValue (node4DataRate));
  onOff_2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onOff_2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  // Install App on 
  ApplicationContainer clientApps_2 = onOff_2.Install(nodes.Get(3));
  clientApps_2.Start(Seconds(4.0));
  clientApps_2.Stop(Seconds(7.0));

  // enable routing table
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // first_2_second.EnablePcapAll("node_1_2");

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
