import os
import flists
'''
0. trace_file ==> 3 tables
	address		hit
	0 - 1 M  	500
	1 - 2 M		10
	...

	
trace_file
	xxx xxx xxxx ....
	xxx xxx xxxx ....
	.....
	xxx xxx xxxx ....
'''
def do_default():
	'''
	'''
	print "it\'s an error here."

	
def insert_dict_2tuple(xdict, key, value):
	'''
	'''
	xdict[key] = value

def insert_dict(xdict, key):
	'''
	'''
	if(not xdict.has_key(key)):
		xdict[key] = 1
	else:
		xdict[key] += 1

def do_offset_adjust_ofs_band():
	'''this func not complemented.
	'''
	return 0

def display_dict_in_tables_fp(xdict, fp, which):
	'''
	'''
	#print 'xxxxx'
	#print xdict.items() 
	a = sorted(xdict.items(), lambda x,y:cmp(x[0],y[0])) #sorted by {key} in asc order
	'''
	if(which == 'offset'):
		for x in range(a[len(a)-1][0] + 1):
			if(xdict.has_key(x)):
				fp.write(str(x)+' '+str(xdict[x])+'\n')
			else:
				fp.write(str(x)+' '+'0'+'\n')
		return
	'''
	for i in a:
		fp.write(str(i[0])+' '+str(i[1])+'\n')
	#print 'tttttt'

def display_dict_in_tables(xdict):
	'''
	'''
	#print 'xxxxx'
	#print xdict.items() 
	a = sorted(xdict.items(), lambda x,y:cmp(x[0],y[0])) #sorted by {key} in asc order
	for i in a:
		print i[0],",",i[1]
	#print 'tttttt'
	 
	return 
	
def display_dict(xdict, way, fp=0, which=0):
	if(way == 'table'):
		display_dict_in_tables(xdict)
	if(way == 'table_fp'):
		display_dict_in_tables_fp(xdict, fp, which)


def do_offset(xdict_list, words, ofs_band=1):
	'''{ofs_band} MBytes-width.
		default no time-consideration.
		
		@param xdict_list: The feature dict list which is defined outside.
		The elements of xdict_list:
		xdict_list[0]: dict for read
		xdict_list[1]: dict for write
		xdict_list[2]: dict for all
		
		@param words: the line read from trace file.
		
		@param ofs_band=1: the width of offset.
	'''
	
	
	#this req.'s time(in sec.)
	time = words[0]
	
	#this req.'s offset(in {ofs_band}MB.)
	ofs = words[2]
	ofs = int(ofs) / ( ofs_band * 1024 * 2 )
	if(words[1] == 'R'):
		insert_dict(xdict_list[0], ofs)
	else:
		insert_dict(xdict_list[1], ofs)
	insert_dict(xdict_list[2], ofs)

def do_size_Cumulate(xdict_list, words, time_band=5):
	'''
	'''
	return 

def do_size_Avg(xdict_list, words, time_band=60):
	'''almost the same as @def do_IOPS_Avg, if there any way in [PY] can implement template like c++???
		here {size} is in kilo-bytes.
	'''
	time = float(words[0])
	if((flists.rec_start - 0) < 0.0001):
		flists.rec_start = time + 1
	if(time - (flists.rec_start - 1) > time_band):
		flists.rec_start = time + 1
		
		#insert
		insert_dict_2tuple(xdict_list[0], time,flists._ios_r > 0 and (flists.Avg_size_r / flists._ios_r) or 0)#read. [PY]'s " ? : " operator: expr and val1 or val2 in py is equal to expr ? val1 : val2 in c/c++
		insert_dict_2tuple(xdict_list[1], time,flists._ios_w > 0 and (flists.Avg_size_w / flists._ios_w) or 0)#write
		insert_dict_2tuple(xdict_list[2], time,flists._ios_a > 0 and (flists.Avg_size_a / flists._ios_a) or 0)#all
		
		#reset
		flists._ios_r = 0
		flists._ios_w = 0
		flists._ios_a = 0
		
		flists.Avg_size_r = 0.0
		flists.Avg_size_w = 0.0
		flists.Avg_size_a = 0.0
	else:
		flists._ios_a += 1
		flists.Avg_size_a += (float(words[3]) / 2)
		if(words[1] == 'R'):
			flists._ios_r += 1
			flists.Avg_size_r += (float(words[3]) / 2)
		else:
			flists._ios_w += 1
			flists.Avg_size_w += (float(words[3]) / 2)


