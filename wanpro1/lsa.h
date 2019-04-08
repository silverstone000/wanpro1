#pragma once
#include <unistd.h>

#include <vector>
#include <queue>

#include "routerMain.h"
#include "message.h"


using namespace std;

class routerMain;

#define LSDB_UPDATE_INTERVAL 15
#define LSDB_UPDATE_TIMEOUT 5



class lsa
{
public:
	//lock for both db
	mutex lsa_db_mtx;
	mutex *my_msg_mtx;
	mutex *nei_msg_mtx;
	mutex *for_msg_lsa_mtx;
	mutex *rt_table_mtx;
	mutex *id_table_mtx;

	queue<lsa_msg> *my_msg_q;
	queue<nei_msg> *nei_msg_q;
	queue<for_msg_lsa> *for_msg_lsa_q;

	
	ROUTER_ID *my_id;

	map<ROUTER_ID, map<ROUTER_ID, int>> lsa_db1, lsa_db2;

	//modifying and using, pointing to two dbs
	map<ROUTER_ID, map<ROUTER_ID, int>> *mod, *use;
	bool ls_db_modified;

	
	//LS advertisement exchange state
	map<ROUTER_ID, bool[2]> sent_state;

	map<ROUTER_ID, ROUTER_ID> *route_table;

//	map<ROUTER_ID, boost::asio::ip::tcp::endpoint> *id_table;

	map < ROUTER_ID, pair<string, unsigned short>> *id_table1;

	map<ROUTER_ID, bool> *connect_flag;

	bool route_update_flag;

	bool running_flag = true;

	int *ls_seq;

	lsa();
	lsa(routerMain* m);
	~lsa();

	
	static void run(void* __this);

	static void lsdb_update(void* __this);

	static void route_update(void* __this);



	void set_my_id(ROUTER_ID _id);
};
