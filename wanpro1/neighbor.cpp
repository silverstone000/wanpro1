#include "neighbor.h"



neighbor::neighbor(routerMain* m)
{
	my_msg_q = &(m->msgq_nei);
	my_msg_mtx = &(m->nei_msg_mtx);
	lsa_msg_q = &(m->msgq_lsa);
	lsa_msg_mtx = &(m->lsa_msg_mtx);
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
			thread(cost_measure, t.router_id, _this).detach();
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

///123213
void neighbor::cost_measure(ROUTER_ID id, void* __this)
{
	neighbor* _this = (neighbor*)__this;
	while (true)
	{




		if (_this->connect_flag[id] == false)
		{

			return;
		}
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