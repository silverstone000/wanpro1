#pragma once
#include <thread>
#include <queue>
#include <netinet/in.h>
#include <chrono>
#include <map>
#include <iostream>
#include <mutex>
#include <boost/asio.hpp>

#include "neighbor.h"
#include "lsa.h"
#include "forwarding.h"
#include "message.h"



using namespace std;





class routerMain
{
public:

	thread::id tid;

	//self id
	ROUTER_ID router_id;
	
	mutex nei_msg_mtx;
	mutex lsa_msg_mtx;
	mutex for_msg_lsa_mtx;
	mutex io_mutex;
	mutex rt_table_mtx;
	mutex id_table_mtx;
	
	queue<nei_msg> msgq_nei;
	queue<lsa_msg> msgq_lsa;
	queue<for_msg_lsa> msgq_for_lsa;
	
	//routing table
	map<ROUTER_ID, ROUTER_ID> route_table;

	map<ROUTER_ID, bool> connect_flag;

	map<ROUTER_ID, boost::asio::ip::tcp::endpoint> id_table;

	unsigned short port;

	int ls_seq = 1;


	void initialize();
	void run();
};