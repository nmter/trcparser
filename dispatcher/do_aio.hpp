#ifndef DO_AIO_HPP
#define DO_AIO_HPP
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <aio.h>

#include "my_RMeter.h"
/*
 * deliver IO from trace, using libaio.
 */
#define ULL unsigned long long
#define LL long long

typedef struct _msr_io_desc{
	ULL ftime;	//time, file time in 100-nano-secs.
	ULL st_ofs;	//ofs in bytes.
	int size;	//size in bytes.
	char rw;	//R/W
}_msr_io_desc;

typedef struct _aio_desc{
	int fd;//
	size_t nbytes;
	off_t aio_ofs;
}_aio_desc;

double avg_response_time = 0;
int avg_response_time_i = 0;
int end_flag = 0;
ULL io_num = 0;

double check_time(global_args *g)
{
	double escape_time, escape_time2 = -1;
	int next_io_idx = -1;
	for(int i = 0; i < g->io_num; i++){
		escape_time = g->aio_data_l[i].io_complete_time - g->aio_data_l[i].io_issue_time;
		for(int j = i + 1; g->aio_data_l[j].io_idx != 0 && g->aio_data_l[j].io_idx == g->aio_data_l[i].io_idx; j++){
			escape_time2 = g->aio_data_l[j].io_complete_time - g->aio_data_l[j].io_issue_time;
			escape_time = escape_time > escape_time2 ? escape_time : escape_time2 ;
		}
		if(escape_time > 0){
			avg_response_time_i++;
			avg_response_time = avg_response_time + (escape_time - avg_response_time) / avg_response_time_i;	
		}
	}
	return avg_response_time;
}

void aio_complete_note( int signo, siginfo_t *info, void *context )
{
	struct aio_data *ptr;//
	int ret;
	if(info->si_signo == SIGIO){
		ptr = (struct aio_data *)info->si_value.sival_ptr;
	}
	if(aio_error64(ptr->aio_cb_ptr) == 0){
		ret = aio_return64(ptr->aio_cb_ptr);
		ptr->io_complete_time = get_time();//update time
/*
#ifdef DBG
		printf("io complete - %lf\n", ptr->io_complete_time - ptr->io_issue_time);
#endif
*/		if(io_num - 1 == ptr->aio_idx){
			end_flag = 1;
		}
	}
}

void initialize_sig(struct sigaction *o_sig_act)
{
	struct sigaction sig_act;
	sigemptyset(&sig_act.sa_mask);
	sig_act.sa_flags = SA_SIGINFO;
 	sig_act.sa_sigaction = aio_complete_note;
	sigaction( SIGIO, &sig_act, o_sig_act );//
}

void release_sig(struct sigaction *o_sig_act)
{
	sigaction( SIGIO, o_sig_act, NULL );
}


void initialize_aio(_aio_desc *d, void * buf, struct aio_data* data, struct aiocb64 *aio_cb0)
{
	
	bzero((char *)aio_cb0,sizeof(struct aiocb64));
	
	aio_cb0->aio_buf = buf;
	aio_cb0->aio_nbytes = d->nbytes;
	aio_cb0->aio_offset = d->aio_ofs;
	aio_cb0->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	aio_cb0->aio_sigevent.sigev_signo = SIGIO;
	aio_cb0->aio_sigevent.sigev_value.sival_ptr = (void *)data;//
	aio_cb0->aio_fildes = d->fd;
	//issue later.
}
#include "naive_db.hpp"
//TODO - add map_table
naive_db *blk_db_from_file = new naive_db_rbt();
off_t new_ofs_0 = 0;
off_t new_ofs_00 = 0;
off_t new_ofs_01 = 0;

LL fsize_nbytes, fsize_nbytes0, fsize_nbytes1;

void _update_ofss_remain(ULL blk_id, int size, ULL ofs)
{
	int n_blks = (size >> 9) - 1;// */512
	while(n_blks){
		((struct value_t*)blk_db_from_file->search(blk_id)->value)->ofs = ofs;
		blk_id++;
		ofs += 512;
		n_blks--;
	}
}


