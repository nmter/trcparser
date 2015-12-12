/*
 * IO密集度，IO局部性，IO大小特点.
 * 	
 *	A1.×
 *	hash_bucket_count.
 *		input 	- a pile of lines. each line contain only one number{double/int}.
		output	- a structure below.
 *		[key range 1]->{}->{}->{}
		[key range 2]->{}->{}->{}->...->{}
		...
		[key range n]->{}->{}
		
		
 *	A2.√
 *	generate graph of <x:time, y:ufactor>
 *
 *
 *	    ↑y:ufactor		
 *		||	...   .
 *		||.~   . ~ 
 *		||------------>x:time
 *
 *
 *	ufactor is :
	 *	LRU cache line hit rate.
	 		input	- trace
	 		output	- lines of <time,hit>.
	 	
	 *	IOPS
	 *	AvgSize
 *
 */
 
 
 
 
 
 
