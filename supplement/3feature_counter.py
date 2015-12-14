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
	print 'do nothing'

def do_offset(xdict, words, ofs_band=1):
	#default 1M-width.
	#default no time-consideration.
	time = words[0]
	ofs = words[2]
	
	
	print time, ofs

def do_size(xdict, words):
	print 'here is do_size'

def do_IOPS(xdict, words):
	print 'here is do_IOPS'
	
def generate_dicts():
	for which in ['offset','size','IOPS']:
		#the feature dict.
		feature_dict = dict();
		
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
				whichs.get(which, do_default)(feature_dict, words)
		fp.close()

if __name__ == '__main__':
	generate_dicts()