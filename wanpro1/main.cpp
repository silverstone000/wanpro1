#include <stdio.h>
#include <iostream>
#include <mutex>

#include "routerMain.h"


#include <unistd.h>



using namespace std;



int main(int argc, char *argv[])
{
	routerMain mt;
	if (argc == 2)
	{
		mt.router_id = atoi(argv[1]);
	}
	else
	{
		cout << "useage program <id>" << endl;
	}

	mt.run();
    return 0;
}

