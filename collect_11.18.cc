#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/point-to-point-module.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include "stdlib.h"
#include <iostream>
#include <string>

#include <iostream>
#include <string>
using namespace ns3;
using namespace std;

	enum 
	{
		WQE,EA,Data,GED,CQ,ACK
	};


//flag=1 means write,flag=2 means read,flag=0 means it's going to be put into cq
struct wqe_node
{
	int len;
	int sequence;
	uint64_t source_address;
	uint64_t destination_address;
	uint32_t flag;
	uint32_t allcate;
	struct wqe_node *next;
};

struct cq_node
{
	uint32_t no;
	struct cq_node *next;
};
class myHeader : public Header
{
public:


  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;

  uint32_t flag;
  uint32_t lenth;
  uint32_t sequence;
  uint64_t src_addr;
  uint64_t dst_addr;
  uint32_t src_port;
  uint32_t dst_port;
  uint32_t allocate;

};
TypeId
myHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyHeader")
    .SetParent<Header> ()
    .AddConstructor<myHeader> ()
  ;
  return tid;
}
TypeId
myHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
myHeader::Print (std::ostream &os) const
{
  // This method is invoked by the packet printing
  // routines to print the content of my header.
  //os << "data=" << m_data << std::endl;
  // os << "data=" << m_data<<std::endl;
}
uint32_t
myHeader::GetSerializedSize (void) const
{
  // we reserve 2 bytes for our header.
  return 40;
}
void
myHeader::Serialize (Buffer::Iterator start) const
{
  // we can serialize two bytes at the start of the buffer.
  // we write them in network byte order.
  start.WriteHtonU32 (flag);
  start.WriteHtonU32 (lenth);
  start.WriteHtonU32 (sequence);
  start.WriteHtonU64 (src_addr);
  start.WriteHtonU64 (dst_addr);
  start.WriteHtonU32 (src_port);
  start.WriteHtonU32 (dst_port);
  start.WriteHtonU32 (allocate);
}
uint32_t
myHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  flag = start.ReadNtohU32 ();
  lenth = start.ReadNtohU32 ();
  sequence = start.ReadNtohU32 ();
  src_addr = start.ReadNtohU64 ();
  dst_addr = start.ReadNtohU64 ();
  src_port = start.ReadNtohU32 (); 
  dst_port = start.ReadNtohU32 ();
  allocate = start.ReadNtohU32 ();

  // we return the number of bytes effectively read.
  return 40;
}

void 
WQE_send(uint32_t xc_sequence,uint32_t xc_lenth,
	uint64_t xc_src_addr,uint64_t xc_dst_addr, uint32_t xc_src_port, uint32_t xc_dst_port,
	Ptr<Socket> sock, Address remoteAddress)
{
	cout<<"WQE_send"<<endl;
	myHeader header;
	header.flag = WQE;
	header.lenth = xc_lenth;
	header.sequence = xc_sequence;
	header.src_addr = xc_src_addr;
	header.dst_addr = xc_dst_addr;
	header.src_port = xc_src_port;
	header.dst_port = xc_dst_port;
	Ptr<Packet> packet = Create<Packet>(0);
	packet->AddHeader(header);
	cout<<"send"<<endl;
	sock->Send(packet);
	cout<<"send finished"<<endl;
}

void
EA_send(myHeader header, Ptr<Socket> sock)
{
	header.flag = EA;
	Ptr<Packet> packet = Create<Packet>(0);
	packet->AddHeader(header);
	sock->Send(packet);
}

void
Data_send(myHeader xc_header, uint32_t xc_size, Ptr<Socket> sock)
{
	myHeader header;
	header = xc_header;
	header.flag = Data;
	Ptr<Packet> packet = Create<Packet>(xc_size);
	packet->AddHeader(header);
	cout<<"wu send packet with lenth of:"<<xc_size<<"  src_addr="<<header.src_addr<<"  dst_addr="<<header.dst_addr<<endl;
	sock->Send(packet);
}

