#include <stdio.h>
#include <iostream>
#include <mutex>

#include "routerMain.h"


#include <unistd.h>



using namespace std;



int main(int argc, char *argv[])
{
	routerMain mt;
	mt.run();
    return 0;
}

