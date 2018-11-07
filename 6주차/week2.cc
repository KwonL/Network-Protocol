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
	int nWifi = 4;
	uint32_t pktSize = 400;
	uint32_t thre = 2000;

	CommandLine cmd;
	cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
	cmd.AddValue ("pktSize", "Size of UDP packet", pktSize);
	cmd.AddValue ("thre", "RTC/CTS threshold", thre);
	cmd.Parse (argc,argv);

	NodeContainer wifiApNode;
	wifiApNode.Create (1);
	
	NodeContainer wifiStaNodes;
	wifiStaNodes.Create (nWifi);

	NodeContainer nodes (wifiApNode, wifiStaNodes);

	//YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
	YansWifiChannelHelper channel;
	channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel"); 
	channel.AddPropagationLoss("ns3::JakesPropagationLossModel");
	
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
	phy.SetChannel (channel.Create ());

	NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

	Ssid ssid = Ssid ("ns-3-ssid");
	mac.SetType ("ns3::ApWifiMac",
			"Ssid", SsidValue (ssid));

	WifiHelper wifi = WifiHelper::Default ();
	//wifi.SetRemoteStationManager ("ns3::ArfWifiManager", 	"RtsCtsThreshold", UintegerValue (thre) );
	wifi.SetRemoteStationManager ("ns3::ArfWifiManager" );
	
	NetDeviceContainer apDevices = wifi.Install (phy, mac, wifiApNode);
	
	mac.SetType ("ns3::StaWifiMac",
			"Ssid", SsidValue (ssid),
			"ActiveProbing", BooleanValue (false));

	NetDeviceContainer staDevices = wifi.Install (phy, mac, wifiStaNodes);

	MobilityHelper mobility_d;
	mobility_d.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	MobilityHelper mobility;
	mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Bounds", RectangleValue (Rectangle (-30, 30, -30, 30)));

	Ptr<UniformDiscPositionAllocator> positionAlloc = CreateObject<UniformDiscPositionAllocator> ();
	Ptr<ListPositionAllocator> positionAlloc_d = CreateObject<ListPositionAllocator> ();
	positionAlloc->SetRho(30.0);
	positionAlloc->AssignStreams(nWifi);
	positionAlloc_d->Add (Vector (0.0, 0.0, 0.0));
//	for (int i = 0; i < nWifi; i++){
//		switch(i%nWifi){
			//case 0:	positionAlloc->Add(Vector((i/4+1)*20.0, 0.0, 0.0)); break;
			//case 1:	positionAlloc->Add(Vector(0.0, (i/4+1)*20.0, 0.0)); break;
			//case 2:	positionAlloc->Add(Vector((i/4+1)*-20.0, 0.0, 0.0)); break;
			//case 3:	positionAlloc->Add(Vector(0.0, (i/4+1)*-20.0, 0.0)); break;
//			positionAlloc->Add();
//		}
//	}
	mobility_d.SetPositionAllocator (positionAlloc_d);
	mobility_d.Install (wifiApNode);
	mobility.SetPositionAllocator (positionAlloc);
	mobility.Install (wifiStaNodes);

	InternetStackHelper stack;
	stack.Install (nodes);

	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer wifiApInterface = address.Assign (apDevices);
	Ipv4InterfaceContainer wifiStaInterfaces = address.Assign (staDevices);

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	uint16_t port = 9;

	OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (wifiApInterface.GetAddress(0), port)));
	onoff.SetAttribute ("OnTime",StringValue ("ns3::ConstantRandomVariable[Constant=1]") );
	onoff.SetAttribute ("OffTime",StringValue ("ns3::ConstantRandomVariable[Constant=0]") );
	onoff.SetAttribute ("PacketSize", UintegerValue(pktSize));
	onoff.SetAttribute ("DataRate", StringValue ("30Mbps"));

	for(int i = 0; i < nWifi; i++){
		ApplicationContainer app = onoff.Install(wifiStaNodes.Get(i));
		app.Start(Seconds(2));
		app.Stop(Seconds(5));
	}

	PacketSinkHelper udpsink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
	ApplicationContainer sinkapp = udpsink.Install (wifiApNode.Get (0));
	sinkapp.Start (Seconds (1.0));
	sinkapp.Stop (Seconds (10.0));

	Simulator::Stop (Seconds (10.0));

	phy.EnablePcap ("week2", apDevices.Get(0));

	Simulator::Run ();
	Simulator::Destroy ();
}
