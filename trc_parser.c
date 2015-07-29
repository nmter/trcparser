#include "trc_parser.h"
#include "rb_tree.h"


extern rb_tree_t rbtree;

void io_trace_generate();

output_stat_t output_stat = {
	0,0,
	-1,-1,
	0,0,
	0,0
};

trc_parser_t trc_parser = {
	NULL,
	NULL,
	NULL, NULL,
	&output_stat,
	&io_trace_generate
};


io_trace_t *this_x_io;//this global-var used in in_tree_callback, initialized in  _io_trace_parser
int perform_flag;




int _hexval(char x)
{
	if(x >= '0' && x <= '9'){
		return x - '0';
	}else if(x >= 'a' && x <= 'f'){
		return x - 'a' + 10;
	}else if(x >= 'A' && x <= 'F'){
		return x - 'A' + 10;
	}else
		return -1;

}
void _streverse(char *dest, char *src)
{
	int i,len = strlen(src);
	for(i = len - 1; i >= 0; i--){
		*dest = src[i];
		dest++;
	}
	*dest = '\0';
}

/*
 *	"1234567890abcdef1234567890abcdef" --> unsigned int x[4];
 *
 *
 */
UINT32 *_convert_32hexStr_to_IntA(char *str)
{
	assert(sizeof(UINT32) == 4);
	assert(strlen(str) == 32);
	char t_str[33];
	_streverse(t_str, str);

	UINT32* temp = malloc(sizeof(UINT32) * 4);//16b
	int i,j,idx = 0,tval,x;
	for(i = 0; i < 4; i++){
		tval = 0;
		for(j = 0; j < 8; j++){
			if((x = _hexval(*(t_str + (idx++)))) == -1){
				printf("error hex val!\n");
				exit(-1);
			}
			x = x << (j*4);
			//printf("x:%x\n",x);
			tval += x; 
		}
		temp[i] = tval;
	}
	/*
	printf("--------\n");
	for(i = 0; i < 4; i++){
		printf("%u\n",temp[i]);
	}
	*/
	return temp;//should be free!
}


char* _trace_generate()
{
	char *tmp_line = malloc(4096*sizeof(char));
	
	memset(tmp_line, 0, 4096*sizeof(char));
	if(!trc_parser.fp){
		if(NULL == (trc_parser.fp = fopen(trc_parser.trc_file_name, "r"))){
			perror("file open error!");
			exit(-1);
		}
	}
	if(!feof(trc_parser.fp)){
		fgets(tmp_line, 4096, trc_parser.fp);

		if(strlen(tmp_line) < 10){
			free(tmp_line);
			return NULL;
		}
		else
			return tmp_line;//should be free!
	}else{
		strcpy(tmp_line, "EOF");
		return tmp_line;
	}
}

char* _trimhead(char* str)
{
	int i,len = strlen(str);
	for(i = 0; i < len; i++){
		if(str[i] != 32){
			return i + str;
		}
	}
}

md5list_t* _md5l_alloc_new(char *str, int num)
{
	int i,idx = 0;
	md5list_t *head = NULL, *before = NULL, *tmp;
	for(i = 0; i < num; i++){
		idx += 33;
		*(str + idx - 1) = '\0';
	}
	idx = 0;
	while(num){
		tmp = malloc(sizeof(md5list_t));
		memset(tmp, 0, sizeof(md5list_t));
		tmp->md5 = _convert_32hexStr_to_IntA(str + idx);
		if(before){
			before->next = tmp;
		}
		
		if(!head){
			head = tmp;
		}
		before = tmp;
		idx += 33;
		num--;
	}
	
	return head;
}

void _perform_io_init()
{
	char *suffix[2] = {".dedup", ".org"};
	char name[2][64];
	int i = 0;
	FILE *fp;
	while(i < 2){
		strcpy(name[i], trc_parser.trc_file_name);
		strcat(name[i], suffix[i]);
		fp = fopen(name[i],"w+");
		if(!fp){
			perror("output file open error!");
			exit(-1);
		}
		trc_parser.ofp[i] = fp;
		i++;
	}
}
	

void _perform_io_to_file(io_trace_t *x, int which)
{
	FILE *fp;
	fp = trc_parser.ofp[which];
	if((which == PERFORM_DEDUP)&&(x->flag & PAGE_IS_ONLY)){
			return;//do nothing
	}
	/*arr-time(ms) dev_num sec-addr size flag*/
	fprintf(fp, "%f 0 %ld %d %d\n",x->iss_time * 1000,
		x->st_blk, x->bcount, x->rw);//output the trace in the format that [disksim] can read
}

void in_tree_callback()
{
	/*this io have to */
	if(this_x_io->flag & PAGE_IS_ONLY){
		return;
	}
	if(this_x_io->flag & PAGE_IS_FIRST){
		this_x_io->st_blk += 8;
	}

	this_x_io->bcount -= 8;
}

