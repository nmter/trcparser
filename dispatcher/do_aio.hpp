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

void issue_io(_msr_io_desc *msr_io, global_args *G_args,int io_idx, int fd){
	struct aio_data *this_data_ptr = G_args->aio_data_l + io_idx;
	struct aiocb64 *this_aiocb_ptr = G_args->aio_cb_l + io_idx;
	_aio_desc desc = {0, 0, 0};
	
	desc.fd = fd;
	desc.nbytes = msr_io->size;
	desc.aio_ofs = (off_t) msr_io->st_ofs;
	
	initialize_aio(&desc, G_args->aio_buf, this_data_ptr, this_aiocb_ptr);
	
	this_data_ptr->io_issue_time = get_time();
	this_data_ptr->aio_cb_ptr = this_aiocb_ptr;
	
	if(msr_io->rw == 'R')
		aio_read64(this_aiocb_ptr);
	else
		aio_write64(this_aiocb_ptr);
}
void issue_io_MultiCS(_msr_io_desc *msr_io, global_args *G_args,int io_idx, int fd0, int fd1){
	
	
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
		*fd0 = open(G_args->dev_names[0], O_RDWR|O_LARGEFILE);
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
 */
void perform_ios(global_args *G_args)
{
	int fd;
	int fd0,fd1;
	
	LL fsize_nbytes, fsize_nbytes0, fsize_nbytes1;
	
	struct sigaction o_act;
	
	FILE *fp;
	ULL io_num = 0;
	_msr_io_desc msr_io;
	ULL msr_start_time;
	double time, msr_escape_time;
	
	initialize_devs(&fd, &fd0, &fd1, G_args);

	if(G_args->MultiCS){
		fsize_nbytes0 = sizeofdev(fd0, G_args->dev_names[0]);
		fsize_nbytes1 = sizeofdev(fd1, G_args->dev_names[1]);
	}else{
		fsize_nbytes = sizeofdev(fd, G_args->dev_name);
	}
	//
	initialize_sig(&o_act);
	
	//parse trace file. && perform io.
	fp = initialize_msr_parse_trace_file(G_args->trace_file_name);
	
	while(get_msr_io(fp, &msr_io) == TRACE_RECIEVED && io_num < MAX_TRC_COUNT){
		//a msr IO ==> a aio or more.
		if(io_num == 0){
			msr_start_time = msr_io.ftime;
		}
		
		msr_escape_time = (msr_io.ftime - msr_start_time) / (double)10000000;
		while(get_time() - G_args->start_time < msr_escape_time){
			//just waiting & asking time. other way to do this ?
		};
		if(G_args->MultiCS){
			//issue_io_MultiCS(msr_io, G_args, io_num, fd);
		}else{
			printf("1\n");
			issue_io(&msr_io, G_args, io_num, fd);
		}
		io_num++;
	}
	
	
	release_sig(&o_act);
	release_devs(fd,fd0,fd1, G_args);
	release_msr_trace_file(fp);
	return;
}
#endif