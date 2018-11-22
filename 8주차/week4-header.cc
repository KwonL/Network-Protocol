#include "week4-header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE("Week4Header");

using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED(Week4Header);

TypeId Week4Header::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::Week4Header")
		.SetParent<Header> ()
		.AddConstructor<Week4Header> ()
		;
	return tid;
}

TypeId Week4Header::GetInstanceTypeId (void) const
{
	return GetTypeId();
}

void Week4Header::Serialize (Buffer::Iterator start) const
{	
	// Write 16 bits for sequence number
	start.WriteHtonU16(m_seq);
	// Write 64 bits for time
	start.WriteHtonU64(m_time);
}

uint32_t Week4Header::GetSerializedSize (void) const
{	
	// seq_num + time = 10bytes
	return 10;
}

uint32_t Week4Header::Deserialize (Buffer::Iterator start)
{
	// Read header and set value
	Buffer::Iterator i = start;

	// sequence field preceed time field
	m_seq = i.ReadNtohU16();
	m_time = i.ReadNtohU64();
	return i.GetDistanceFrom(start);
}

void Week4Header::Print (std::ostream &os) const
{
    //TODO
    //Printing seq value set in the header
	os << "m_seq = " << m_seq << std::endl;
}

void Week4Header::SetTime(void)
{
    //TODO
    //Setting time value(microseconds) in the header
	m_time = Simulator::Now ().GetMicroSeconds();
}

uint64_t Week4Header::GetTime(void) const
{
    //TODO
    //Getting time value

	return m_time;
}

void Week4Header::SetSeq(uint16_t seq) {
	// Set m_seq as argument 
	m_seq = seq;
}

uint16_t Week4Header::GetSeq(void) const {
	// return m_seq;
	return m_seq;
}