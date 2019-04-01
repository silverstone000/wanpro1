#include "routerMain.h"






void routerMain::initialize()
{



	return;
}


void routerMain::run()
{
	forwarding for_d(this);
	neighbor nei_d(this);
	lsa lsa_d(this);


	for_d.initialize();
	

	while (!for_d.id_ready)
	{
		this_thread::sleep_for(chrono::milliseconds(500));
	}


	thread for_t(forwarding::run, &for_d);
	this_thread::sleep_for(chrono::milliseconds(500));
	thread nei_t(neighbor::run, &nei_d);
	thread lsa_t(lsa::run, &lsa_d);

	for_t.join();
	nei_t.join();
	lsa_t.join();


}

