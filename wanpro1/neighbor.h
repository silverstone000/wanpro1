#pragma once
#include <queue>
#include <mutex>
#include <map>
#include <unistd.h>
#include <list>
#include "routerMain.h"
#include "message.h"



using namespace std;

class routerMain;



class neighbor
{
public:
	
	
	mutex *my_msg_mtx;
	queue<nei_msg> *my_msg_q;
	map<ROUTER_ID, double> cost_map;
	map<ROUTER_ID, bool> connect_flag;

	neighbor(routerMain* m);

	neighbor();
	~neighbor();

	void run();

	void cost_measure(ROUTER_ID id);

	double exp_smooth(double oldc, double newc);

};
