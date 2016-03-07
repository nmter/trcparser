#include "naive_db.hpp"
#include <stdio.h>
int main()
{
	naive_db *db = new naive_db_rbt();
	record *ptr;
	for(int i = 0; i < 20; i++){
		ptr = db->ins(i);
	}
	delete db;
	
	return 0;
}