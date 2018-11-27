#include"ns3/ptr.h"
#include"ns3/packet.h"
#include"ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
using namespace ns3;
void ServerRecv(Ptr<Socket> sock)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = sock->RecvFrom (from)))
  {
    std::cout<<"Server recv packet of size:"<<packet->GetSize ()<<std::endl;
    sock->SendTo(packet,0,from);
  }
}

void ClientRecv(Ptr<Socket> sock)
{
  Ptr<Packet> packet;
  while(packet = sock->Recv())
  {
    std::cout<<"Client recv packet of size:"<<packet->GetSize ()<<std::endl;
  }
}
int main(){
  NodeContainer nodes;
  nodes.Create(2);

  InternetStackHelper stack;
  stack.Install(nodes);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("500Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);
  // CsmaHelper csmaHelper;
  // NetDeviceContainer cmsaNetDevice = csmaHelper.Install(nodes);

  Ipv4AddressHelper addressHelper;
  addressHelper.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = addressHelper.Assign(devices);

  //server sockets
  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  Ptr<Socket> server = Socket::CreateSocket(nodes.Get(0), tid);
  InetSocketAddress addr = InetSocketAddress(interfaces.GetAddress(0), 10086);
  server->Bind(addr);

  Ptr<Socket> client = Socket::CreateSocket(nodes.Get(1), tid);
  // InetSocketAddress clientAddr = InetSocketAddress(interfaces.GetAddress(1), 10086);
  // client->Bind(addr);

  client->Connect(addr);
  client->SetRecvCallback(MakeCallback(&ClientRecv));
  server->SetRecvCallback(MakeCallback(&ServerRecv));
  Ptr<Packet> packet = Create<Packet>(100);
  client->Send(packet);
  // server->Connect(clientAddr);
  // client->SetRecvCallback(MakeCallback(&YouRecv));

  // server->Bind();
  // server->Connect(addr);
  // server->SetRecvCallback(MakeCallback(&wuRecv));

  // Host A;
  // Host B;
  // A.getkind(0);
  // A.getptr(server);
  // A.setRemoteAddress(interfaces.GetAddress(1,0));
  // B.getkind(1);
  // B.getptr(client);
  // B.setRemoteAddress(interfaces.GetAddress(0,0));
  // // A.write(123,456,10);
  // A.run();


  // pointToPoint.EnablePcap ("sender", nodes.Get (0)->GetId (), 0);
  // pointToPoint.EnablePcap ("receiver", nodes.Get (1)->GetId (), 0);
  //   // csmaHelper.EnablePcap("two", nodes);  

  Simulator::Run();
  Simulator::Stop(Seconds(10.0));
  Simulator::Destroy();
  return 0;
}
