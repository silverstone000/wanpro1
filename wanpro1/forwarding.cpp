#include "forwarding.h"




forwarding::forwarding(routerMain* m)
//	:io_context(),resolver(this->io_context)
{
	route_table = &(m->route_table);
	nei_msg_mtx = &(m->nei_msg_mtx);
	lsa_msg_mtx = &(m->lsa_msg_mtx);
	my_msg_mtx = &(m->for_msg_lsa_mtx);
	rt_mtx = &(m->rt_table_mtx);

	nei_msg_q = &(m->msgq_nei);
	lsa_msg_q = &(m->msgq_lsa);
	my_msg_q = &(m->msgq_for_lsa);

	connect_flag = &(m->connect_flag);
	id_table1 = &(m->id_table1);

	my_id = &(m->router_id);
	port = &(m->port);
}




//setting flag, wait some time and exit
forwarding::~forwarding()
{
	running_flag = false;
	this_thread::sleep_for(chrono::seconds(5));
	return;
}


//connect to controller to get id, blocking method
void forwarding::initialize()
{

	*my_id = 1;//place holder for testing
	id_ready = true;
	return;
}

void forwarding::initialize(int my_port)
{
	boost::asio::io_context io_context;

	tcp::resolver resolver(io_context);

	ifstream config("config.txt", ifstream::in);
	ifstream connect("connect.txt", ifstream::in);
	ROUTER_ID id;
	config >> id;
	*my_id = id;
	string ip;
	unsigned short int t_port;
	tcp::endpoint remote;

	//load id endpoint relationship
	while (config >> id >> ip >> t_port)
	{
		//remote.address(boost::asio::ip::make_address_v4(ip));
		//remote.port(t_port);
//		cout << remote.address() << "   " << remote.port() << endl;
		(*id_table1)[id].first = ip;
		(*id_table1)[id].second = t_port;
	}
	*port = (*id_table1)[*my_id].second;

	cout << *port << endl;

	//connect routers
	int r1, r2;
	while (connect >> r1 >> r2)
	{
		nei_msg connect_msg;
		if (r1 == *my_id)
		{
			connect_msg.type = nei_msg::connect;
			connect_msg.router_id = r2;
		}
		else if (r2 == *my_id)
		{
			connect_msg.type = nei_msg::connect;
			connect_msg.router_id = r1;
		}
		else
		{
			continue;
		}
		nei_msg_mtx->lock();
		nei_msg_q->push(connect_msg);
		nei_msg_mtx->unlock();
	}




	id_ready = true;
	return;
}

//maintain message queues and forward messages
void forwarding::run(void* __this)
{
	forwarding* _this = (forwarding*)__this;
	thread tcp_server_t(forwarding::tcp_server, _this);
	tcp_server_t.detach();


	while (_this->running_flag)
	{
		if (_this->in_msg_q.empty() &&
			_this->out_msg_q.empty() && _this->my_msg_q->empty())
		{
			//this_thread::yield();
			this_thread::sleep_for(chrono::milliseconds(READ_INTERVAL));
			continue;
		}

		//if(!_this->in_msg_q.empty())
		//{
		//	json dp;
		//	_this->in_q_mtx.lock();
		//	dp = _this->in_msg_q.front();
		//	_this->in_msg_q.pop();
		//	_this->in_q_mtx.unlock();


		//}

		if (!_this->my_msg_q->empty())
		{
			for_msg_lsa msg;
			_this->my_msg_mtx->lock();
			msg = _this->my_msg_q->front();
			_this->my_msg_q->pop();
			_this->my_msg_mtx->unlock();

			json jmsg;
			jmsg.clear();
			switch (msg.type)
			{
			case for_msg_lsa::ls_resend:
				jmsg["target"] = msg.router_id;
				jmsg["source"] = *_this->my_id;
				jmsg["type"] = 1;
				jmsg["data"]["type"] = 3;
				jmsg["data"]["seq"] = msg.seq;


				for (auto cost_it = msg.cost_map.begin();
					cost_it != msg.cost_map.end();cost_it++)
				{

					jmsg["data"]["cost_map"][to_string(cost_it->first).c_str()]
						= cost_it->second;
				}
				break;
			case for_msg_lsa::ls_adv:
				jmsg["source"] = *_this->my_id;
				jmsg["type"] = 1;
				jmsg["data"]["type"] = 1;
				jmsg["data"]["seq"] = msg.seq;

				////test
				////print cost map to send
				//cout << "cost map self " << endl;
				//cout << "in forwarding, received from lsdb:" << endl;
				//for (auto mp_it = msg.cost_map.begin();
				//	mp_it != msg.cost_map.end();++mp_it)
				//{
				//	cout << "to: " << mp_it->first
				//		<< " cost: " << mp_it->second << endl;
				//}


				for (auto cost_it = msg.cost_map.begin();
					cost_it != msg.cost_map.end();cost_it++)
				{
//					cout << cost_it->first << ' ' << cost_it->second << endl;
					jmsg["data"]["cost_map"][to_string(cost_it->first).c_str()]
						= cost_it->second;
				}
				break;

			default:
				break;
			}

			//test
			//print lsdb json file being sent
			//cout << "json file to send before into out queue:" << endl;
			//cout << jmsg.dump() << endl;


			_this->out_q_mtx.lock();
			_this->out_msg_q.push(jmsg);
			_this->out_q_mtx.unlock();

		}
		if (!_this->out_msg_q.empty())
		{
			json dp;
			_this->out_q_mtx.lock();
			dp = _this->out_msg_q.front();
			_this->out_msg_q.pop();
			_this->out_q_mtx.unlock();
			if (dp["type"] == 1)
			{
				if (dp["data"]["type"] == 1)
				{


					//test
					//print lsdb json file being sent
					//cout << "inside out queue" << endl;
					//cout << "before tcp_send, json message: " << endl;
					//cout << dp.dump() << endl;


					map<ROUTER_ID, bool> connect_flag = *_this->connect_flag;

					////test
					////print connect flag
					//cout << "inside out queue:" << endl
					//	<< "current connect flag is:" << endl;
					//for (auto con_it = connect_flag.begin();
					//	con_it != connect_flag.end();con_it++)
					//{
					//	cout << "to " << con_it->first << endl
					//		<< "\t" << con_it->second << endl;
					//}



					for (auto con_it = connect_flag.begin();
						con_it != connect_flag.end();con_it++)
					{
						if (con_it->second && con_it->first != *_this->my_id)
						{
							thread(forwarding::tcp_send, _this,
								dp, con_it->first).detach();
						}
					}

				}
				else if (dp["data"]["type"] >= 2 && dp["data"]["type"] <= 4)
				{
					thread(forwarding::tcp_send, _this,
						dp, dp["target"]).detach();
				}

			}

		}

		this_thread::sleep_for(chrono::milliseconds(READ_INTERVAL));

	}

}