int issue_io(_msr_io_desc *msr_io, global_args *G_args,int io_idx, int fd){
	struct aio_data *this_data_ptr = G_args->aio_data_l + io_idx;
	struct aiocb64 *this_aiocb_ptr = G_args->aio_cb_l + io_idx;
	class record *rptr;
	ULL mask;
	ULL blk_id;
	_aio_desc desc = {0, 0, 0};
	
	desc.fd = fd;
	desc.nbytes = (size_t) msr_io->size;//int
	
	memset(&mask, 0xFF, sizeof(mask));
	mask &= ~(512 - 1);
	
	blk_id = msr_io->st_ofs & (mask);
	blk_id = blk_id >> 9;
	
	rptr = blk_db_from_file->search(blk_id);
	if(((struct value_t*)rptr->value)->ofs == 0){
		//printf()
		desc.aio_ofs = ((struct value_t*)rptr->value)->ofs = new_ofs_0;
		new_ofs_0 += msr_io->size;
		if(new_ofs_0 % 4096 != 0){//4096 aligned I/O
			new_ofs_0 += (4096 - new_ofs_0 % 4096);
		}
		new_ofs_0 = new_ofs_0 % fsize_nbytes;//avoid offset overflow
		
		_update_ofss_remain(blk_id + 1, msr_io->size, ((struct value_t*)rptr->value)->ofs);
	}else{
		desc.aio_ofs = ((struct value_t*)rptr->value)->ofs;
	}
	
	initialize_aio(&desc, G_args->aio_buf, this_data_ptr, this_aiocb_ptr);
	
	this_data_ptr->io_issue_time = get_time();
	this_data_ptr->aio_cb_ptr = this_aiocb_ptr;
	this_data_ptr->aio_idx = io_idx;
/*
#ifdef DBG
	printf("%lld %d %c %lf\n", 
		(LL)desc.aio_ofs, (int)desc.nbytes, msr_io->rw, this_data_ptr->io_issue_time);
	
//	return ;
#endif
*/
	
	if(msr_io->rw == 'R')
		aio_read64(this_aiocb_ptr);
	else
		aio_write64(this_aiocb_ptr);
	return 1;
}



