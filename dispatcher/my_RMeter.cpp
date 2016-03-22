#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "my_RMeter.h"
#include "do_aio.hpp"

global_args G_args;
static const char *opt_str = "t:o:d:h?";//static & const.

void display_usage(void){
	printf("light weight RaidMeter @ywj\n");
	printf("usage: %s -t <trace_file_name> [-o <result_file_name>] -d <device_name>\n", G_args.executable_name);
	exit(EXIT_FAILURE);
}

void do_args(int argc, char* argv[])//[this version] the args don't support long format.
{
	int opt = 0;
	initialze_global_args(&G_args, argv);
	while((opt = getopt(argc, argv, opt_str))!= -1){
		switch(opt){
			case 't'://-t trace_file_name
				G_args.trace_file_name = optarg;break;
			case 'o'://-o result_file_name
				G_args.result_file_name = optarg;break;
			case 'd'://-d device_name
				G_args.dev_name = optarg;break;
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
	perform_ios(G_args.dev_name);
	return 0;
}