//used to accept tcp connection, start in run
void forwarding::tcp_server(void* __this)
{
	forwarding* _this = (forwarding*)__this;

	boost::asio::io_context io_context;
	try
	{
		tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), *_this->port));
		while (_this->running_flag)
		{
			std::thread(forwarding::tcp_session, _this, a.accept()).detach();
			this_thread::yield();
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Exception in thread tcp_server: " << e.what() << "\n";
	}
}

//accepting one message. dispatcher is here
void forwarding::tcp_session(void *__this, tcp::socket sock)
{
	forwarding* _this = (forwarding*)__this;

//	sock.non_blocking(1);
	
	string mes;
	mes.clear();
	try
	{
		while (_this->running_flag)
		{
			char data[BUFFER_LENGTH];

			boost::system::error_code error;
			size_t length = sock.read_some(boost::asio::buffer(data), error);
			if (error == boost::asio::error::eof)
			{
				//translate message and exit

				//test
				//print recieved json file
				//cout << "inside session, receive thread:" << endl;
				//cout << "reveived message, json string:" << endl;
				//cout << mes << endl;

				json msg;
				msg = json::parse(mes.begin(), mes.end());



				if (msg["type"] == 1)
				{
					//lsa_adv
					if (msg["source"] == *_this->my_id)
					{
						break;
					}
					if (msg["data"]["type"] == 1 )
					{
						//if seq number not exist
						if (_this->seq_map.count(msg["source"]) == 0)
						{
							_this->seq_map[msg["source"]] = 
								(int)msg["data"]["seq"] - 1;
						}
						//if recieved seq is smaller
						if (_this->seq_map[msg["source"]] >= msg["data"]["seq"])
						{
							break;
						}
						_this->seq_map[msg["source"]] = msg["data"]["seq"];

						lsa_msg msg_to_lsa;
						msg_to_lsa.type = lsa_msg::lsa_adv;
						msg_to_lsa.router_id = msg["source"];
						//construct a cost map
						map<ROUTER_ID, int> cost_temp;
						
						for (auto it = msg["data"]["cost_map"].begin();
							it != msg["data"]["cost_map"].end();it++)
						{
							cost_temp[stoi(it.key())] = (int)it.value();
						}


						msg_to_lsa.cost_map = cost_temp;


						//test
						//print constructed cost map
						cout << "inside session, receive thread:" << endl;
						cout << "constructed cost map:" << endl;
						for (auto mp_it = msg_to_lsa.cost_map.begin();
							mp_it != msg_to_lsa.cost_map.end();++mp_it)
						{
							cout << "to: " << mp_it->first
								<< " cost: " << mp_it->second << endl;
						}
						

						_this->lsa_msg_mtx->lock();
						_this->lsa_msg_q->push(msg_to_lsa);
						_this->lsa_msg_mtx->unlock();
						
						//construct ack and send to other neighbours
						json my_ack;

						my_ack["source"] = *_this->my_id;
						my_ack["target"] = msg["source"];
						my_ack["type"] = 1;
						my_ack["data"]["type"] = 2;
						my_ack["data"]["seq"] = msg["data"]["seq"];

						_this->out_q_mtx.lock();
						_this->out_msg_q.push(msg);
						_this->out_msg_q.push(my_ack);
						_this->out_q_mtx.unlock();


					}
					//ack
					else if (msg["data"]["type"] == 2)
					{
						if (_this->seq_map.count(msg["source"]) == 0)
						{
							_this->seq_map[msg["source"]] =
								(int)msg["data"]["seq"] - 1;
						}
						if (_this->seq_map[msg["source"]] >= msg["data"]["seq"])
						{
							break;
						}
						_this->seq_map[msg["source"]] = msg["data"]["seq"];

						lsa_msg msg_to_lsa;
						msg_to_lsa.type = lsa_msg::lsa_ack;
						msg_to_lsa.router_id = msg["source"];

						_this->lsa_msg_mtx->lock();
						_this->lsa_msg_q->push(msg_to_lsa);
						_this->lsa_msg_mtx->unlock();
					}
					//resend
					else if (msg["data"]["type"] == 3)
					{
						if (msg["target"] == *_this->my_id)
						{
							if (_this->seq_map.count(msg["source"]) == 0)
							{
								_this->seq_map[msg["source"]] =
									(int)msg["data"]["seq"] - 1;
							}
							if (_this->seq_map[msg["source"]] >= msg["data"]["seq"])
							{
								break;
							}
							_this->seq_map[msg["source"]] = msg["data"]["seq"];

							lsa_msg msg_to_lsa;
							msg_to_lsa.type = lsa_msg::lsa_adv;
							msg_to_lsa.router_id = msg["source"];
							map<ROUTER_ID, int> cost_temp;

							for (auto it = msg["data"]["cost_map"].begin();
								it != msg["data"]["cost_map"].end();it++)
							{
								cost_temp[stoi(it.key())] = (int)it.value();
							}
							msg_to_lsa.cost_map = cost_temp;

							_this->lsa_msg_mtx->lock();
							_this->lsa_msg_q->push(msg_to_lsa);
							_this->lsa_msg_mtx->unlock();
						}
						else
						{
							_this->out_q_mtx.lock();
							_this->out_msg_q.push(msg);
							_this->out_q_mtx.unlock();
						}
					}
					//file transfer(internal)
					else if (msg["data"]["type"] == 4)
					{
						if (msg["target"] == *_this->my_id)
						{
							cout << msg["data"]["data"] << endl;
							break;
						}
						else
						{
							_this->out_q_mtx.lock();
							_this->out_msg_q.push(msg);
							_this->out_q_mtx.unlock();
							//dp.data["data"] = msg["data"]["data"];
						}
					}

				}

				//switch (msg["tpye"])
				//{
				//case (nlohmann::detail::value_t) 1:


				//	break;

				//default:
				//	break;
				//}

				break;
			}
				
			// Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.

			//assemble message;
			mes.append(data, length);

//			boost::asio::write(sock, boost::asio::buffer(data, length));

			this_thread::sleep_for(chrono::milliseconds(READ_INTERVAL));
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Exception in thread tcp session: " << e.what() << "\n";
	}
}

//take json object and send to target
void forwarding::tcp_send(void *__this, json j, ROUTER_ID target)
{
	forwarding* _this = (forwarding*)__this;

	char msg[BUFFER_LENGTH];
	boost::asio::io_context io_context;

	tcp::socket s(io_context);
	tcp::resolver resolver(io_context);

	//	s.non_blocking(1);

	ROUTER_ID next_hop;

	_this->rt_mtx->lock();
	if (_this->route_table->count(target) == 0)
	{
		cout << "route to " << target << " unavailable" << endl;


		//test
		//pring what message is sent when route unreachable
		cout << "the message being droped is: "
			<< j.dump() << endl;



		_this->rt_mtx->unlock();
		return;
	}
	next_hop = _this->route_table->at(target);
	_this->rt_mtx->unlock();
	try
	{
		//auto tar_endpoint = _this->id_table->at(next_hop);
		//vector<tcp::endpoint> end_list;
		//end_list.push_back(tar_endpoint);
		boost::asio::connect(s, resolver.resolve(
			_this->id_table1->at(next_hop).first,
			to_string(_this->id_table1->at(next_hop).second)));


		int len = j.dump().size() + 1;

		strncpy(msg, j.dump().c_str(), len);


		boost::asio::write(s, boost::asio::buffer(msg, len));

		//test 
		//inside tcpsend json string being sent
		cout << "inside tcp_send :" << endl;
		cout << j.dump()<<" sent" << endl;//testing


	}
	catch (std::exception& e)
	{
		std::cout << "Exception in thread tcp_send: " << e.what() << "\n";
	}
	return;
}