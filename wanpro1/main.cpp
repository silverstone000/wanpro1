#include <stdio.h>
#include <iostream>
#include <thread>
#include <queue>

#include "neighbor.h"
#include "lsa.h"
#include "routet.h"
#include "forwarding.h"

#include <unistd.h>

using namespace std;

class routerMain
{
public:

	thread::id tid;
	neighbor nei_d;
	lsa lsa_d;
	routet rou_d;
	forwarding for_d;




	void initial()
	{
		


		return;
	}
};


int main(int argc, char *argv)
{

    return 0;
}

