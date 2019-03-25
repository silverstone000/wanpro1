#pragma once

#include <unistd.h>
#include <list>

#include <stdlib.h>

#include <boost/asio.hpp>

#include "routerMain.h"
#include "message.h"
#include "icmp/pinger.h"

//in seconds
#define NEI_UPDATE_INTERVAL 3


using namespace std;

using boost::asio::ip::udp;

class routerMain;



class neighbor
{
public:
	
	
	mutex *my_msg_mtx;
	mutex *lsa_msg_mtx;
	queue<nei_msg> *my_msg_q;
	queue<lsa_msg> *lsa_msg_q;

	//cost of neighbors
	map<ROUTER_ID, int> cost_map;

	//for terminate cost measure thread
	map<ROUTER_ID, bool> connect_flag;

	short port;

	//for terminate lsa update thread
	bool lsa_update_flag = true;

	neighbor(routerMain* const m);

	neighbor();
	~neighbor();

	static void run(void* __this);

	//ping like function for delay measure udp echo client
	static void cost_measure(void* __this, ROUTER_ID id);

	static double exp_smooth(double oldc, double newc);

	//periodic update cost information to lsa module
	static void lsa_nei_update(void* __this);

	//blocking UDP echo server for process cost measure packet reply.
	//running on a seperate thread
	//block on call
	static void echo_server(void* __this);
};