void
ACK_send(myHeader xc_header, Ptr<Socket> sock)
{
	myHeader header;
	header = xc_header;
	header.flag = ACK;
	Ptr<Packet> packet = Create<Packet>(0);
	packet->AddHeader(header);
	sock->Send(packet);
}

void
GED_send(myHeader xc_header, Ptr<Socket> sock, uint32_t size)
{
	myHeader header;
	header = xc_header;
	header.lenth = size;//复用lenth位
	header.flag = GED;
	Ptr<Packet> packet = Create<Packet>(0);
	packet->AddHeader(header);
	sock->Send(packet);
	cout<<(Simulator::Now ()).GetSeconds ()<<endl;
	cout<<"you Send GED : src_addr:"<<header.src_addr<<"  dst_addr:"<<header.dst_addr<<endl;
}

// void
// HCA::complete_recv(myHeader xc_header)
// {
// 	myHeader header;
// 	header = xc_header;
// 	header.flag = CQ;
// 	head->allcate = 0;
// 	head++;
// }

void
WQE_recv(Ptr<Packet> packet, Ptr<Socket> sock)
{
	myHeader header;
	packet->PeekHeader(header);
	// wqe_node* tail;
	// tail = host->getend();
	// tail->allcate = 1;
	// tail->source_address = header.src_addr;
	// tail->destination_address = header.dst_addr;
	// tail->sequence = header.sequence;
	// tail->dst_port = header.dst_port;
	// tail->src_port = header.src_port;
	EA_send(header,sock);
	uint32_t MSS=1,cwnd=10,total_len;
	total_len = header.lenth;
	myHeader GED_header;
	GED_header = header;
	GED_header.flag = GED;
	while(total_len>0)
	{
		for(uint32_t i=0;i<(total_len<cwnd? total_len:cwnd);i++)
		{
			GED_send(GED_header,sock,MSS);
			GED_header.src_addr+=MSS;
			GED_header.dst_addr+=MSS;
		}
		total_len-=cwnd;
	}

}

void
EA_recv(myHeader xc_header)
{
}

void
Data_recv(Ptr<Packet> packet, Ptr<Socket> sock)
{
	myHeader header;
	packet->PeekHeader(header);
	cout<<"recv packet :"
	<<" src_addr:"<<header.src_addr
	<<" size:"<<packet->GetSize()<<endl;
	Ptr<Packet> ack = Create<Packet>(0);
	header.flag = ACK;
	ack->AddHeader(header);
	sock->Send(ack);
}

void
ACK_recv(Ptr<Packet> packet)
{
	myHeader header;
	packet->PeekHeader(header);
	cout<<"recv ack :"
	<<" src_addr:"<<header.src_addr<<endl;
}

void
 GED_recv(myHeader header, Ptr<Socket> sock)
 {
 	cout<<(Simulator::Now ()).GetSeconds ()<<endl;
 	Data_send(header,header.lenth,sock);
 }


// void
// HCA::complete_recv()
// {
// 	head->allcate = 0;
// 	head++;
// }
// 

class Host{
public:
	Host();
	~Host();
	wqe_node* head;
	wqe_node* end;
	cq_node* cq_head;
	cq_node* cq_end;
	bool checkcq();
	void generatecq(uint32_t number);
	void writeTest();
	void write(uint64_t address1,uint64_t address2,uint32_t length);
	void read(uint64_t address1,uint64_t address2,uint32_t length);
	wqe_node* gethead();
	wqe_node* getend();
	void getkind(bool k);
	void getptr(Ptr<Socket> s);
	void setRemoteAddress(Address addr);
	void run();
private:
	bool kind;
	uint32_t counter;
	Ptr<Socket> sock;
	Address remoteAddress;
};
Host::Host(){
	counter=0;
	head=NULL;
	end=NULL;
	cq_head=NULL;
	cq_end=NULL;
}


Host:: ~Host(){
	delete head;
	delete cq_head;
	delete end;
	delete cq_end;
}

void Host::run(){
	Simulator::Schedule (Seconds(10.0), &Host::writeTest, this);
}

