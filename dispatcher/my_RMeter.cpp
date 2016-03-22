#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "my_RMeter.h"

global_args G_args;

void usage(void)
{
 	printf("usage:%s [-t tracefilename] [-r resultfilename] [-p processnumber] [-m time] [-nnumber] dev\n",myname);
	printf("      [-t tracefilename]:\n");
	printf("      [-r resultfilename]:\n");
	printf("      [-c raid capacity(KB)]:\n");
	printf("      [-i timescale(default:1, faster 2+)]:\n");
	printf("      [-a rangescale(default:0, bigger 2+)]:\n");
	printf("      [-m deal time]:\n");
	printf("      [-n deal number]:\n");
	printf("      dev:\n");
}



int main(int argc, char* argv[])
{
	
	return 0;
}