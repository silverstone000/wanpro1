#include "forwarding.h"




forwarding::forwarding(routerMain* m)
{
	route_table = &(m->route_table);
	nei_msg_mtx = &(m->nei_msg_mtx);
	lsa_msg_mtx = &(m->lsa_msg_mtx);
	my_msg_mtx = &(m->for_msg_lsa_mtx);

	nei_msg_q = &(m->msgq_nei);
	lsa_msg_q = &(m->msgq_lsa);
	my_msg_q = &(m->msgq_for_lsa);

	my_id = &(m->router_id);
	
}




//setting flag, wait some time and exit
forwarding::~forwarding()
{
}


//connect to controller to get id, blocking method
void forwarding::initialize()
{

	*my_id = 1;//place holder for testing
	id_ready = true;
	return;
}

//maintain message queues and forward messages
void forwarding::run(void* __this)
{

}

//used to accept tcp connection, start during 
void forwarding::tcp_server(void* __this)
{

}

