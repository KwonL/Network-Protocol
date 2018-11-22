/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 */
#include "ns3/log.h"
#include "myapp.h"
#include "ns3/week4-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MyApp");
NS_OBJECT_ENSURE_REGISTERED (MyApp);

TypeId
MyApp::GetTypeId (void) {
  static TypeId tid = TypeId ("ns3::MyApp")
    .SetParent<Application> ()
    .AddConstructor<MyApp> () 
    .AddAttribute ("Mode", "The mode : Sender(true), Receiver(false)",
                   BooleanValue (false),
                   MakeBooleanAccessor (&MyApp::m_mode),
                   MakeBooleanChecker ())
    .AddAttribute ("Address", "The address",
                   AddressValue (),
                   MakeAddressAccessor (&MyApp::m_address),
                   MakeAddressChecker ())
    .AddAttribute ("NPackets", "The total number of packets to send",
                   UintegerValue (10000),
                   MakeUintegerAccessor (&MyApp::m_nPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DataRate", "The data rate",
                   DataRateValue (DataRate ("500kb/s")),
                   MakeDataRateAccessor (&MyApp::m_dataRate),
                   MakeDataRateChecker ())
		.AddTraceSource("Tx", "A new packet is created and is sent", MakeTraceSourceAccessor(&MyApp::m_txTrace),"ns3::Packet::TracedCallback")
		.AddTraceSource("Rx", "A packet has been received", MakeTraceSourceAccessor (&MyApp::m_rxTrace),"ns3::Packet::TracedCallback")
		// Add trace source. This value will trace sequence number in header field. So, we can check if there are packet loss event.
		.AddTraceSource("Err", "A packet has error header", MakeTraceSourceAccessor(&MyApp::m_errTrace), "ns3::Packet::TracedCallback")
 ;
  return tid;
}

// Initialize private values
MyApp::MyApp () 
	: m_socket (0),
		m_packetSize (1000),
		m_packetsSent (0),
		m_running (false)
{
  NS_LOG_FUNCTION (this);
}

MyApp::~MyApp ()
{
  NS_LOG_FUNCTION (this);
}

void
MyApp::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

	if(m_mode == true)
	{	
		// Sender initialize sequence number as -1, because we will first ++ seq_num and send.
		seq_num = -1;
		
		if(!m_socket){
			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
			m_socket = Socket::CreateSocket(GetNode(), tid);
			m_socket->Bind();
			m_socket->Connect(m_address);
		}
		m_running = true;
		SendPacket();
	}
	else
	{
		// receiver initialize sequence number as -1 (0xff)
		seq_num = -1;

		if(!m_socket){
			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
			m_socket = Socket::CreateSocket(GetNode(), tid);
			m_socket->Bind(m_address);
			m_socket->Listen();
			m_socket->ShutdownSend();
			m_socket->SetRecvCallback (MakeCallback (&MyApp::HandleRead, this));
		}
	}
}

void
MyApp::StopApplication ()
{
  NS_LOG_FUNCTION (this);
	// Set state as not running
	m_running = false;
	if(m_sendEvent.IsRunning())
	{
	// Cancel send
  	Simulator::Cancel (m_sendEvent);
	}
	if(m_socket)
	{
	// close scoket
		m_socket->Close();
	}
}

void
MyApp::SendPacket (void)
{
  NS_LOG_FUNCTION (this);
	Ptr<Packet> packet = Create<Packet> (m_packetSize);
	m_txTrace(packet);

	Week4Header header;
	// Set sequence number with ++
	header.SetSeq(++seq_num);
	// In class work
	header.SetTime();
	header.Print(std::cout);
	// Set header 
	packet->AddHeader(header);

	m_socket->Send(packet);

	if(++m_packetsSent < m_nPackets)
	{
		ScheduleTx();
	}
}

void
MyApp::ScheduleTx (void)
{
  if(m_running)
	{
		Time tNext (Seconds (m_packetSize*8/static_cast<double>(m_dataRate.GetBitRate())));
		m_sendEvent = Simulator::Schedule(tNext, &MyApp::SendPacket, this);
	}
}

void
MyApp::HandleRead (Ptr<Socket> socket)
{
	Ptr<Packet> packet;
	Address from;
	
	while ((packet = m_socket->RecvFrom(from)))
	{
		if(packet->GetSize() > 0)
		{
			Week4Header header;
			packet->RemoveHeader(header);

			// In receiver node, Get sequence number and set m_seq to check if there are packet loss 
			seq_num = header.GetSeq();
			// std::cout << "Receive header = " << seq_num << std::endl;

			m_rxTrace(packet);
			// packet error check trace source.
			m_errTrace(packet, header);
		}
	}
}

} // Namespace ns3
