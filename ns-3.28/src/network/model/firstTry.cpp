#include<iostream>

using namespace std;

class myheader:public Header
{
public:
    uint64_t remote_addr;
    uint64_t local_addr;
    uint16_t flag;
    uint32_t data_lenth;
    uint32_t sequence;
    uint32_t dst_port;
    uint32_t src_port;
    uint16_t Packet_type;
};

struct WQE
{
    uint64_t local_addr;
    uint64_t remote_addr;
    uint16_t flag;
    uint32_t data_lenth;
    uint32_t sequence;
    uint8_t allcated;//1则表示以占用，0表示可覆盖
};
class myfun
{
public:
    myfun();
    virtual ~myfun();
    void initial_qp(struct WQE*);
    void RDMA_send(Ptr<Socket>, void*);
    void RDMA_recv(Ptr<Socket> ,void* ,Ptr<Packet>);
    void RDMA_write(void* , uint32_t , struct WQE*);
    void RDMA_read(void*, uint32_t , struct WQE*);
    void* mem(uint32_t);
    void set_addr(void*)
    
private:
	uint32_t set_sequence(void);
    void *allocate_mem_to_store(uint32_t len);
    uint64_t m_remote_addr;
    uint64_t m_local_addr;
};
myfun::myfun(){}
myfun::~myfun(){}

struct WQE*
myfun::initial_qp(struct WQE *QP)
{
    static struct WQE queue_pair[10];
    return queue_pair;
}

Ptr<Packet>
myfun::RDMA_send(Ptr<Socket> sock,void* local_addr)
{
    myheader sh;
    sh.local_addr = local_addr;
    sh.flag = 0;
    Ptr<Packet> sp = Create<Packet>(10);
    sp->AddHeader(sh);
    return sp;
}

void
myfun::RDMA_recv(Ptr<Socket> sock, void *addr_to_recv ,Ptr<Packet> rp)
{
    myheader rh;
    rp->PeekHeader(rh);
    m_remote_addr = rp.local_addr;
}

void
myfun::RDMA_write(void* _context, uint32_t len)
{
    sq_send->remote_addr = m_remote_addr;
    sq_send->local_addr = _context;
    sq_send->data_len = len;
    sq_send->flag = 1;
    sq_send->sequence = set_sequence();
    sq_send->allcated = 1;
    sq_send++;
}

void
myfun::RDMA_read(void *Mem_to_store_in, uint32_t mem_len,)
{
    sq_send.remote_addr = m_remote_addr;
    sq_send.local_addr = Mem_to_store_in;
    sq_send.data_len = mem_len;
    sq_send.flag = 2;
    sq_send->sequence = set_sequence();
    sq_send->allcated = 1;
    sq_send++;
}

void*
myfun::allocate_mem_to_store(uint32_t len)
{
	void *b = (void*)(new char[len]);
	return b;
}

uint32_t
myfun::set_sequence(void)  //仅支持点对点
{
	static seq = 0;
	if(seq == 100000)
	return	seq = 0;
	return ++seq;
}

void*
myfun::mem(uint32_t len)
{
	return *allocate_mem_to_store(uint32_t len);
}

void
set_addr(void* in)
{
    m_local_addr = in;
}