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


void perform_ios(const char *dev)
{
	int fd = open(dev, O_RDWR|O_LARGEFILE);
	struct stat fst_buf;
	LL file_size_in_bytes;
	LL file_size_in_sectors;//512 Bytes
	if(-1 == fd){
		goto ERROR;
	}
	if(-1 == fstat(fd, &fst_buf)){
		close(fd);goto ERROR;
	}
	
	//port size of block device
	if(-1 == ioctl(fd, BLKGETSIZE64, &file_size_in_bytes)){
		close(fd);goto ERROR;
	}
	file_size_in_sectors = file_size_in_bytes >> 9;
/*
	if(-1 == ioctl(fd, BLKGETSIZE, &file_size_in_sectors)){
		close(fd);goto ERROR;
	}
*/
	
	//modify @fst_buf's size field. because fstat() will return a block_device's size as 0.
	*((LL *)(&fst_buf.st_size)) = file_size_in_bytes;
	*((LL *)(&fst_buf.st_blocks)) = file_size_in_sectors;
	
	//to see something in fd's stat buffer.
	printf("%s %lld %c %lld\n", 
		dev,
		(long long)fst_buf.st_size, //0
		S_ISBLK(fst_buf.st_mode) ? 'y' : 'n', 
		(long long)fst_buf.st_blocks
		);
	//stat end.
	
	

	
	close(fd);
	return;
	
ERROR:
	perror(dev);
	exit(EXIT_FAILURE);		
}
#endif