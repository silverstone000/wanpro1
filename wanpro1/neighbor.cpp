#include "neighbor.h"



neighbor::neighbor(routerMain* m)
{

	my_msg_q = &(m->msgq_nei);
	my_msg_mtx = &(m->nei_msg_mtx);
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
void neighbor::run()
{
	   

	
	while (1)
	{
		nei_msg t;

		if (my_msg_q->empty())
		{
			sleep(SLEEP_TIME);
			continue;
		}

		my_msg_mtx->lock();
		t = my_msg_q->front();
		my_msg_q->pop();
		my_msg_mtx->unlock();

		switch (t.type)
		{
		case NEI_CONNECT:
			/*create a new thread to measure cost*/
			connect_flag[t.router_id] = true;
			thread(cost_measure, t.router_id).detach();

			break;
		case NEI_DISCONNECT:
			connect_flag[t.router_id] = false;
			break;
		default:

			break;
		}



	}
	

	return;


}

///123213
void neighbor::cost_measure(ROUTER_ID id)
{
	while (true)
	{




		if (connect_flag[id] == false)
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