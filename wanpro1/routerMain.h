#pragma once
#include <thread>
#include <queue>
#include <netinet/in.h>
#include <chrono>
#include <map>

#include "neighbor.h"
#include "lsa.h"
#include "forwarding.h"

using namespace std;

class routerMain
{
public:

	thread::id tid;

	int router_id;
	
	struct nei_msg 
	{
		int type;
		int router_id;
		/*
		1	connect
		2	disconnect
		*/
	};
	struct lsa_msg
	{
		int type;
		int router_id;
		map<int, int> cost_map;
	};
	struct for_msg
	{
		int type;
		int router_id;
		chrono::steady_clock::time_point sent_time;
		/*
		1	cost calc
		2	lsa alive
		3	lsa ack
		4	lsa adv
		*/
	};
	queue<nei_msg> msgq_nei;
	queue<lsa_msg> msgq_lsa;
	queue<for_msg> msgq_for;

	

	void initialize();
	void run();
};