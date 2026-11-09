/*
 * EECE5155 Lab 2 - Minimal two-node 802.11 ad hoc model (ns-3.48)
 *
 * Purpose: produce an ns-3 counterpart to the Colosseum Wi-Fi run so the two
 * can be compared. Two nodes, 802.11g ad hoc, one UDP flow offered at 5 Mbps
 * (matching the iperf3 client in the Colosseum part). FlowMonitor reports the
 * delivered throughput and the mean end-to-end delay.
 *
 * Build/run (from the ns-3.48 root, with this file in scratch/):
 *   ./ns3 run scratch/lab2-wifi-adhoc
 *
 * Optional argument: --distance=<m> (default 10 m).
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  double distance = 10.0;   // separation between the two nodes [m]
  double simTime  = 10.0;   // seconds
  std::string offeredRate = "5Mbps";   // matches "iperf3 -c ... -u -b 5M"

  CommandLine cmd;
  cmd.AddValue ("distance", "Distance between the two nodes [m]", distance);
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (2);

  // --- PHY / channel: ns-3 default (idealized) log-distance channel ---
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());

  // --- 802.11g ad hoc MAC ---
  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211g);

  WifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer devices = wifi.Install (phy, mac, nodes);

  // --- fixed positions ---
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator> ();
  pos->Add (Vector (0.0, 0.0, 0.0));
  pos->Add (Vector (distance, 0.0, 0.0));
  mobility.SetPositionAllocator (pos);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  // --- IP stack ---
  InternetStackHelper stack;
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer ifaces = address.Assign (devices);

  // --- UDP flow: node 0 -> node 1, offered at 5 Mbps ---
  uint16_t port = 9;
  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     InetSocketAddress (ifaces.GetAddress (1), port));
  onoff.SetConstantRate (DataRate (offeredRate), 1400);   // 1400-byte payload
  ApplicationContainer clientApp = onoff.Install (nodes.Get (0));
  clientApp.Start (Seconds (1.0));
  clientApp.Stop (Seconds (simTime));

  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer serverApp = sink.Install (nodes.Get (1));
  serverApp.Start (Seconds (0.0));
  serverApp.Stop (Seconds (simTime + 1.0));

  // --- FlowMonitor ---
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> monitor = fmHelper.InstallAll ();

  Simulator::Stop (Seconds (simTime + 1.0));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier> (fmHelper.GetClassifier ());
  auto stats = monitor->GetFlowStats ();

  for (auto &kv : stats)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (kv.first);
      double rxMbps = kv.second.rxBytes * 8.0 /
                      (simTime - 1.0) / 1e6;
      double meanDelayMs = (kv.second.rxPackets > 0)
          ? kv.second.delaySum.GetSeconds () / kv.second.rxPackets * 1e3
          : 0.0;
      std::cout << "Flow " << t.sourceAddress << " -> "
                << t.destinationAddress << "\n";
      std::cout << "  Tx packets : " << kv.second.txPackets << "\n";
      std::cout << "  Rx packets : " << kv.second.rxPackets << "\n";
      std::cout << "  Throughput : " << rxMbps << " Mbps\n";
      std::cout << "  Mean delay : " << meanDelayMs << " ms\n";
    }

  Simulator::Destroy ();
  return 0;
}
