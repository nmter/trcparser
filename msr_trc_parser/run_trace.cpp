#include "naive_db.hpp"
#include <stdio.h>
#define BLKSIZE0 512 //blk size in bytes
#define BLKSIZE1 4096 //4K 

#define BLKSIZE BLKSIZE0

unsigned long long blks_sum = 0;
ULL io_num = 0;
ULL io_num_be_split = 0;
naive_db *blk_db_from_file = new naive_db_rbt();


/*
 * one IO --> It's blks will be mixed?
 */
int msr_getblks(ULL st_idx, int size, char rw)
{
//	printf("%llu %d %c\n", st_idx, size, rw);
	unsigned long long mask;
	int blk_num, div_num;
	ULL l_idx, r_idx;//window's l,r
	ULL wt, rt;
	double dev;
	int rw_type, last_rw_type;
	/*
	 * 0 - write intensive, 90% w
	 * 1 - read intensive, 90% r
	 * 2 - wr mixed, otherwise.
	 */

	record *ptr;

	memset(&mask, 0xFF, sizeof(mask));
	mask &= ~(BLKSIZE - 1);

	st_idx &= mask;
	blk_num = size / BLKSIZE;
	blks_sum += blk_num;
	last_rw_type = -1;
	l_idx = r_idx = st_idx;
	div_num = 0;

	while(blk_num){
		ptr = blk_db_from_file->search(st_idx);
		//a blk's mapping info.
		//1. rw_intensity:
		wt = *(ULL*)get_field(ptr->value, "w_");
		rt = *(ULL*)get_field(ptr->value, "r_");
	
		//printf("%llu %llu\n", wt, rt);

		if((dev = rt == 0 ? 10.0 : (double) wt / (double)rt) > 9.0){
			rw_type = 0;
		}else if(dev < 0.11){
			rw_type = 1;
		}else{
			rw_type = 2;
		}

		if(last_rw_type < 0 || last_rw_type == rw_type){
			r_idx++;
		}else{
			//printf("%llu %llu %llu %d\n",io_num, l_idx, r_idx - 1, rw_type);//deliver io
			l_idx = r_idx;
			div_num++;
		}

		last_rw_type = rw_type;
		st_idx++;
		blk_num--;
	}
	//printf("%llu %llu %llu %d\n",io_num, l_idx, r_idx - 1, rw_type);//deliver io
	printf("===IO %llu: %d, %d.\n",io_num, size / BLKSIZE ,div_num);
	
	io_num_be_split += div_num >= 1 ? 1 : 0;
}

int msr_getIO(FILE* fp)
{
	ULL st_idx = 0;
	int size = 0, i = 0;
	char rw, c;
	while((c = fgetc(fp))!= '\n' && !feof(fp)){
		if(c == ','){
			i++;continue;
		}
		switch(i){
			case 3://R/W
				if(c == 'R'){
					rw = c;
				};
				if(c == 'W'){
					rw = c;
				};
				break;
			case 4://ofs (in bytes from MSR-Cam format)
				st_idx *= 10;
				st_idx += c - '0';
				break;
			case 5://size (in bytes from MSR-Cam format)
				size *= 10;
				size += c -'0';
				break;
		}
	}
	if(feof(fp)){
		return 1;
	}

	msr_getblks(st_idx, size, rw);
	return 0;
}


void help()
{
	printf("run: ./run_trace db_file_name trace_file_name\n");
}



int main(int argc, char* argv[])
{
	FILE *fp;

	if(argc != 3){
		help();return -1;
	}

	if((fp = fopen(argv[1], "r")) == NULL){//db_file
		perror(argv[1]);
		return -1;
	}
	fclose(fp);

	printf("rebuild db from %s...\n", argv[1]);	
	blk_db_from_file->rebuild_from(argv[1]);

	blk_db_from_file->nodes_number();

	if((fp = fopen(argv[2], "r")) == NULL){//trace_file
		perror(argv[2]);
		return -1;
	}

	io_num = 0;
	while(msr_getIO(fp) == 0){
		io_num++;
	}

	printf("---Number of IOs: %llu, be splited: %llu\n", io_num, io_num_be_split);

//	blk_db_from_file->travel_rbt_pr();
//	
}