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
	ls_db_modified = false;
	route_table = &(m->route_table);

	id_table = &(m->id_table);
	id_table_mtx = &(m->id_table_mtx);

}

lsa::~lsa()
{
}

void lsa::run(void* __this)
{
	lsa* _this = (lsa*)__this;

	thread lsdb_update_t(lsa::lsdb_update, _this);
	thread route_update_t(lsa::route_update, _this);
	lsdb_update_t.detach();
	route_update_t.detach();


	while (_this->running_flag)
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
			_this->ls_db_modified = true;
			_this->lsa_db_mtx.unlock();
			break;
		case lsa_msg::lsa_ack:
			if (msg.seq > _this->ls_seq)
			{
				_this->sent_state[msg.router_id][1] = true;
			}
			break;
		case lsa_msg::lsa_adv:
			_this->lsa_db_mtx.lock();
			(*_this->mod)[msg.router_id] = msg.cost_map;
			_this->ls_db_modified = true;
			_this->lsa_db_mtx.unlock();
			break;
		default:
			break;
		}


	}
	

}

//start with run, used to send ls adv and resend
void lsa::lsdb_update(void* __this)
{
	lsa* _this = (lsa*)__this;
	chrono::steady_clock::time_point tp = chrono::steady_clock::now();


	while (_this->running_flag)
	{
		//first start a update than sleep

		//cost map snapshot

		_this->sent_state.clear();
		//initalize ack table according to id table.
		_this->id_table_mtx->lock();
		map<ROUTER_ID, boost::asio::ip::tcp::endpoint>
			id_table_snap =	*_this->id_table;
		_this->id_table_mtx->unlock();

		for(auto sent_s_it = id_table_snap.begin();
			sent_s_it!= id_table_snap.end();sent_s_it++)
		{
			_this->sent_state[sent_s_it->first][0] = true;
			_this->sent_state[sent_s_it->first][1] = false;
		}

		_this->lsa_db_mtx.lock();
		map<ROUTER_ID, int> cost_map_tosend = (*_this->use)[_this->my_id];
		_this->lsa_db_mtx.unlock();

		for_msg_lsa msg;
		msg.cost_map = cost_map_tosend;
		msg.type = for_msg_lsa::ls_adv;
		msg.seq = _this->ls_seq;

		_this->for_msg_lsa_q->push(msg);

		//handle time out
		chrono::steady_clock::time_point timeout_timer
			= chrono::steady_clock::now();

		while (chrono::steady_clock::now() - tp
			<= chrono::seconds(LSDB_UPDATE_INTERVAL))
		{
			this_thread::sleep_until(timeout_timer
				+ chrono::seconds(LSDB_UPDATE_TIMEOUT));
			for_msg_lsa resend_msg = msg;
			resend_msg.type = for_msg_lsa::ls_resend;
			for (auto resend_it = _this->sent_state.begin();
				resend_it != _this->sent_state.end();resend_it++)
			{
				resend_msg.router_id = resend_it->first;
				_this->for_msg_lsa_q->push(resend_msg);
			}
		}

		_this->ls_seq++;


	}
}

//periodic and triggered
void lsa::route_update(void* __this)
{
	lsa* _this = (lsa*)__this;

	this_thread::sleep_for(chrono::milliseconds(SLEEP_TIME * 1000 * 3));


	chrono::steady_clock::time_point tp = chrono::steady_clock::now();


	//route update flag is maintained by other threads, 
	//either trigered by other module or periodicly.
	while (_this->running_flag)
	{
		//if (!_this->route_update_flag)
		//{
		//	this_thread::sleep_for(chrono::milliseconds(SLEEP_TIME * 1000 * 5));
		//	continue;
		//}


		//periodic update only
		while (tp < chrono::steady_clock::now())
		{
			tp += chrono::seconds(NEI_UPDATE_INTERVAL);
		}
		this_thread::sleep_until(tp);


		_this->lsa_db_mtx.lock();
		auto *lsmap = _this->mod;
		_this->mod = _this->use;
		_this->use = lsmap;
		_this->lsa_db_mtx.unlock();

		//for time reason it is implemented directly using following 
		//code instead of put into a function
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

				//only consider cost below certain value as reachable
				if (j->second < NEIGHBOR_UNREACHABLE)
				{
					index_lsdb
					[reverse_router_list[i->first]]
				[reverse_router_list[j->first]]
				= j->second;
				}
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
					|| index_lsdb[v_cur.id][ind_nei] >= INF_DIS
					|| visited[ind_nei])
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