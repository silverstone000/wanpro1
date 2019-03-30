#pragma once
#include <map>
#include <chrono>


#define ROUTER_ID int
#define SEQ_NUM int
#define SEQ_NUM_MAX 0x303030

//ls cost deem as unreachable
#define INF_DIS 0x30303030

//cost value, if above, host deem unreachable as neighbor
#define NEIGHBOR_UNREACHABLE 65536

#define SLEEP_TIME 2

using namespace std;

struct nei_msg
{
	enum 
	{
		connect = 1, 
		disconnect = 2
	};
	int type;
	ROUTER_ID router_id;
	/*
	1	connect
	2	disconnect
	*/
};

//struct nei_msg_cost
//{
//
//	int type;
//	ROUTER_ID router_id;
//	SEQ_NUM seq;
//	chrono::steady_clock::time_point receive_time;
//	/*
//	1	connect
//	2	disconnect
//	*/
//};

struct lsa_msg
{
	enum {
		inter_update = 1, 
		lsa_adv = 2, 
		lsa_ack = 3, 
		lsa_alive = 4};
	int type;
	ROUTER_ID router_id;
	map<ROUTER_ID, int> cost_map;
	long int seq;
};

//struct for_msg_cost
//{
//	int type;
//	ROUTER_ID router_id;
//	SEQ_NUM seq;
//	chrono::steady_clock::time_point sent_time;
//	/*
//	1	cost calc
//	*/
//};

struct for_msg_lsa
{
	enum
	{
		ls_adv = 1,
		ls_ack = 2
	};
	int type;
	ROUTER_ID router_id;
	map<int, double> cost_map;
	chrono::steady_clock::time_point sent_time;
	long int seq;

};