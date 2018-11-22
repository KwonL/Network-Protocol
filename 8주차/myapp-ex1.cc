#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

static void PacketTx (Ptr<const Packet> p)
{
	NS_LOG_UNCOND ("\n" << Simulator::Now().GetSeconds() << "\t" << "A new packet is sent at Node 0");
}

static void PacketRx (Ptr<const Packet> p)
{
	NS_LOG_UNCOND (Simulator::Now().GetSeconds() << "\t" << "A packet is received at Node 1");
}

static uint16_t latest_seq = -1;
// Trace Packet sequence number to check packet loss
static void PacketErr (Ptr<const Packet> p, Week4Header &h) 
{
	NS_LOG_UNCOND (Simulator::Now().GetSeconds() << "\t" << "A packet is received with sequence num " << std::to_string(h.GetSeq()));
	// Expected sequence number is latest seq num + 1. else, packet loss occured
	if ((++latest_seq) != h.GetSeq()) {
		NS_LOG_UNCOND("Sequence number error! expected seq num is : " << std::to_string(latest_seq));
	}
	// Correctly received packet!
	else {
		NS_LOG_UNCOND("Sequence number is correct");
	}
	// Update latest seq num
	latest_seq = h.GetSeq();
}

int main (int argc, char *argv[])
{
  LogComponentEnable ("MyApp", LOG_LEVEL_INFO);
	NodeContainer nodes;
	nodes.Create (2);

	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

	NetDeviceContainer devices;
	devices = pointToPoint.Install (nodes);

	// Set error model for checking sequence number
	Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
	em->SetAttribute ("ErrorRate", DoubleValue (0.0002));
	devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

	InternetStackHelper stack;
	stack.Install (nodes);

	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.1.1.0", "255.255.255.252");
	Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

	uint16_t port = 8080;
	Address destination (InetSocketAddress (interfaces.GetAddress (1), port));

	MyAppHelper sender (true, destination);
	sender.SetAttribute("NPackets", UintegerValue(200));
	sender.SetAttribute("DataRate", DataRateValue(DataRate("2Mb/s"))); 
	ApplicationContainer senderApp = sender.Install(nodes.Get(0));
	senderApp.Start (Seconds(1.0));
	senderApp.Stop (Seconds(5.0));
	
	Address any (InetSocketAddress (Ipv4Address::GetAny(), port));
	
	MyAppHelper receiver (false, any);
	ApplicationContainer receiverApp = receiver.Install(nodes.Get(1));
	receiverApp.Start (Seconds(0.5));
	receiverApp.Stop (Seconds(7.0));

	senderApp.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback (&PacketTx));
	receiverApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback (&PacketRx));
	receiverApp.Get(0)->TraceConnectWithoutContext("Err", MakeCallback (&PacketErr));
  
  pointToPoint.EnablePcapAll ("myapp-test");

	Simulator::Stop (Seconds (20));
	Simulator::Run ();
	Simulator::Destroy ();

	return 0;
}