int issue_io_MultiCS(_msr_io_desc *msr_io, global_args *G_args,int io_idx, int fd0, int fd1){//possibly multi blocks 
	struct aio_data *this_data_ptr = G_args->aio_data_l + io_idx, *this_data_ptr_l, *tmp2;
	struct aiocb64 *this_aiocb_ptr = G_args->aio_cb_l + io_idx, *this_aiocb_ptr_l, *tmp;
	
	_aio_desc desc = {0, 0, 0};
	ULL mask;
	ULL blk_id, l_idx;
	
	int n_blks = msr_io->size >> 9;//number of blocks
	int blk_type, last_blk_type;
	int n_aios = 1;
	double dev;
	class record *rptr;
	
	memset(&mask, 0xFF, sizeof(mask));
	mask &= ~(512 - 1);
	blk_id = msr_io->st_ofs & (mask);
	blk_id = blk_id >> 9;
	
	l_idx = blk_id;
	last_blk_type = -1;
	this_data_ptr_l = this_data_ptr;
	this_aiocb_ptr_l = this_aiocb_ptr;
	while(n_blks){
		rptr = blk_db_from_file->search(blk_id);
		dev = ((struct value_t*)rptr->value)->rt == 0 ? 
			10.0 : ((double)((struct value_t*)rptr->value)->wt / (double)((struct value_t*)rptr->value)->rt);
		if(dev > 9.0){//w intensity
			blk_type = 0;
		}else if(dev < 0.11){//r intensity
			blk_type = 1;
		}else{
			blk_type = 2;
		}
		if(last_blk_type > 0 && last_blk_type != blk_type){
				//diff from last blk.
				//offset - l_idx << 9, size -  blk_id - l_idx
			desc.fd = last_blk_type == 1 ? fd1 : fd0;//
			desc.nbytes = (blk_id - l_idx)<<9;
			
			rptr = blk_db_from_file->search(l_idx);
			
			if(((struct value_t*)rptr->value)->ofs == 0){
				if(desc.fd == fd1){
					desc.aio_ofs = ((struct value_t*)rptr->value)->ofs = new_ofs_01;
					new_ofs_01 += desc.nbytes;
					new_ofs_01 += new_ofs_01 % 4096 ?(4096 - new_ofs_01 % 4096) : 0;
					_update_ofss_remain(l_idx + 1, desc.nbytes, ((struct value_t*)rptr->value)->ofs);
				}else{
					desc.aio_ofs = ((struct value_t*)rptr->value)->ofs = new_ofs_00;
					new_ofs_00 += desc.nbytes;
					new_ofs_00 += new_ofs_00 % 4096 ?(4096 - new_ofs_00 % 4096) : 0;
					_update_ofss_remain(l_idx + 1, desc.nbytes, ((struct value_t*)rptr->value)->ofs);
				}
			}else{
				desc.aio_ofs = ((struct value_t*)rptr->value)->ofs;
			}
			
			initialize_aio(&desc, G_args->aio_buf, this_data_ptr, this_aiocb_ptr);
			
			this_data_ptr->aio_cb_ptr = this_aiocb_ptr;
			this_data_ptr->aio_idx = io_idx + n_aios - 1;
			this_data_ptr->io_idx = io_idx;
			
			this_data_ptr++;
			this_aiocb_ptr++;
			l_idx = blk_id;
			n_aios++;
		}
		
		last_blk_type = blk_type;
		blk_id++;
		n_blks--;
	}
	
	
	desc.fd = last_blk_type == 1 ? fd1 : fd0;//
	desc.nbytes = (blk_id - l_idx)<<9;
			
	rptr = blk_db_from_file->search(l_idx);
			
	if(((struct value_t*)rptr->value)->ofs == 0){
		if(desc.fd == fd1){
			desc.aio_ofs = ((struct value_t*)rptr->value)->ofs = new_ofs_01;
			new_ofs_01 += desc.nbytes;
			new_ofs_01 += new_ofs_01 % 4096 ?(4096 - new_ofs_01 % 4096) : 0;
		}else{
			desc.aio_ofs = ((struct value_t*)rptr->value)->ofs = new_ofs_00;
			new_ofs_00 += desc.nbytes;
			new_ofs_00 += new_ofs_00 % 4096 ?(4096 - new_ofs_00 % 4096) : 0;
		}
	}else{
		desc.aio_ofs = ((struct value_t*)rptr->value)->ofs;
	}
			
	initialize_aio(&desc, G_args->aio_buf, this_data_ptr, this_aiocb_ptr);
	this_data_ptr->aio_cb_ptr = this_aiocb_ptr;
	this_data_ptr->aio_idx = io_idx + n_aios - 1;
	this_data_ptr->io_idx = io_idx;
		
	this_data_ptr++;
	this_aiocb_ptr++;
	
	//issue
	
		if(msr_io->rw == 'R'){
			for(tmp = this_aiocb_ptr_l, tmp2 = this_data_ptr_l; tmp < this_aiocb_ptr; tmp++, tmp2++){
				tmp2->io_issue_time = get_time();
				aio_read64(tmp);
			}
		}else{
			for(tmp = this_aiocb_ptr_l, tmp2 = this_data_ptr_l; tmp < this_aiocb_ptr; tmp++, tmp2++){
				tmp2->io_issue_time = get_time();
				aio_write64(tmp);
			}		
		}
	
	return n_aios;
}

#define TRACE_RECIEVED 0
#define TRACE_END 1

int get_msr_io(FILE *fp, _msr_io_desc *msr_io)
{
	char c;
	int i = 0;
	msr_io->st_ofs = msr_io->ftime = 0;//
	msr_io->size = 0;
	while((c = fgetc(fp)) != '\n' && !feof(fp)){
		if(c == ','){
			i++;continue;
		}
		switch(i){
			case 0:
				msr_io->ftime *= 10;
				msr_io->ftime += c - '0';break;
			case 3:
				if(c == 'R' || c == 'W')
					msr_io->rw = c;break;
			case 4:
				msr_io->st_ofs *= 10;
				msr_io->st_ofs += c - '0';break;
			case 5:
				msr_io->size *= 10;
				msr_io->size += c - '0';break;
		}
	}
	if(feof(fp)){
		return TRACE_END;
	}
	return TRACE_RECIEVED;
}

FILE* initialize_msr_parse_trace_file(const char* trace_file_name)
{
	FILE *fp;
	if((fp = fopen(trace_file_name, "r")) == NULL){
		perror(trace_file_name);
		exit(EXIT_FAILURE);
	}
	return fp;
}
void release_msr_trace_file(FILE *fp)
{
	fclose(fp);
}

