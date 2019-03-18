#pragma once

#include <unistd.h>
#include <list>
#include "routerMain.h"
#include "message.h"

//in seconds
#define NEI_UPDATE_INTERVAL 3


using namespace std;

class routerMain;



class neighbor
{
public:
	
	
	mutex *my_msg_mtx;
	mutex *lsa_msg_mtx;
	queue<nei_msg> *my_msg_q;
	queue<lsa_msg> *lsa_msg_q;

	//cost of neighbors
	map<ROUTER_ID, double> cost_map;

	//for terminate cost measure thread
	map<ROUTER_ID, bool> connect_flag;

	//for terminate lsa update thread
	bool lsa_update_flag = true;

	neighbor(routerMain* m);

	neighbor();
	~neighbor();

	static void run(void* __this);

	static void cost_measure(ROUTER_ID id, void* __this);

	double exp_smooth(double oldc, double newc);

	static void lsa_nei_update(void* __this);

};
