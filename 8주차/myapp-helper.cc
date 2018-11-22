#include "myapp-helper.h"
#include "ns3/names.h"

namespace ns3 {
	MyAppHelper::MyAppHelper (bool mode, Address address) {
		m_factory.SetTypeId ("ns3::MyApp");
		m_factory.Set ("Mode", BooleanValue (mode));
		m_factory.Set ("Address", AddressValue (address));
		//m_factory.Set ("NPackets", UintegerValue (nPackets));
		//m_factory.Set ("DataRate", DataRateValue (dataRate));
	}

	void MyAppHelper::SetAttribute (std::string name, const AttributeValue &value)	{
		m_factory.Set (name, value);
	}

	ApplicationContainer MyAppHelper::Install (Ptr<Node> node) const {
		return ApplicationContainer (InstallPriv (node));
	}

	ApplicationContainer MyAppHelper::Install (std::string nodeName) const {
		Ptr<Node> node = Names::Find<Node> (nodeName);
		return ApplicationContainer (InstallPriv (node));
	}

	ApplicationContainer MyAppHelper::Install (NodeContainer c) const {
		ApplicationContainer apps;
		for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
			apps.Add (InstallPriv (*i));
		}
		return apps;
	}

	Ptr<Application> MyAppHelper::InstallPriv (Ptr<Node> node) const {
		Ptr<Application> app = m_factory.Create<Application> ();
		node->AddApplication (app);
		return app;
	}
}