void _md5l_push_into_rbt(md5list_t *head, io_trace_t *x)
{
	int pg_num = 0, vpg_num = 0;
	while(head){
		x->flag = 0;
		if(pg_num == 0){
			x->flag |= PAGE_IS_FIRST;
		}
		if(x->bcount == 8){
			x->flag |= PAGE_IS_ONLY;
		}
		if(0 == rbtree.rb_insert(rbtree.rb_alloc_new(head->md5))){//0 - insert successful
			pg_num++;
			x->flag = 0;
		}
		head = head->next;
		vpg_num++;
	}
	/*collect statistics*/
	trc_parser.stat->v_npg += vpg_num; //
	trc_parser.stat->r_npg += pg_num;
}


void _io_trace_parser(char* line)
{
	io_trace_t x_io;
	x_io.flag = 0;
	int i, magic_idx[6] = {16,18,31,42,53,69};//for every line, the idx of '\t' are same
	for(i = 0; i < 6; i++){
		line[magic_idx[i]] = '\0';
	}
	/*iss_time*/
	x_io.iss_time = atof(_trimhead(line));
	//printf("%s %f\n",trimhead(line) + line, x_io.iss_time);

	/*
	 * time stastictis
	 */
	if(trc_parser.stat->st_time < 0){
		trc_parser.stat->st_time = x_io.iss_time;
	}
	trc_parser.stat->ed_time = x_io.iss_time;
	
	/*rw*/
	x_io.rw = 0 == strcmp(_trimhead(line + magic_idx[0] + 1), "W")?
		1:0;
	//printf("%d\n",x_io.rw);

	/*st_blk*/
	x_io.st_blk = atol(_trimhead(line + magic_idx[1] + 1));
	//printf("%ld\n", x_io.st_blk);
	
	/*bcount*/
	x_io.bcount = atoi(_trimhead(line + magic_idx[2] + 1));
	//printf("%d\n", x_io.bcount);

	/*md5list*/
	x_io.md5list = _md5l_alloc_new(line + magic_idx[5] + 1, x_io.bcount / 8);
	//printf("%s\n",line + magic_idx[5] + 1);

	this_x_io = &x_io;//only use in call back

	_perform_io_to_file(&x_io, PERFORM_ORG);

	if(x_io.rw){//if is write io
		_md5l_push_into_rbt(x_io.md5list, &x_io);	
	}
	
	/*
	 *	io after dedup is output. 
	 */
	_perform_io_to_file(&x_io, PERFORM_DEDUP);

	/*
	 *	some statistics collection
	 *	virtual page num collected in _md5l_push...()
	 *
	 */
	trc_parser.stat->nissue_before++;
	trc_parser.stat->nissue_after++;
	if(x_io.rw){
		trc_parser.stat->nwr_before++;
		if(!(x_io.flag & PAGE_IS_ONLY)){
			trc_parser.stat->nwr_after++;		
		}else{
			trc_parser.stat->nissue_after--;
		}
	}

}

void _print_stat()
{
	printf("------------------------\n");
	printf("%d\t%d\t%f\t%f \n", trc_parser.stat->r_npg ,trc_parser.stat->v_npg, 
		(double)trc_parser.stat->r_npg / trc_parser.stat->v_npg,
		1 - (double)trc_parser.stat->r_npg / trc_parser.stat->v_npg);

	printf("Before Dedup\nw-io-tfc(MB):%d\nlast time(h):%f\nio number(-):%ld (w:%ld,%f)\n\
After Dedup\nw-io-tfc(MB):%d\nlast time(s):%f\nio number(-):%ld (w:%ld,%f)\n\n",
		 VPGNUM / 256,
		 (EDTIME - STIME) / 60 / 60,
		 B_ISSUM, B_WRSUM, (float)B_WRSUM / B_ISSUM,
		 RPGNUM / 256,
		 (EDTIME - STIME) / 60 / 60,
		 A_ISSUM, A_WRSUM, (float)A_WRSUM / A_ISSUM);
}

void _closefiles()
{
	int i = 0;
	if(trc_parser.fp){
		fclose(trc_parser.fp);
	}
	while(i < 2){
		if(trc_parser.ofp[i])
			fclose(trc_parser.ofp[i]);
		i++;
	}
}

void io_trace_generate()
{
	char *line;
	_perform_io_init();

	while(1){
		//printf("line: %d\n",i++);
		line = _trace_generate();

		if(NULL == line){
			continue;
		}
		if(0 == strcmp(line, "EOF")){
			break;
		}
		_io_trace_parser(line);

		free(line);
		/**/
	}
	if(line){
		free(line);
	}
	rbtree.rb_destroy();
	_print_stat();
	_closefiles();
}


