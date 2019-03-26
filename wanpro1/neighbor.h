#pragma once

#include <unistd.h>
#include <list>

#include <stdlib.h>
#include <string>
#include <boost/asio.hpp>

#include "routerMain.h"
#include "message.h"
#include "icmp/pinger.h"

//in seconds
#define NEI_UPDATE_INTERVAL 3
#define EXP_SMOOTH_FACTOR 0.1

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

	mutex cost_map_mtx;

	//cost of neighbors
	map<ROUTER_ID, int> cost_map;

	//for terminate cost measure thread
	map<ROUTER_ID, bool> connect_flag;

	map<ROUTER_ID, boost::asio::ip::address_v4> *id_table;

	short int port;

	//for terminate lsa update thread
	bool lsa_update_flag = true;
	
	thread echo_server_t;
	thread msg_proc_t;


	neighbor(routerMain* const m);

	neighbor();
	~neighbor();

	static void run(void* __this);

	//ping like function for delay measure udp echo client
	static void cost_measure(void* __this, ROUTER_ID id);


	//periodic update cost information to lsa module
	static void lsa_nei_update(void* __this);

	//blocking UDP echo server for process cost measure packet reply.
	//running on a seperate thread
	//sync method with yield()
	static void echo_server(void* __this);

	static void average(void* __this, int* cost, int *delay, ROUTER_ID id);

	static void update_cost(void* __this, int cost, ROUTER_ID id);

};
