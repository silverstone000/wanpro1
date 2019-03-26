#include "neighbor.h"



neighbor::neighbor(routerMain* const m)
{
	my_msg_q = &(m->msgq_nei);
	my_msg_mtx = &(m->nei_msg_mtx);
	lsa_msg_q = &(m->msgq_lsa);
	lsa_msg_mtx = &(m->lsa_msg_mtx);
	port = m->port;
	id_table = &(m->id_table);
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
	
	try
	{
		boost::asio::io_context io_context;

		int cost = 0;
		int delay = 0;

		//    pinger p(io_context, argv[1]);
		pinger p(io_context, (*_this->id_table)[id].to_string().c_str(), 
			_this->port, &delay);

		//std::cout << 2 << std::endl;
		//sleep(1);
		std::thread t(average, &cost, &delay);
		t.detach();

		io_context.run();

	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
//		std::cout << "Exception: " << e.what() << std::endl;
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



		//a fixed gap of NEI_UPDATE_INTERVAL, may jump several gaps
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
	
	enum {
		echo_reply = 0, destination_unreachable = 3, source_quench = 4,
		redirect = 5, echo_request = 8, time_exceeded = 11, parameter_problem = 12,
		timestamp_request = 13, timestamp_reply = 14, info_request = 15,
		info_reply = 16, address_request = 17, address_reply = 18
	};

	boost::asio::io_context io_context;


	udp::socket sock(io_context, udp::endpoint(udp::v4(), _this->port));

	//std::cout << "udp echo server port number:" 
	//	<< sock.local_endpoint().port() << std::endl;

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

		//change type to echo_reply
		data[0] = echo_reply;

		sock.send_to(boost::asio::buffer(data, length), sender_endpoint);
		this_thread::yield();
	}
}

void neighbor::average(int* cost, int *delay)
{
	while (true)
	{
		*cost = *cost + 0.1 * (*delay - *cost);
		std::cout << "cost is " << *cost << std::endl;
		//		sleep(5);
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	}
}

void neighbor::update_cost(void* __this, int cost, ROUTER_ID id)
{
	neighbor* _this = (neighbor*)__this;

	_this->cost_map[id] = cost;
	return;
}