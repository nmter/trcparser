#ifndef _TRC_PARSER_H
#define _TRC_PARSER_H
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
typedef unsigned int UINT32;
#define PAGE_IS_FIRST	0x00000001
#define PAGE_IS_ONLY	0x00000010
#define ST_TIME_SET		0x00000100
#define VPGNUM trc_parser.stat->v_npg
#define RPGNUM trc_parser.stat->r_npg
#define B_ISSUM trc_parser.stat->nissue_before
#define B_WRSUM trc_parser.stat->nwr_before
#define A_ISSUM trc_parser.stat->nissue_after
#define A_WRSUM trc_parser.stat->nwr_after
#define STIME trc_parser.stat->st_time
#define EDTIME trc_parser.stat->ed_time
#define PERFORM_DEDUP	0
#define PERFORM_ORG		1



typedef struct _output_stat_t{
	int v_npg, r_npg;		//redundency = r_nblk / v_nblk(only for write io)
	float st_time, ed_time;

	long nissue_before, nwr_before;
	long nissue_after, nwr_after;
} output_stat_t;

typedef struct _md5list_t{
	UINT32 *md5;
	struct _md5list_t *next;
}md5list_t;

typedef struct _iotrace_t{
	float iss_time;		
	short rw;				//r - 0;w - 1
	long st_blk;			//start block number
	int bcount;
	md5list_t *md5list;
	int flag;//
}io_trace_t;

/*
 * what trc_parser do:
 *				1. filter for the NULL line in origin trace file
 *				2. convert the hex md5 list from char_string to int_array
 *				3. find out how mang redundancy in all trace's md5-value
 */

typedef struct _trc_parser_t{
	char *trc_file_name;
	FILE *fp;				//init to NULL
	FILE *ofp[2];	/*	
					 *  2 ouput fp:
					 *	output after-dedup trace for disksim,
					 *	output origin trace for disksim
					 */
	output_stat_t *stat;

	void (*io_trace_generate)();

}trc_parser_t;

typedef struct _md5base_t{
	 
	int (* in_md5base)(UINT32 *md5);
}_md5base_t;	//here is a "Violence-searching"
				// remain to change to "RBtree" or "other search data structure"

#endif

