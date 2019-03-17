#pragma once
#include <thread>
#include <queue>
#include <netinet/in.h>
#include <chrono>
#include <map>
#include <iostream>

#include "neighbor.h"
#include "lsa.h"
#include "forwarding.h"
#include "message.h"



using namespace std;





class routerMain
{
public:

	thread::id tid;

	int router_id;
	
	mutex nei_msg_mtx;
	mutex lsa_msg_mtx;
	mutex for_msg_cost_mtx;
	mutex for_msg_lsa_mtx;
	
	queue<nei_msg> msgq_nei;
	queue<lsa_msg> msgq_lsa;
	queue<for_msg_cost> msgq_for_cost;
	queue<for_msg_lsa> msgq_for_lsa;
	

	void initialize();
	void run();
};