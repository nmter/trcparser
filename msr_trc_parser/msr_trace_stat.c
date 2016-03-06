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
	printf("run: ./trcstatMSR trace_file_name\n");
}


#define BLKSIZE0 512 //blk size in bytes
#define BLKSIZE1 4096 //4K 

#define BLKSIZE BLKSIZE0
unsigned long long blks_sum = 0;

int msr_toIOblk(LARGENUM *ofs, int size, char rw)
{
	unsigned long long blk_id = 0, mask;

	int blk_num;
	char* ptr = ofs->buf;
	
	
	memset(&mask, 0xFF, sizeof(mask));
	mask &= ~(BLKSIZE - 1);
	//mask = 0xF..FFFE00 lowest 9 bits eq. zero.
	
	while(*ptr){
		blk_id *= 10;
		blk_id += *ptr - '0';
		ptr++;
	}
	
//	printf("%lld %lld %llx %llx -- ", blk_id, blk_id & mask, blk_id & mask, mask);
	blk_id &= mask;
	blk_num = size / BLKSIZE;
//	printf("%d %d\n", blk_num, size);
	blks_sum += blk_num;
	
	
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
	msr_toIOblk(&ofs, size, rw);
	return 0;
}


int main(int argc, char* argv[])
{
	FILE* fp;
	int i = 0;
	if(argc != 2){
		help();
	}
	if((fp = fopen(argv[1], "r")) == NULL){
		perror(argv[1]);
		return -1;
	}
	while(msr_getIO(fp) == 0 /*&& i < 5*/){
		i++;
	}
	printf("---Number of IOs: %d, SUM blks %lld---\n", i, blks_sum);
	return 0;
}