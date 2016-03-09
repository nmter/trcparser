#include "naive_db.hpp"
#include <stdio.h>
int main()
{
	naive_db_rbt *db = new naive_db_rbt();
	record *ptr;
	ULL tmp = 0;
//	FILE* fp = fopen("1.input", "r");
	
	db->rebuild_from("1.input");
	db->nodes_number();
	
	delete db;

	return 0;
}