void initialize_devs(int *fd, int *fd0, int *fd1, global_args *G_args)
{
	if(G_args->MultiCS){
		*fd0 = open(G_args->dev_names[0], O_RDWR|O_LARGEFILE);//smaller chunk - w intensity
		*fd1 = open(G_args->dev_names[1], O_RDWR|O_LARGEFILE);
		if(-1 == *fd0){
			perror(G_args->dev_names[0]);goto ERROR;
		}
		if(-1 == *fd1){
			perror(G_args->dev_names[1]);goto ERROR;
		}
	}else{
		*fd = open(G_args->dev_name, O_RDWR|O_LARGEFILE);		
		if(-1 == *fd){
			perror(G_args->dev_name);goto ERROR;
		}
	}
	return;
ERROR:
	exit(EXIT_FAILURE);		
}
void release_devs(int fd, int fd0, int fd1, global_args *G_args)
{
	if(G_args->MultiCS){
		close(fd0);close(fd1);
	}else{
		close(fd);
	}
}

LL sizeofdev(int fd, const char *str)
{
	LL file_size_in_bytes;
	//port size of block device
	if(-1 == ioctl(fd, BLKGETSIZE64, &file_size_in_bytes)){
		close(fd);
		perror(str);
		exit(EXIT_FAILURE);		
	}
	return file_size_in_bytes;
}

/*
 * TODO - Rate : how to speed up the replay ?
		- offset larger than the max_size ?
		- MultiCS
 */

#define Min(x) (60*(x))
#define MIN2 Min(2)
#define MIN10 Min(10)
#define MIN60 Min(60)
#define NOLIMIT -1
#define MAX_IO_TIME Min(20)
 
void perform_ios(global_args *G_args)
{
	int fd;
	int fd0,fd1;
	int n_aios;
	char db_name[255];
	struct sigaction o_act;
	
	FILE *fp;

	_msr_io_desc msr_io;
	ULL msr_start_time;
	double time, msr_escape_time;
	
	initialize_devs(&fd, &fd0, &fd1, G_args);
	
	if(G_args->MultiCS){
		fsize_nbytes0 = sizeofdev(fd0, G_args->dev_names[0]);//smaller chunk
		fsize_nbytes1 = sizeofdev(fd1, G_args->dev_names[1]);//bigger chunk
	}else{
		fsize_nbytes = sizeofdev(fd, G_args->dev_name);
	}
	//
	initialize_sig(&o_act);
	
	//parse trace file. && perform io.
	fp = initialize_msr_parse_trace_file(G_args->trace_file_name);
	
	strcpy(db_name, G_args->trace_file_name);
	strcat(db_name, ".db");
	//
#ifdef DBG
	printf("rebuild from DB: %s\n",db_name);
#endif
	blk_db_from_file->rebuild_from(db_name);

#ifdef DBG
	printf("start parse trace.\n");
#endif
	
	G_args->start_time = get_time();
	
	while(get_msr_io(fp, &msr_io) == TRACE_RECIEVED && io_num < MAX_TRC_COUNT){
		//a msr IO ==> a aio or more.
		if(get_time() - G_args->start_time > (double)G_args->mins * 60)
			break;
		
		if(io_num == 0){
			msr_start_time = msr_io.ftime;
		}
		
		msr_escape_time = (msr_io.ftime - msr_start_time) / (double)10000000 / (double)G_args->rate;//take rate into consider.
		while(get_time() - G_args->start_time < msr_escape_time){
			//just waiting & asking time. other way to do this ?
		};
		if(G_args->MultiCS){
			n_aios = issue_io_MultiCS(&msr_io, G_args, io_num, fd0, fd1);
		}else{
			n_aios = issue_io(&msr_io, G_args, io_num, fd);
		}
		io_num += n_aios;
	}
	G_args->io_num = io_num;
	
	while(end_flag != 1){//wait for all aio complete.
		sleep(1);
	}
	
#ifdef DBG
	
	printf("perform io end.\n");
#endif
	
	delete blk_db_from_file;
	release_sig(&o_act);
	release_devs(fd,fd0,fd1, G_args);
	release_msr_trace_file(fp);
	return;
}
#endif