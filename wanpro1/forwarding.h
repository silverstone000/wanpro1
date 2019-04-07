#pragma once

#include <unistd.h>
#include <boost/asio.hpp>

#include <utility>
#include <memory>

#include "routerMain.h"
#include "message.h"

using boost::asio::ip::tcp;

#define BUFFER_LENGTH 1024
#define READ_INTERVAL 50

class routerMain;

class forwarding
{
public:


	map<ROUTER_ID, ROUTER_ID> *route_table;
	mutex *nei_msg_mtx;
	mutex *lsa_msg_mtx;
	mutex *my_msg_mtx;

	queue<nei_msg> *nei_msg_q;
	queue<lsa_msg> *lsa_msg_q;
	queue<for_msg_lsa> *my_msg_q;

	thread tcp_server_t;

	map<ROUTER_ID, queue< data_payload>> in_msg_q, out_msg_q;

	map<ROUTER_ID, tcp::socket> soc_map;

	ROUTER_ID *my_id;

	unsigned short int *port;

	boost::asio::io_context io_context;

	bool running_flag = true;

	//set when get a router id
	bool id_ready = false;

	forwarding(routerMain* m);

	~forwarding();

	void initialize();

	static void run(void* __this);

	static void tcp_server(void* __this);

	static void tcp_session(void *__this, tcp::socket sock);


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