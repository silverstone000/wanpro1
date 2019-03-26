#pragma once

/*
modifed ping client running on udp

server reply with exeactly the same udp packet to measure link delay rtt

*/
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <istream>
#include <iostream>
#include <ostream>

#include <unistd.h>

#include "icmp_header.hpp"


//in seconds
#define ECHO_TIMEOUT 3

//in milliseconds
#define ECHO_GAP 1000

using boost::asio::ip::udp;
using boost::asio::steady_timer;


class pinger
{
public:
	pinger(boost::asio::io_context& io_context, 
		const char* destination, short port, int *delay);

private:
	void start_send();
	void handle_timeout();
	void start_receive();
	void handle_receive(std::size_t length);

	//return pid for unix
	static unsigned short get_identifier();

	udp::resolver resolver_;
	udp::endpoint destination_;
	udp::socket socket_;
	steady_timer timer_;
	unsigned short sequence_number_;
	boost::asio::chrono::steady_clock::time_point time_sent_;
	boost::asio::streambuf reply_buffer_;
	std::size_t num_replies_;

	int *delay;
};

