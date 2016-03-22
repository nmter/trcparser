
typedef struct global_args{
	const char* trace_file_name;
	const char* result_file_name;
	const char* executable_name;
	const char* dev_name;
}global_args;

#define EXIT_FAILURE -1
#define EXIT_SUCCESS 0

void initialze_global_args(global_args *g, char* argv[]){
	g->executable_name = argv[0];
	g->result_file_name = g->dev_name = g->trace_file_name = NULL;
}