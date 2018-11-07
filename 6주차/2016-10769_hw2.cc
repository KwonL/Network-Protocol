#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Week2Example");

int main (int argc, char *argv[])
{
	// variables for input
	int nWifi = 2;
	uint32_t pktSize = 400;
	uint32_t thre = 0;
	uint32_t algo = 0;

	CommandLine cmd;
	cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
	cmd.AddValue ("pktSize", "Size of UDP packet", pktSize);
	cmd.AddValue ("thre", "RTC/CTS threshold", thre);
	cmd.AddValue("algo", "Rate algorithm", algo);
	cmd.Parse (argc,argv);

	// LogComponentEnable("PacketSink", LOG_LEVEL_FUNCTION);

	NodeContainer wifiApNode;
	wifiApNode.Create (1);
	
	NodeContainer wifiStaNodes;
	wifiStaNodes.Create (nWifi);

	NodeContainer nodes (wifiApNode, wifiStaNodes);

	// Set channel as Constant Speed Propagation Delay Model, Jakes Propagation Loss Model
	YansWifiChannelHelper channel;
	channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel"); 
	channel.AddPropagationLoss("ns3::JakesPropagationLossModel");
	
	// Physical Layer helper
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
	phy.SetChannel (channel.Create ());

	// WIFI MAC helper
	NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

	Ssid ssid = Ssid ("ns-3-ssid");
	mac.SetType ("ns3::ApWifiMac",
			"Ssid", SsidValue (ssid));

	WifiHelper wifi = WifiHelper::Default ();
	// Selected wifi manager
	// Set RTS/CTS threshold
	switch (algo) {
		case 0 :
			wifi.SetRemoteStationManager("ns3::ArfWifiManager", "RtsCtsThreshold", UintegerValue(thre));
			break;
		case 1 :
			wifi.SetRemoteStationManager("ns3::CaraWifiManager", "RtsCtsThreshold", UintegerValue(thre));
			break;
		case 2 :
			wifi.SetRemoteStationManager("ns3::IdealWifiManager", "RtsCtsThreshold", UintegerValue(thre));
			break;	
	}
	
	NetDeviceContainer apDevices = wifi.Install (phy, mac, wifiApNode);
	
	mac.SetType ("ns3::StaWifiMac",
			"Ssid", SsidValue (ssid),
			"ActiveProbing", BooleanValue (false));

	NetDeviceContainer staDevices = wifi.Install (phy, mac, wifiStaNodes);

	MobilityHelper mobility_d;
	mobility_d.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	MobilityHelper mobility;
	// Set mobility for Bounded, Pause, for STA
	mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel","Bounds", RectangleValue (Rectangle (-30, 30, -30, 30)), "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.5]"));

	Ptr<UniformDiscPositionAllocator> positionAlloc = CreateObject<UniformDiscPositionAllocator> ();
	Ptr<ListPositionAllocator> positionAlloc_d = CreateObject<ListPositionAllocator> ();
	// AP node's loacation is O
	positionAlloc_d->Add (Vector (0.0, 0.0, 0.0));
	mobility_d.SetPositionAllocator (positionAlloc_d);
	mobility_d.Install (wifiApNode);

	// This is for mobility stations
	positionAlloc->SetRho(30.0);
	positionAlloc->AssignStreams(nWifi);
	mobility.SetPositionAllocator (positionAlloc);
	mobility.Install (wifiStaNodes);

	InternetStackHelper stack;
	stack.Install (nodes);

	// Set ip address for nodes
	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer wifiApInterface = address.Assign (apDevices);
	Ipv4InterfaceContainer wifiStaInterfaces = address.Assign (staDevices);

	// enable routing table
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	uint16_t port = 9;

	// OnOff application configure
	OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (wifiApInterface.GetAddress(0), port)));
	onoff.SetAttribute ("OnTime",StringValue ("ns3::ConstantRandomVariable[Constant=1]") );
	onoff.SetAttribute ("OffTime",StringValue ("ns3::ConstantRandomVariable[Constant=0]") );
	onoff.SetAttribute ("PacketSize", UintegerValue(pktSize));
	onoff.SetAttribute ("DataRate", StringValue ("30Mbps"));

	// Install Onoff App to station nodess
	for(int i = 0; i < nWifi; i++){
		ApplicationContainer app = onoff.Install(wifiStaNodes.Get(i));
		app.Start(Seconds(2));
		app.Stop(Seconds(5));
	}

	PacketSinkHelper udpsink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
	ApplicationContainer sinkapp = udpsink.Install (wifiApNode.Get (0));
	sinkapp.Start (Seconds (1.0));
	sinkapp.Stop (Seconds (10.0));

	Simulator::Stop (Seconds (12.0));

	phy.EnablePcap ("hw2_n_" + std::to_string(nWifi) + "_algo_" + std::to_string(algo) + "_thre_" + std::to_string(thre), apDevices.Get(0));

	Simulator::Run ();
	Simulator::Destroy ();
}
