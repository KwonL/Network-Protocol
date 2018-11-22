#ifndef MY_APP_H
#define MY_APP_H

#include "ns3/application.h"
#include "ns3/network-module.h"
#include "ns3/week4-header.h"

namespace ns3 {

class Socket;
class Packet;

class MyApp : public Application
{
	public:
		static TypeId GetTypeId (void);
		MyApp ();
		virtual ~MyApp ();

	private:
		virtual void StartApplication (void);
		virtual void StopApplication (void);

		void ScheduleTx (void);
		void SendPacket (void);
		void HandleRead (Ptr<Socket> socket);

		bool m_mode; // Tx: true, Rx: false
		Address m_address;
		uint32_t m_nPackets;
		DataRate m_dataRate;
		
		Ptr<Socket> m_socket;
		uint32_t m_packetSize;
		uint32_t m_packetsSent;
		EventId m_sendEvent;
		bool m_running;

		uint16_t seq_num;

		TracedCallback<Ptr<const Packet> > m_txTrace;
		TracedCallback<Ptr<const Packet> > m_rxTrace;
		TracedCallback<Ptr<const Packet>, Week4Header & > m_errTrace;
};

} // namespace ns3

#endif 
