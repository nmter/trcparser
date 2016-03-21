#include "naive_db.hpp"
#include <stdio.h>
#include <stdlib.h>
#define BLKSIZE0 512 //blk size in bytes
#define BLKSIZE1 4096 //4K 

#define BLKSIZE BLKSIZE0

unsigned long long blks_sum = 0;
ULL io_num = 0;
ULL io_num_be_split = 0;
ULL R_RI_num = 0, W_RI_num = 0;
ULL R_WI_num = 0, W_WI_num = 0;
ULL R_MIX_num = 0, W_MIX_num = 0;
ULL R_split_num = 0, W_split_num = 0;
ULL wr_t = 0;
naive_db *blk_db_from_file = new naive_db_rbt();

ULL *rptr[3] = {&R_WI_num, &R_RI_num, &R_MIX_num}; 
ULL *wptr[3] = {&W_WI_num, &W_RI_num, &W_MIX_num}; 
void update_stat(int rw_type, char rw){
	ULL **opr = rw == 'R' ? rptr : wptr;
	(*opr[rw_type])++;
}
void print_stats(int mode)
{
	if(mode){
		printf("IO_Sum: %llu, be splited: %llu, read to be splited: %llu, write to be spilted: %llu\n",io_num, io_num_be_split, R_split_num, W_split_num);
		printf("Read access to RI_BLKS: %llu, WI_BLKS: %llu, MIX_BLKS: %llu\n", R_RI_num, R_WI_num, R_MIX_num);
		printf("Write access to RI_BLKS: %llu, WI_BLKS: %llu, MIX_BLKS: %llu\n", W_RI_num, W_WI_num, W_MIX_num);
	}else{
		printf("%llu:%llu:%llu:%llu:%llu:%llu:%llu:%llu:%llu:%llu\n", 
			io_num, io_num_be_split,
			R_RI_num + R_WI_num + R_MIX_num + R_split_num, R_RI_num, R_WI_num, R_MIX_num,
			W_RI_num + W_WI_num + W_MIX_num + W_split_num, W_RI_num, W_WI_num, W_MIX_num);
	}
}


#define MAX(a, b) ((a) > (b) ? (a) : (b))
/*
 * one IO --> It's blks will be mixed?
 */

int msr_getblks(ULL st_idx, int size, char rw)
{
//	printf("%llu %d %c\n", st_idx, size, rw);
	unsigned long long mask;
	int blk_num, div_num;
	ULL l_idx, r_idx;//window's l,r
	ULL wt, rt, st, ma, mi;
	double dev;
	int rw_type, last_rw_type;
	struct value_t* v_ptr;
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
		if(ptr->key != st_idx){
			printf("run_trace error - \n");
			exit(-1);
		}
		//a blk's mapping info.
		//1. rw_intensity:
		wt = *(ULL*)get_field(ptr->value, "w_");
		rt = *(ULL*)get_field(ptr->value, "r_");
		ma = *(int*)get_field(ptr->value, "ma");
		mi = *(int*)get_field(ptr->value, "mi");
		st = *(int*)get_field(ptr->value, "st");
		

		wr_t = MAX(MAX(wt,rt),wr_t);
		//printf("%llu %llu\n", wt, rt);
		
		
		if((dev = rt == 0 ? 10.0 : (double) wt / (double)rt) > 9.0){
			rw_type = 0;
		}else if(dev < 0.11){
			rw_type = 1;
		}else{
			rw_type = 2;
		}
		
		v_ptr = (struct value_t*)ptr->value;
		
	/*
		if(mi == 0){//blk never be read.
			printf("%llu %llu %d %d %d %d\n", 
				v_ptr->wt, v_ptr->rt, v_ptr->ma, 
				v_ptr->mi, v_ptr->av, v_ptr->st);
		}
	*/	
	
		if(last_rw_type < 0 || last_rw_type == rw_type){
			r_idx++;
		}else{
			//printf("%llu %llu %llu %d\n",io_num, l_idx, r_idx - 1, rw_type);//===>deliver io
			l_idx = r_idx;
			div_num++;
		}

		last_rw_type = rw_type;
		st_idx++;
		blk_num--;
	}
	//printf("%llu %llu %llu %d\n",io_num, l_idx, r_idx - 1, rw_type);//===>deliver io
/*
	if(div_num)//output the spilted_num
		printf("===IO %llu: %d, %d.\n",io_num, size / BLKSIZE ,div_num);
*/	
	io_num_be_split += div_num >= 1 ? 1 : 0;
	if(div_num > 0){
		switch(rw){
			case 'W':
				W_split_num++;break;
			case 'R':
				R_split_num++;break;
		}
	}else{
		update_stat(rw_type, rw);
	}
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

//	printf("rebuild db from %s...\n", argv[1]);	
	blk_db_from_file->rebuild_from(argv[1]);

//	blk_db_from_file->nodes_number();

	if((fp = fopen(argv[2], "r")) == NULL){//trace_file
		perror(argv[2]);
		return -1;
	}

	io_num = 0;
	while(msr_getIO(fp) == 0){
		io_num++;
	}

//	printf("---Number of IOs: %llu, be splited: %llu, \n", io_num, io_num_be_split);
//	printf("MAX wr_t %llu.\n", wr_t);
//	blk_db_from_file->travel_rbt_pr();
//	printf("No zero std blk: %llu (of %llu)\n", no_zero_std, blk_db_from_file->num);
	printf("%s:",argv[2]);
	print_stats(0);
	delete blk_db_from_file;
	return 0;
//	blk_db_from_file->travel_rbt_pr();
//	
}