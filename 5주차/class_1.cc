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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{

  uint32_t MaxPackets = 1;
  float Interval = 1.0;
  uint32_t PacketSize = 1024;
  CommandLine cmd;
  cmd.AddValue("MaxPackets", "Max Packets vaule", MaxPackets);
  cmd.AddValue("Interval", "interval vaule", Interval);
  cmd.AddValue("PacketSize", "Max Packets vaule", PacketSize);
  cmd.Parse (argc, argv);
  
  // In class logging
  LogComponentEnable ("UdpEchoServerApplication", LOG_FUNCTION);

  Time::SetResolution (Time::NS);
  // LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  // LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (3);

  PointToPointHelper pointToPoint_1;
  pointToPoint_1.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint_1.SetChannelAttribute ("Delay", StringValue ("1ms"));
  PointToPointHelper pointToPoint_2;
  pointToPoint_2.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint_2.SetChannelAttribute ("Delay", StringValue ("100ms"));

  NetDeviceContainer devices_1, devices_2;
  devices_1 = pointToPoint_1.Install (nodes.Get(0), nodes.Get(1));
  devices_2 = pointToPoint_2.Install (nodes.Get(1), nodes.Get(2));

  pointToPoint_1.EnablePcapAll("first");
  pointToPoint_2.EnablePcapAll("second");

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces_1 = address.Assign (devices_1);
  
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_2 = address.Assign (devices_2);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces_1.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (MaxPackets));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (Interval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (PacketSize));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient_2 (interfaces_2.GetAddress (0), 9);
  echoClient_2.SetAttribute ("MaxPackets", UintegerValue (MaxPackets));
  echoClient_2.SetAttribute ("Interval", TimeValue (Seconds (Interval)));
  echoClient_2.SetAttribute ("PacketSize", UintegerValue (PacketSize));

  ApplicationContainer clientApps_2 = echoClient_2.Install (nodes.Get (2));
  clientApps_2.Start (Seconds (4.0));
  clientApps_2.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
