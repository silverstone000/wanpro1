#include "neighbor.h"



neighbor::neighbor(routerMain* const m)
{
	my_msg_q = &(m->msgq_nei);
	my_msg_mtx = &(m->nei_msg_mtx);
	lsa_msg_q = &(m->msgq_lsa);
	lsa_msg_mtx = &(m->lsa_msg_mtx);
	port = m->port;
	return;
}

neighbor::neighbor()
{


}

neighbor::~neighbor()
{


}


/*
A thread for run()

- main operating thread

-> connect -> new thread for cost measure send cost to LSA
-> disconnect -> stop measure and send INF cost to LSA


*/
void neighbor::run(void* __this)
{
	neighbor* _this = (neighbor*)__this;


	/*periodic update cost to lsa with NEI_UPDATE_INTERVAL */
	thread lsa_update(lsa_nei_update,_this);
	lsa_update.detach();
	
	while (1)
	{
		nei_msg t;

		if (_this->my_msg_q->empty())
		{
			sleep(SLEEP_TIME);
			continue;
		}

		_this->my_msg_mtx->lock();
		t = _this->my_msg_q->front();
		_this->my_msg_q->pop();
		_this->my_msg_mtx->unlock();

		switch (t.type)
		{
		case NEI_CONNECT:
			/*create a new thread to measure cost*/
			_this->connect_flag[t.router_id] = true;
			thread(cost_measure, _this, t.router_id).detach();
			/*lsa part*/
			sleep(SLEEP_TIME);//wait for some cost


			break;
		case NEI_DISCONNECT:
			_this->connect_flag[t.router_id] = false;

			/*lsa part*/

			break;
		default:
			cout << "neighbor mod something wrong" << endl;
			break;
		}



	}
	

	return;


}

//ICMP header in UDP packet, because raw socket need root permission
void neighbor::cost_measure(void* __this, ROUTER_ID id)
{
	neighbor* _this = (neighbor*)__this;
	//cost from 0-65535, above concidered disconnected
	
	
	//liner mapping, 0-ECHO_TIMEOUT*1000 -> 0-65535



		if (_this->connect_flag[id] == false)
		{

			return;
		}


	return;
}


double neighbor::exp_smooth(double oldc, double newc)
{
	return oldc + EXP_SMOOTH_FACTOR * (newc - oldc);
}

void neighbor::lsa_nei_update(void* __this)
{

	neighbor* _this = (neighbor*)__this;
	//initialize time point of reference
	chrono::steady_clock::time_point tp = chrono::steady_clock::now();
	//problem: colision with other device possible

	while (true)
	{
		/*
		exit condition here
		
		*/
		if (!_this->lsa_update_flag)
		{
			return;
		}



		//fixed gap of NEI_UPDATE_INTERVAL, may jump several gaps
		while (tp < chrono::steady_clock::now())
		{
			tp += chrono::seconds(NEI_UPDATE_INTERVAL);
		}
		this_thread::sleep_until(tp);


		lsa_msg msg;
		msg.cost_map = _this->cost_map;
		msg.type = 1;//internal update
		_this->lsa_msg_mtx->lock();
		_this->lsa_msg_q->push(msg);
		_this->lsa_msg_mtx->unlock();
			   
	}
	return;
}

//for process cost measure packet reply.
void neighbor::echo_server(void* __this)
{
	neighbor* _this = (neighbor*)__this;

	//maximum packet length
	enum { max_length = 1024 };
	
	boost::asio::io_context io_context;


	udp::socket sock(io_context, udp::endpoint(udp::v4(), _this->port));

	std::cout << "udp echo server port number:" 
		<< sock.local_endpoint().port() << std::endl;

	while (true)
	{
		if (false)//exit condition
		{
			break;
		}

		char data[max_length];
		udp::endpoint sender_endpoint;
		size_t length = sock.receive_from(
			boost::asio::buffer(data, max_length), sender_endpoint);
		sock.send_to(boost::asio::buffer(data, length), sender_endpoint);
	}
}
