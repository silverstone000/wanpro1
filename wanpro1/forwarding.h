#pragma once

#include <unistd.h>
#include <boost/asio.hpp>

class forwarding
{
public:

	map<ROUTER_ID, ROUTER_ID> *route_table;





	forwarding();
	~forwarding();
};
