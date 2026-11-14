/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Wired ICN topology for the ndnSIM lab.
//
// Hexagonal content-router mesh (7 CRs) plus clients and two content sources.
// Each NDN node runs NFD, so every node has CS, PIT, and FIB.
//
//        cr_ul -------- cr_ur ---- producer1     /C1 /C3 /C4
//          |              |
//     cr_lc +-- cr_ctr ---+ cr_rc   (centre not linked to lc or rc)
//     client1,2          |
//        cr_ll -------- cr_lr ---- server2       /C2 /C5 /C6
//   Hex ring: ul-ur-rc-lr-ll-lc-ul
//
// Perimeter (hexagon): cr_ul, cr_ur, cr_rc, cr_lr, cr_ll, cr_lc — each to neighbours.
// cr_ctr links only to cr_ul, cr_ur, cr_ll, cr_lr (not cr_lc or cr_rc).
//
// Run from the project root:
//   .\scripts\run.ps1
//   .\scripts\run.ps1 --frequency=10 --csSize=50 --simTime=20

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

#include <string>

namespace ns3 {

static Ptr<Node>
MakeNode(const std::string& name)
{
  Ptr<Node> node = CreateObject<Node>();
  Names::Add(name, node);
  NS_LOG_UNCOND("Node " << node->GetId() << " = " << name);
  return node;
}

static void
InstallProducer(Ptr<Node> node, const std::string& prefix, const std::string& payload,
                ndn::GlobalRoutingHelper& routing)
{
  ndn::AppHelper producer("ns3::ndn::Producer");
  producer.SetPrefix(prefix);
  producer.SetAttribute("PayloadSize", StringValue(payload));
  producer.Install(node);
  routing.AddOrigins(prefix, node);
  NS_LOG_UNCOND("Producer " << Names::FindName(node) << " serves " << prefix);
}

static void
InstallConsumer(Ptr<Node> node, const std::string& prefix, const std::string& frequency,
                double start, double stop)
{
  ndn::AppHelper consumer("ns3::ndn::ConsumerCbr");
  consumer.SetPrefix(prefix);
  consumer.SetAttribute("Frequency", StringValue(frequency));
  ApplicationContainer apps = consumer.Install(node);
  apps.Start(Seconds(start));
  apps.Stop(Seconds(stop));
  NS_LOG_UNCOND("Consumer " << Names::FindName(node) << " requests " << prefix
                            << " at " << frequency << " interests/s"
                            << " from t=" << start << " to t=" << stop);
}

static void
Link(PointToPointHelper& p2p, Ptr<Node> a, Ptr<Node> b)
{
  p2p.Install(a, b);
}

int
main(int argc, char* argv[])
{
  std::string results = "/results";
  std::string strategy = "/localhost/nfd/strategy/best-route";
  std::string csPolicy = "nfd::cs::lru";
  std::string payload = "1024";
  std::string frequency = "2";
  uint32_t csSize = 100;
  double simTime = 15.0;

  CommandLine cmd;
  cmd.AddValue("results", "Directory for tracer files", results);
  cmd.AddValue("strategy", "NFD forwarding strategy installed for /", strategy);
  cmd.AddValue("csPolicy", "Content Store policy (nfd::cs::lru or nfd::cs::priority_fifo)", csPolicy);
  cmd.AddValue("payload", "Producer payload size in bytes", payload);
  cmd.AddValue("frequency", "Interests per second, per consumer prefix", frequency);
  cmd.AddValue("csSize", "Content Store size in packets, on every node", csSize);
  cmd.AddValue("simTime", "Simulation length in seconds", simTime);
  cmd.Parse(argc, argv);

  const std::string accessRate = "100Mbps";
  const std::string accessDelay = "1ms";
  const std::string coreRate = "10Mbps";
  const std::string coreDelay = "10ms";

  Config::SetDefault("ns3::DropTailQueue<Packet>::MaxSize", StringValue("20p"));

  Ptr<Node> client1 = MakeNode("client1");
  Ptr<Node> client2 = MakeNode("client2");
  Ptr<Node> cr_ul = MakeNode("cr_ul");
  Ptr<Node> cr_ur = MakeNode("cr_ur");
  Ptr<Node> cr_rc = MakeNode("cr_rc");
  Ptr<Node> cr_lr = MakeNode("cr_lr");
  Ptr<Node> cr_ll = MakeNode("cr_ll");
  Ptr<Node> cr_lc = MakeNode("cr_lc");
  Ptr<Node> cr_ctr = MakeNode("cr_ctr");
  Ptr<Node> producer1 = MakeNode("producer1");
  Ptr<Node> server2 = MakeNode("server2");

  PointToPointHelper access;
  access.SetDeviceAttribute("DataRate", StringValue(accessRate));
  access.SetChannelAttribute("Delay", StringValue(accessDelay));
  Link(access, client1, cr_lc);
  Link(access, client2, cr_lc);

  PointToPointHelper core;
  core.SetDeviceAttribute("DataRate", StringValue(coreRate));
  core.SetChannelAttribute("Delay", StringValue(coreDelay));

  // Closed hexagon on the perimeter.
  Link(core, cr_ul, cr_ur);
  Link(core, cr_ur, cr_rc);
  Link(core, cr_rc, cr_lr);
  Link(core, cr_lr, cr_ll);
  Link(core, cr_ll, cr_lc);
  Link(core, cr_lc, cr_ul);

  // Centre links to corner quadrant routers only (not cr_lc or cr_rc).
  Link(core, cr_ctr, cr_ul);
  Link(core, cr_ctr, cr_ur);
  Link(core, cr_ctr, cr_ll);
  Link(core, cr_ctr, cr_lr);

  Link(core, cr_ur, producer1);
  Link(core, cr_lr, server2);

  ndn::StackHelper ndnHelper;
  ndnHelper.setCsSize(csSize);
  ndnHelper.setPolicy(csPolicy);
  ndnHelper.InstallAll();

  ndn::StrategyChoiceHelper::InstallAll("/", strategy);

  ndn::GlobalRoutingHelper routing;
  routing.InstallAll();

  const double stop = simTime - 1.0;
  InstallConsumer(client1, "/C1", frequency, 1.0, stop);
  InstallConsumer(client1, "/C3", frequency, 1.0, stop);
  InstallConsumer(client1, "/C5", frequency, 1.0, stop);
  InstallConsumer(client2, "/C2", frequency, 1.0, stop);
  InstallConsumer(client2, "/C4", frequency, 1.0, stop);
  InstallConsumer(client2, "/C6", frequency, 1.0, stop);
  InstallConsumer(client2, "/C1", frequency, 8.0, stop);

  InstallProducer(producer1, "/C1", payload, routing);
  InstallProducer(producer1, "/C3", payload, routing);
  InstallProducer(producer1, "/C4", payload, routing);
  InstallProducer(server2, "/C2", payload, routing);
  InstallProducer(server2, "/C5", payload, routing);
  InstallProducer(server2, "/C6", payload, routing);

  ndn::GlobalRoutingHelper::CalculateRoutes();

  ndn::L3RateTracer::InstallAll(results + "/l3-rate-trace.txt", Seconds(1.0));
  ndn::CsTracer::InstallAll(results + "/cs-trace.txt", Seconds(1.0));
  ndn::AppDelayTracer::InstallAll(results + "/app-delay-trace.txt");

  NS_LOG_UNCOND("Topology: 11 nodes (2 clients, 7 CRs, producer1, server2), 14 P2P links");
  NS_LOG_UNCOND("Strategy " << strategy);
  NS_LOG_UNCOND("CS policy " << csPolicy << ", size " << csSize << " packets");
  NS_LOG_UNCOND("Traces in " << results);

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}

