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
		mt.port = atoi(argv[1]);
	}
	else
	{
		cout << "useage program <port>" << endl;
	}

	mt.run();
    return 0;
}

