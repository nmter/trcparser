#ifndef MY_RMETER_H
#define MY_RMETER_H
#define MAX_TRC_COUNT 1000000
#define MAX_DEV_NUM 4

#include <sys/time.h>
#include <aio.h>
#include <string.h>
#include "trc_stat.h"

struct aio_data{
	double io_issue_time;
	double io_complete_time;
	struct aiocb64 *aio_cb_ptr;
	int aio_idx;
	int io_idx;
}aio_data;
	


typedef struct global_args{
	const char* trace_file_name;
	const char* result_file_name;
	const char* executable_name;
	const char* dev_name;
	const char* dev_names[MAX_DEV_NUM];
	
	double start_time;
	
	void *aio_buf;
	
	
	struct aiocb64 aio_cb_l[MAX_TRC_COUNT];
	struct aio_data aio_data_l[MAX_TRC_COUNT];
	unsigned long long io_num;
	int rate;
	int mins;
	int MultiCS;
}global_args;

//#define EXIT_FAILURE -1 redefine in stdlib.h:133
void display_usage(void);
void do_args(int argc, char* argv[]);

double get_time(void)
{
	struct	timeval	mytime;
	gettimeofday(&mytime,NULL);
	return (mytime.tv_sec*1.0+mytime.tv_usec/1000000.0);
}

void initialize_g_aio_buf(global_args *g){
	int aio_buf_size_in_bytes = 40960;
	for(int i = 0; i < sizeof(Traces_Stat_Max_Sizes)/sizeof(struct trc_stat); i++){
		if(strcmp(g->trace_file_name, Traces_Stat_Max_Sizes[i].name) == 0){
			aio_buf_size_in_bytes = Traces_Stat_Max_Sizes[i].max_size;
			break;
		}
	}
#ifdef DBG
	printf("aio buf size: %d\n", aio_buf_size_in_bytes);
#endif
	g->aio_buf = malloc(aio_buf_size_in_bytes * sizeof(char) + 1);//aio_buf_size_in_bytes is max size of io.
}

void release_g_aio_buf(global_args *g){
	free(g->aio_buf);
}

void initialze_global_args(global_args *g, char* argv[]){
	g->executable_name = argv[0];
	g->io_num = 0;
	g->result_file_name = g->dev_name = g->trace_file_name = NULL;
	g->rate = 1;
	g->mins = 30;
	g->MultiCS = 0;//Multi chunk size
	for(int i = 0; i < MAX_DEV_NUM; i++){
		g->dev_names[i] = NULL;
	}
}


void check_global_args(global_args *g){//for full-version
	const char *err_info_name[] = {"trace_file_name", "dev_name"};
	int err_catch[] = {0,0};
	int err_num = 0;
	/*check args list*/
	if(g->trace_file_name == NULL)
		err_catch[0] = 1;
	if(g->dev_name == NULL)
		err_catch[1] = 1;
	/*errors summary*/
	for(int i = 0; i < sizeof(err_catch)/sizeof(int); i++){
		if(err_catch[i]){
			printf("<%s> ", err_info_name[i]);
			err_num++;
		}	
	}
	if(err_num){
		printf("can't be null. %d args error found.\n", err_num);
		display_usage();
	}
#ifdef DBG
	printf("trace_file_name: %s, rate: %d, MultiCS: %d, dev(s):",
		g->trace_file_name, g->rate, g->MultiCS);
	if(g->MultiCS){
		printf("%s %s", g->dev_names[0], g->dev_names[1]);
	}else{
		printf("%s", g->dev_name);
	}
	printf("\n");
#endif
}
#endif