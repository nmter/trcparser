#include "trc_parser.h"
#include "rb_tree.h"
#include "time.h"


/*
void in_tree_callback()
{
	oops++;
	//printf("oops!\n");
}
*/

extern trc_parser_t trc_parser;
extern rb_tree_t rbtree;

void test_rbt() // for key's type is int*
{
	int num = 100000,i,*key;
	srand((unsigned)time(NULL));
	for(i = 0; i < num; i++){
		key = malloc(sizeof(int));
		*key = rand() % num;
		rbtree.rb_insert(rbtree.rb_alloc_new(key));
	}
	rbtree.rb_print();
	rbtree.rb_destroy();
}
void test_trc_parser(char* argv)
{
	trc_parser.trc_file_name = argv;
	trc_parser.io_trace_generate();

}


int main(int argc, char* argv[])
{
	
	//test_rbt();
	//printf("insert:%d oops:%d\n", rbtree.num,oops);
	test_trc_parser(argv[1]);
	return 0;
}