def do_IOPS_Cumulate(xdict_list, words, time_band=5):
	'''in {time_band} seconds. Not Complemented yet.
	'''
	time = words[0]
	return

def do_IOPS_Avg(xdict_list, words, time_band=60):
	'''in {time_band} seconds.
	'''
	time = float(words[0])
	if((flists.rec_start - 0) < 0.0001):
		flists.rec_start = time + 1
	
	if(time - (flists.rec_start - 1) > time_band):
		flists.rec_start = time + 1
		
		#insert
		insert_dict_2tuple(xdict_list[0], time, flists.Avg_IOPS_r)#read
		insert_dict_2tuple(xdict_list[1], time, flists.Avg_IOPS_w)#write
		insert_dict_2tuple(xdict_list[2], time, flists.Avg_IOPS_a)#all
		
		#reset
		flists.Avg_IOPS_r = 0
		flists.Avg_IOPS_w = 0
		flists.Avg_IOPS_a = 0

	else:
		flists.Avg_IOPS_a += 1
		if(words[1] == 'R'):
			flists.Avg_IOPS_r += 1
		else:
			flists.Avg_IOPS_w += 1
	
def display_dictlists_fp(feature_dict_lists):
	'''
	'''
	whichdictlist = ['offset','size','IOPS']
	whichdict = ['read', 'write', 'all']
	idx_diclist = 0
	for i in feature_dict_lists:
		#print "====",whichdictlist[idx_diclist],"===="
		idx_dict = 0
		for j in i:
			#print "---",whichdict[idx_dict],"---"
			fp=open('./'+whichdictlist[idx_diclist]+'_'+whichdict[idx_dict],"w+")
			display_dict(j, 'table_fp', fp, whichdictlist[idx_diclist])
			idx_dict += 1
			fp.close()
		idx_diclist += 1

	
def display_dictlists(feature_dict_lists):
	'''
	'''
	whichdictlist = ['offset','size','IOPS']
	whichdict = ['read', 'write', 'all']
	idx_diclist = 0
	for i in feature_dict_lists:
		print "====",whichdictlist[idx_diclist],"===="
		idx_dict = 0
		for j in i:
			print "---",whichdict[idx_dict],"---"
			display_dict(j, 'table')
			idx_dict += 1
		idx_diclist += 1

	
def generate_dicts():
	'''
	'''
	f_idx = 0
	for which in ['offset','size','IOPS']:
		#3 features' function
		whichs = {
					'offset': do_offset,
					'size': do_size_Avg,
					'IOPS': do_IOPS_Avg
		}
		#reset when it is time to begin
		flists.rec_start = 0
		flists.Avg_IOPS_r = 0
		flists.Avg_IOPS_w = 0
		flists.Avg_IOPS_a = 0
		
		flists.Avg_size_r = 0
		flists.Avg_size_w = 0
		flists.Avg_size_a = 0
		
		#open trace file & readline
		fp = open("../../temp_file/wechat-1h.trace", 'r', -1)
		for l in fp.readlines():
			words = l.split()
			if(len(words) == 7):
				whichs.get(which, do_default)(flists.feature_dict_lists[f_idx], words)
		fp.close()
		f_idx += 1

if __name__ == '__main__':
	generate_dicts()
	display_dictlists_fp(flists.feature_dict_lists)
