#pragma once

#include <unistd.h>
#include "boost/asio.hpp"
#include "routerMain.h"
#include "message.h"



//using namespace std;

	class forwarding
	{
	public:

		map<ROUTER_ID, ROUTER_ID> *route_table;





		forwarding();
		~forwarding();
	};
