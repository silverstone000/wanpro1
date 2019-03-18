#include "lsa.h"



lsa::lsa()
{





}


lsa::lsa(routerMain* m)
{
	my_msg_mtx = &(m->lsa_msg_mtx);
	nei_msg_mtx = &(m->nei_msg_mtx);
	for_msg_lsa_mtx = &(m->for_msg_lsa_mtx);
	mod = &lsa_db1;
	use = &lsa_db2;
	route_table = &(m->route_table);
}

lsa::~lsa()
{
}

void lsa::run(void* __this)
{
	lsa* _this = (lsa*)__this;






}

//triggered
void lsa::lsdb_update(void* __this)
{
	lsa* _this = (lsa*)__this;




}

//periodic and triggered
void lsa::route_update(void* __this)
{
	lsa* _this = (lsa*)__this;




}

void lsa::set_my_id(ROUTER_ID _id)
{
	my_id = _id;
	return;
}