void Host::writeTest(){
	write(123,456,10);
}

void Host::getkind(bool k){
	kind=k;
}

void Host::getptr(Ptr<Socket> s){
	sock=s;
}

void Host::setRemoteAddress(Address addr){
	remoteAddress=addr;
}


void Host::write(uint64_t address1,uint64_t address2,uint32_t length){
	    cout<<"write"<<endl;
	    std::cout <<" " << Simulator::Now ().GetSeconds () << std::endl;
		// wqe_node newnode;
		// newnode.len=length;
		// newnode.source_address=address1;
		// newnode.destination_address=address2;
		// *end->next=newnode;
		// end->flag=1;
		// end=&newnode;
		// if(head==NULL){
		// 	head=&newnode;
		// }
		uint32_t sequence=0;
		WQE_send(sequence++,length,address1,address2,0,1,sock,remoteAddress);		
}

void Host::read(uint64_t address1,uint64_t address2,uint32_t length){
		wqe_node newnode;
		newnode.len=length;
		newnode.source_address=address1;
		newnode.destination_address=address2;
		*end->next=newnode;
		end->flag=2;
		end=&newnode;
		if(head==NULL){
			head=&newnode;
		}
}

void Host::generatecq(uint32_t number){
	cq_node newnode;
	newnode.no=number;
	*cq_end->next=newnode;
	cq_end=&newnode;
	if(cq_head==NULL){
		cq_head=&newnode;
	}
	counter++;
}

bool Host::checkcq(){
	if(counter>0)
		return 1;
	return 0;
}

wqe_node* Host::gethead(){
	return head;
}

wqe_node* Host::getend(){
	return end;
}

void YouRecv(Ptr<Socket> sock)
{
  cout<<"YouRecv"<<endl;
  Ptr<Packet> packet;
  while (packet = sock->Recv())
  {
    myHeader header;
    packet->PeekHeader(header);
    switch(header.flag)
    {
      case 0: cout<<"you recv a WQE!"<<endl;WQE_recv(packet, sock); break;
      //case 1: EA_recv(header);break;
      case 2: Data_recv(packet, sock);break;
      case 3: ACK_recv(packet);break;
      default: cout<<"recv error!"<<endl;
    }
  }
}

void wuRecv(Ptr<Socket> sock)
{
  cout<<"wuRecv"<<endl;
  Ptr<Packet> packet;
  while(packet = sock->Recv())
  {
    myHeader header;
    packet->PeekHeader(header);
    switch(header.flag)
    {
      case 3:cout<<"wu recv a GED"<<endl; GED_recv(header,sock);break;
      case 5:ACK_recv(packet);break;
      default:cout<<"recv error!"<<endl;
    }
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
  InetSocketAddress addr = InetSocketAddress(interfaces.GetAddress(1), 10086);

  Ptr<Socket> client = Socket::CreateSocket(nodes.Get(1), tid);
  // InetSocketAddress clientAddr = InetSocketAddress(interfaces.GetAddress(1), 10086);
  client->Bind(addr);

  // client->Connect(addr);
  // server->Connect(clientAddr);
  client->SetRecvCallback(MakeCallback(&YouRecv));

  server->Bind();
  server->Connect(addr);
  server->SetRecvCallback(MakeCallback(&wuRecv));

  Host A;
  Host B;
  A.getkind(0);
  A.getptr(server);
  A.setRemoteAddress(interfaces.GetAddress(1,0));
  B.getkind(1);
  B.getptr(client);
  B.setRemoteAddress(interfaces.GetAddress(0,0));
  // A.write(123,456,10);
  A.run();


  pointToPoint.EnablePcap ("sender", nodes.Get (0)->GetId (), 0);
  pointToPoint.EnablePcap ("receiver", nodes.Get (1)->GetId (), 0);
    // csmaHelper.EnablePcap("two", nodes);  

  Simulator::Run();
  Simulator::Stop(Seconds(100.0));
  Simulator::Destroy();
  return 0;
}
