import os

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
	print "it\'s an error here."

def insert_dict(xdict, key):
	if(not xdict.has_key(key)):
		xdict[key] = 1
	else:
		xdict[key] += 1

def do_offset_adjust_ofs_band():
	'''this func not complemented.
	'''
	return 0


def display_dict_in_tables(xdict):
	#print 'xxxxx'
	#print xdict.items() 
	#sorted(xdict.items(), lambda x,y:cmp(x[1],y[1]))
	#print 'tttttt'
	#print xdict.items() 
	return 
	
def display_dict(xdict, way):
	if(way == 'table'):
		display_dict_in_tables(xdict)


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


def do_size(xdict, words):
	print 'here is do_size'

def do_IOPS(xdict, words):
	print 'here is do_IOPS'
	
def generate_dicts():
	f_idx = 0
	for which in ['offset','size','IOPS']:
		
		
		#3 feature_dict_lists for offset/size/IOPS
		feature_dict_lists = [[{}, {}, {}],	#lifetime ?
		[{}, {}, {}],
		[{}, {}, {}]]
		
		#3 features' function
		whichs = {
					'offset': do_offset,
					'size': do_size,
					'IOPS': do_IOPS
		}
		
		#open trace file & readline
		fp = open("../../temp_file/wechat-6h.trace", 'r', -1)
		for l in fp.readlines():
			words = l.split()
			if(len(words) == 7):
				whichs.get(which, do_default)(feature_dict_lists[f_idx], words)
		fp.close()
		f_idx += 1
	return feature_dict_lists
def display_dictlists(feature_dict_lists):
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

if __name__ == '__main__':
	feature_dict_lists = generate_dicts()
	display_dictlists(feature_dict_lists)
