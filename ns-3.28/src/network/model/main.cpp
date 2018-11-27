int main()
{
	//server sockets
	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	Ptr<Socket> server = Socket::CreateSocket(nodes.Get(0), tid);
	InetSocketAddress addr = InetSocketAddress(Ipv4Address::GetAny(), 10086);
	server->Bind(addr);

	Ptr<Socket> client = Socket::CreateSocket(nodes.Get(4), tid);
	InetSocketAddress serverAddr = InetSocketAddress(interfaces.GetAddress(0), 10086);
	client->Connect(serverAddr);

	myfun Server;
	myfun Client;

	struct WQE *client_SQ_ptr, *client_RQ_ptr, *server_SQ_ptr, *server_RQ_ptr;

	server_SQ_ptr = Server.initial_qp();
	server_RQ_ptr = Server.initial_qp();

	client_SQ_ptr = Client.initial_qp();
	client_RQ_ptr = Client.initial_qp();

	



}