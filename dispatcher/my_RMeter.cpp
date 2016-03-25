/*test version - just for research. @ywj*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DBG

#include "my_RMeter.h"
#include "do_aio.hpp"

global_args G_args;

//static const char *opt_str = "t:o:d:h?";//static & const.

static const char *opt_str = "t:r:m:Xh?";//static & const.

static const char *dev_name = "/dev/md0";//single chunk 

static const char *dev_name0 = "/dev/md0";//multi-chunk 's 2 dev. smaller
static const char *dev_name1 = "/dev/md1";//larger

void display_usage(void){
	printf("light weight RaidMeter @ywj\n");
	printf("usages: %s -t <trace_file_name> [-r <rate = 1>] [-X] [-m <minutes>]\n", G_args.executable_name);
	printf("\t\t-t <trace_file_name> the input MSR trace file.\n");
	printf("\t\t-r <rate> to be faster running trace in <rate> times.\n");
	printf("\t\t-X run trace in MultiCS mode.\n");
	printf("\t\t-m <minutes> test last for <minutes> minutes.\n");
	exit(EXIT_FAILURE);
}

void do_args(int argc, char* argv[])//[this version] the args don't support long format.
{
	int opt = 0;
	initialze_global_args(&G_args, argv);
	
	G_args.dev_name = dev_name;
	G_args.dev_names[0] = dev_name0;
	G_args.dev_names[1] = dev_name1;
	
	while((opt = getopt(argc, argv, opt_str))!= -1){
		switch(opt){
			case 't'://-t trace_file_name
				G_args.trace_file_name = optarg;break;
//			case 'o'://-o result_file_name
//				G_args.result_file_name = optarg;break;
//			case 'd'://-d device_name
//				G_args.dev_name = optarg;break;
			case 'r':
				G_args.rate = atoi(optarg);break;
			case 'm':
				G_args.mins = atoi(optarg);break;
			case 'X':
				G_args.MultiCS = 1;break;
			case 'h'://-h
			case '?'://if error, will get here.
				display_usage();break;
			default ://acctually you won't get here.
				break;
		}
	}
	check_global_args(&G_args);
}


int main(int argc, char* argv[])
{
	do_args(argc, argv);
	initialize_g_aio_buf(&G_args);
	perform_ios(&G_args);
	
	//CHECK	time issues: get avg-response_time
	printf("%lf\n", check_time(&G_args));
	
	release_g_aio_buf(&G_args);
	return 0;
}