#include "naive_db.hpp"
#include <stdio.h>
int main()
{
	naive_db_rbt *db = new naive_db_rbt();
//	record *ptr;
	ULL tmp = 0;
//	FILE* fp = freopen("1.input", "r", stdin);
//	while(scanf("%llu", &tmp) != EOF){
		if(tmp == 5548077057){
			printf("!!!\n");
		}
		printf("%llu.\n", tmp);
//		db->ins(tmp);
		
//		db->travel_rbt_pr();
//	}
//	fclose(fp);
	delete db;

	return 0;
}