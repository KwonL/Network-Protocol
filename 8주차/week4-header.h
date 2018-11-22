#ifndef WEEK4_HEADER_H
#define WEEK4_HEADER_H

#include "ns3/header.h"

using namespace ns3;

class Week4Header : public Header
{
	public:
		static TypeId GetTypeId (void);
		virtual TypeId GetInstanceTypeId (void) const;
		virtual void Print (std::ostream &os) const;
		virtual uint32_t GetSerializedSize (void) const;
		virtual void Serialize (Buffer::Iterator start) const;
		virtual uint32_t Deserialize (Buffer::Iterator start);
		void SetTime(void);
		uint64_t GetTime(void) const;
		void SetSeq(uint16_t seq);
		uint16_t GetSeq(void) const;
	
	private:
		uint64_t m_time;
		uint16_t m_seq;
};	

#endif
