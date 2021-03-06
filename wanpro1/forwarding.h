#pragma once

#include <unistd.h>
#include <boost/asio.hpp>

#include <utility>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <fstream>

#include "routerMain.h"
#include "message.h"

using boost::asio::ip::tcp;

#define BUFFER_LENGTH 4096

//in milliseconds
#define READ_INTERVAL 50

//waiting time for creation of rtconfig files in seconds
#define STARTUP_TIME 20


class routerMain;

class forwarding
{
public:


	map<ROUTER_ID, ROUTER_ID> *route_table;
	mutex *nei_msg_mtx;
	mutex *lsa_msg_mtx;
	mutex *my_msg_mtx;
	mutex in_q_mtx, out_q_mtx;
	mutex *rt_mtx;

	queue<nei_msg> *nei_msg_q;
	queue<lsa_msg> *lsa_msg_q;
	queue<for_msg_lsa> *my_msg_q;

	map<ROUTER_ID, bool> *connect_flag;

	//	map<ROUTER_ID, boost::asio::ip::tcp::endpoint> *id_table;

	map < ROUTER_ID, pair<string, unsigned short>> *id_table1;

	thread tcp_server_t;

	queue<json> in_msg_q, out_msg_q;



	//map<ROUTER_ID, tcp::socket> soc_map;

	map<ROUTER_ID, int> seq_map;

	ROUTER_ID *my_id;

	unsigned short int *port;

//	boost::asio::io_context io_context;

	//tcp::resolver resolver;

	bool running_flag = true;

	//set when get a router id
	bool id_ready = false;

	forwarding(routerMain* m);

	~forwarding();

	void initialize();

	void initialize(int my_port);

	void initialize(int my_id, int dum);

	static void run(void* __this);

	static void tcp_server(void* __this);

	static void tcp_session(void *__this, tcp::socket sock);

	static void tcp_send(void *__this, json j, ROUTER_ID target);
};


class session
	: public std::enable_shared_from_this<session>
{
public:
	session(tcp::socket socket)
		: socket_(std::move(socket))
	{
	}

	void start()
	{
		do_read();
	}

private:
	void do_read()
	{
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
			[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				do_write(length);
			}
		});
	}

	void do_write(std::size_t length)
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
			[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				do_read();
			}
		});
	}

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
};