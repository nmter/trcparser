#include <stdio.h>
#include <string.h>
#define LEN 32
#define LMAX 64

typedef struct LargeNum{
	char buf[LMAX];
	int idx;
}LARGENUM;

void init_LN(LARGENUM *x){
	memset(x->buf, 0, sizeof(x->buf));
	x->idx = 0;
}
void insert_LN(LARGENUM *x, char n){
	x->buf[x->idx++] = n;
}
char* strval_LN(LARGENUM *x){
	return x->buf;
}



void help()
{
	printf("run: ./msr_gen_blk_db trace_file_name\n");
}


#define BLKSIZE0 512 //blk size in bytes
#define BLKSIZE1 4096 //4K 

#define BLKSIZE BLKSIZE0
#include "naive_db.hpp"

unsigned long long blks_sum = 0;
unsigned long long blks_sum_before = 0;
naive_db *blk_db = new naive_db_rbt();//insert IOblks into db.



int msr_toIOblks(LARGENUM *ofs, int size, char rw)
{
	unsigned long long blk_id = 0, mask;

	int blk_num;
	char* ptr = ofs->buf;
	

	record * rcd_ptr;

	ULL tmp_sum_size, *tmp_ptr, rtime, tmp;

	memset(&mask, 0xFF, sizeof(mask));
	mask &= ~(BLKSIZE - 1);
	//mask = 0xF..FFFE00 lowest 9 bits eq. zero.
	
	while(*ptr){
		blk_id *= 10;
		blk_id += *ptr - '0';
		ptr++;
	}
	
//	printf("%lld %lld %llx %llx -- ", blk_id, blk_id & mask, blk_id & mask, mask);
	blk_id &= mask;//start blk id.
	blk_num = size / BLKSIZE;//blk num.
//	printf("%d %d\n", blk_num, size);
	blks_sum += blk_num;

//	printf("insert %d blks.\n", blk_num);
	while(blk_num){
		//printf("%llu\n", blk_id);
		rcd_ptr = blk_db->ins(blk_id);
		if(!is_new_ins((void*)rcd_ptr->value)){ blks_sum_before++; }
		if(rw == 'R'){
			if(is_new_ins((void*)rcd_ptr->value)){
			//is new.
				*(int *)get_field(rcd_ptr->value, "avg_rsize") = size;
				*(int *)get_field(rcd_ptr->value, "std_rsize") = 0;
				*(int *)get_field(rcd_ptr->value, "max_rsize") = size;
				*(int *)get_field(rcd_ptr->value, "min_rsize") = size;
			}else{
			//not new.
				*(int *)get_field(rcd_ptr->value, "max_rsize") = size > *(int *)get_field(rcd_ptr->value, "max_rsize") ? size : *(int *)get_field(rcd_ptr->value, "max_rsize");
				*(int *)get_field(rcd_ptr->value, "min_rsize") = ((*(int *)get_field(rcd_ptr->value, "min_rsize")) && size > *(int *)get_field(rcd_ptr->value, "min_rsize")) ?
					*(int *)get_field(rcd_ptr->value, "min_rsize") : size;

				rtime = *(int *)get_field(rcd_ptr->value, "r_time");
				tmp_sum_size = (*(int *)get_field(rcd_ptr->value, "avg_rsize")) * rtime;
				tmp_sum_size += size;

				tmp = *(int *)get_field(rcd_ptr->value, "avg_rsize") = tmp_sum_size / (rtime + 1);
				*(int *)get_field(rcd_ptr->value, "std_rsize") += (size - tmp) * (size - tmp);
			}
		}

		
		tmp_ptr = (ULL*)get_field(rcd_ptr->value, rw == 'R' ? "r_time" : "w_time");//w/r_time	
		(*tmp_ptr)++;

		blk_id++;
		blk_num--;
	}
	return 0;
}

int msr_getIO(FILE* fp)//a line from file to specified field
{
	LARGENUM ofs;
	int size = 0, i;
	char rw,c;

	i = 0;
	init_LN(&ofs);
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
				insert_LN(&ofs, c);
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
	//printf("%s %d %c\n", strval_LN(&ofs), size, rw);
	msr_toIOblks(&ofs, size, rw);
	return 0;
}


int main(int argc, char* argv[])
{
	FILE* fp;
	char dp_f_name[64]; 
	int i = 0;
	if(argc != 2){
		help();
		return -1;
	}
	if((fp = fopen(argv[1], "r")) == NULL){
		perror(argv[1]);
		return -1;
	}
	while(msr_getIO(fp) == 0 /*&& i < 5*/){
		i++;
	}
	printf("---Number of IOs: %d, SUM blks %llu ,blks Repeat %llu---\n", i, blks_sum, blks_sum_before);
	
	strcpy(dp_f_name, argv[1]);
	strcat(dp_f_name, ".db");

	blk_db->dump(dp_f_name);
	blk_db->nodes_number();
//	printf("%s:",argv[1]);
//	blk_db->travel_rbt_pr();
	
	delete blk_db;
	fclose(fp);
	return 0;
}