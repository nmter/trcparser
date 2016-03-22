#ifndef MY_RMETER_H
#define MY_RMETER_H
typedef struct global_args{
	const char* trace_file_name;
	const char* result_file_name;
	const char* executable_name;
	const char* dev_name;
}global_args;

//#define EXIT_FAILURE -1 redefine in stdlib.h:133
void display_usage(void);
void do_args(int argc, char* argv[]);



void initialze_global_args(global_args *g, char* argv[]){
	g->executable_name = argv[0];
	g->result_file_name = g->dev_name = g->trace_file_name = NULL;
}

void check_global_args(global_args *g){
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

}
#endif