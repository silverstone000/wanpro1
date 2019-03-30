#include "lsa.h"

struct dij_dis
{
	int id, dis;
	dij_dis(const int& _id, const int& _dis)
	{
		id = _id;
		dis = _dis;
		return;
	}
};

class dij_dis_cmp
{
	bool reverse;
public:
	dij_dis_cmp(const bool& revparam = true)
	{
		reverse = revparam;
	}
	bool operator() (const dij_dis& lhs, const dij_dis&rhs) const
	{
		if (reverse)
		{
			return (lhs.dis > rhs.dis);
		}
		else
		{
			return (lhs.dis < rhs.dis);
		}
	}
};



lsa::lsa()
{





}


lsa::lsa(routerMain* m)
{
	my_msg_mtx = &(m->lsa_msg_mtx);
	nei_msg_mtx = &(m->nei_msg_mtx);
	for_msg_lsa_mtx = &(m->for_msg_lsa_mtx);
	rt_table_mtx = &(m->rt_table_mtx);

	my_msg_q = &(m->msgq_lsa);
	for_msg_lsa_q = &(m->msgq_for_lsa);

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



	while (true)
	{
		lsa_msg msg;
		
		if (_this->my_msg_q->empty())
		{
			sleep(SLEEP_TIME);
			continue;
		}
			   		 		
		_this->my_msg_mtx->lock();
		msg = _this->my_msg_q->front();
		_this->my_msg_q->pop();
		_this->my_msg_mtx->unlock();

		switch (msg.type)
		{
		case lsa_msg::inter_update:
			_this->lsa_db_mtx.lock();
			(*_this->mod)[_this->my_id] = msg.cost_map;
			_this->lsa_db_mtx.unlock();
			break;
		case lsa_msg::lsa_ack:

			break;
		case lsa_msg::lsa_adv:

			break;
		default:
			break;
		}


	}
	

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

	this_thread::sleep_for(chrono::milliseconds(SLEEP_TIME * 1000 * 3));

	while (true)
	{
		if (!_this->route_update_flag)
		{
			this_thread::sleep_for(chrono::milliseconds(SLEEP_TIME * 1000 * 3));
			continue;
		}


		_this->lsa_db_mtx.lock();
		auto *lsmap = _this->mod;
		_this->mod = _this->use;
		_this->use = lsmap;
		_this->lsa_db_mtx.unlock();


		//constructing a routing table using ls db

		map<ROUTER_ID, ROUTER_ID> route_table_constructing;
		route_table_constructing.clear();

		map<ROUTER_ID, map<ROUTER_ID, int>>::iterator ls_it;

		int rt_list_index = 0;
		vector<ROUTER_ID> router_list;
		map<ROUTER_ID, int> reverse_router_list;
		router_list.clear();
		reverse_router_list.clear();

		for (ls_it = lsmap->begin();ls_it != lsmap->end();ls_it++)
		{
			router_list.push_back(ls_it->first);
			reverse_router_list[ls_it->first] = rt_list_index;
			rt_list_index++;
		}

		//initialize a 2d array for distance look up, indexed lsdb
		vector<vector<int>> index_lsdb;
		index_lsdb = vector<vector<int>>(router_list.size(),
			vector<int>(router_list.size(), INF_DIS));
		//fill in distance data from lsdb
		for (map<ROUTER_ID, map<ROUTER_ID, int>>::iterator i = lsmap->begin();
			i != lsmap->end();i++)
		{
			for (map<ROUTER_ID, int > ::iterator j = i->second.begin();
				j != i->second.end();j++)
			{
				index_lsdb
					[reverse_router_list[i->first]]
					[reverse_router_list[j->first]] 
					= j->second;
			}
		}

		//index of self
		int ind_self = reverse_router_list[_this->my_id];


		//INF_DIS for no prev 
		vector<int> prev;
		prev = vector<int>(router_list.size(), INF_DIS);
		//visited list
		vector<bool> visited;
		visited = vector<bool>(router_list.size(), false);
		//distance from source node
		vector<int> dis_list;
		//initialize all nodes as unreachable from source node(self)
		dis_list = vector<int>(router_list.size(), INF_DIS);

		//record for next hop 2d 
		vector<int> next_hop;
		next_hop = vector<int>(router_list.size(), INF_DIS);

		dis_list[ind_self] = 0;
		
		//initialize a priority queue
		priority_queue<dij_dis, vector<dij_dis>, dij_dis_cmp>  vort_q;
		while (!vort_q.empty())
		{
			vort_q.pop();
		}
		vort_q.push(dij_dis(ind_self, dis_list[ind_self]));

		while (!vort_q.empty())
		{
			dij_dis v_cur = vort_q.top();
			vort_q.pop();
			if (visited[v_cur.id])
			{
				continue;
			}
			if (v_cur.dis >= INF_DIS)
			{
				//no reachable, no means to continue.
				break;
			}
			for (unsigned int ind_nei = 0;ind_nei < router_list.size();ind_nei++)
			{
				int alt_dis;
				if (ind_nei == v_cur.id 
					|| index_lsdb[v_cur.id][ind_nei] >= INF_DIS)
				{
					//filter out some useless condition
					continue;
				}

				//alternative distance
				alt_dis = dis_list[v_cur.id] + index_lsdb[v_cur.id][ind_nei];
				if (alt_dis < dis_list[ind_nei])
				{
					//directly get next hop
					if (v_cur.id == ind_self)
					{
						next_hop[ind_nei] = ind_nei;
					}
					else
					{
						next_hop[ind_nei] = next_hop[v_cur.id];
					}
					dis_list[ind_nei] = alt_dis;
					prev[ind_nei] = v_cur.id;

					vort_q.push(dij_dis(ind_nei, alt_dis));
				}
			}
			visited[v_cur.id] = true;
		}

		//transfer index into ROUTER_ID;
		for (unsigned int ind_nexthop = 0;ind_nexthop < router_list.size();ind_nexthop++)
		{
			if (next_hop[ind_nexthop] != INF_DIS)
			{
				route_table_constructing[router_list[ind_nexthop]] = router_list[next_hop[ind_nexthop]];
			}
		}

		//end constructing routing table

		//update routing table in use
		_this->rt_table_mtx->lock();
		*(_this->route_table) = route_table_constructing;
		_this->rt_table_mtx->unlock();
	}


}

void lsa::set_my_id(ROUTER_ID _id)
{
	my_id = _id;
	return;
}