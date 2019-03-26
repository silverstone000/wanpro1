#pragma once
#include <map>
#include <chrono>

#define NEI_CONNECT 1
#define NEI_DISCONNECT 2
#define ROUTER_ID int
#define SEQ_NUM int
#define SEQ_NUM_MAX 0x303030
#define INF 0x303030

#define SLEEP_TIME 2

using namespace std;

struct nei_msg
{
	int type;
	ROUTER_ID router_id;
	/*
	1	connect
	2	disconnect
	*/
};
struct nei_msg_cost
{
	int type;
	ROUTER_ID router_id;
	SEQ_NUM seq;
	chrono::steady_clock::time_point receive_time;
	/*
	1	connect
	2	disconnect
	*/
};

struct lsa_msg
{
	enum {inter_update = 1, lsa_adv = 2, lsa_ack = 3, lsa_alive = 4};
	int type;
	ROUTER_ID router_id;
	map<ROUTER_ID, int> cost_map;
	long int seq;
	/*
	1 internal update
	2 lsa adv
	3 lsa ack
	4 lsa alive
	*/
};

struct for_msg_cost
{
	int type;
	ROUTER_ID router_id;
	SEQ_NUM seq;
	chrono::steady_clock::time_point sent_time;
	/*
	1	cost calc
	*/
};

struct for_msg_lsa
{
	int type;
	ROUTER_ID router_id;
	map<int, double> cost_map;
	chrono::steady_clock::time_point sent_time;
	long int seq;
	/*
	1	lsa alive
	2	lsa ack
	3	lsa adv
	*/
};