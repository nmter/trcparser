#include <stdio.h>
//./execute filename 
#define ULL unsigned long long
ULL start_time = 0, end_time = 0;//in 100-nano_second(windows filetime)
double escape_time;//in seconds.
ULL io_num = 0;
double avg_read_size = 0, avg_write_size = 0;
ULL read_num = 0, write_num = 0;

void help()
{
	printf("./execute trace_filename\n");
}

void update_avg_size(int isize, char rw)//isize - in bytes
{
	double *ptr = rw == 'R' ? &avg_read_size : &avg_write_size;
	read_num += rw == 'R' ? 1 : 0;
	write_num += rw == 'R' ? 0 : 1;
	*ptr = *ptr + ((double)isize - *ptr)/(rw == 'R'? read_num : write_num);
}

int msr_parseIO(FILE *fp)//get <filetime, rw, size>
{
	char c, rw;
	int i = 0;
	int size = 0;
	ULL time = 0;
	while((c = fgetc(fp)) != '\n' && !feof(fp)){
		if(c == ','){
			i++;continue;
		}
		switch(i){
			case 0:
				time *= 10;
				time += c - '0';
				break;
			case 3:
				if(c == 'R'){
					rw = c;
				};
				if(c == 'W'){
					rw = c;
				};
				break;
			case 5://size
				size *= 10;
				size += c -'0';
				break;
		}
	}
	if(feof(fp)){
		return 1;
	}
	
	end_time = time;
	if(io_num == 0){
		start_time = time;
	}
	update_avg_size(size, rw);
	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	if(argc != 2){
		help();
		return -1;
	}
	
	if((fp = fopen(argv[1], "r")) == NULL){//trace_file
		perror(argv[1]);
		return -1;
	}
	while(msr_parseIO(fp) == 0){
		io_num++;
	}
	escape_time = (end_time - start_time)/(double)10000000;
	
//1
//	printf("IOPS - %0.2lf, IO_NUM - %llu, Read - %llu, Write - %llu, AVG_Rsize - %0.2lf, AVG_Wsize - %0.2lf\n",
//		io_num / escape_time, io_num, read_num, write_num, avg_read_size, avg_write_size);
//0
	printf("%s:",argv[1]);
	printf("%0.2lf:%llu:%llu:%llu:%0.2lf:%0.2lf\n",
		io_num / escape_time, io_num, read_num, write_num, avg_read_size/1024, avg_write_size/1024);//in KB
	fclose(fp);	
	return 0;
}