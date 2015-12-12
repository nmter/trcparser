def generate_feature_dict(dict, fObj, what):
	dict[1] = 4
	print dict[1]


if __name__ == '__main__':
	feature_dict_list = {}
	feature_dict_list['size'] = dict()
	generate_feature_dict(feature_dict_list['size'], 0, 0)