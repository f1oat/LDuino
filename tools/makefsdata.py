import os
import sys
import fnmatch
import binascii
import gzip

declaration="""
struct httpd_fsdata_file {
	const char *name;
	const int len;
	const boolean gzip;
	const struct httpd_fsdata_segment *first;
	const struct httpd_fsdata_file *next;
};

struct httpd_fsdata_segment {
	const char *data;
	const struct httpd_fsdata_segment *next;
};

"""

symbols = []
segment_len = 30000
use_gzip = True

def GenerateStructures(output):
	global symbols
	next_symbol = "NULL"
	for (symbol, segment, len) in symbols:
		next_segment = "NULL"
		for s in range(segment,-1,-1):

			output.write("static const struct httpd_fsdata_segment %s_segment%d PROGMEM = {%s_data%d, %s};\n" 
				% (symbol, s, symbol, s, next_segment))
			next_segment = "&%s_segment%d" % (symbol, s)
		output.write("static const struct httpd_fsdata_file %s_file PROGMEM = {%s_name, %d, %s, &%s_segment0, %s};\n" 
				% (symbol, symbol, len, "true", symbol, next_symbol))
		next_symbol = "&" + symbol + "_file"
			

def GetFilesList(rootdir):
	# Read list of file patterns to ignore
	fo = open(rootdir + "/ignore.txt", "r")
	ignore = fo.readlines()
	fo.close()
	ignore = [os.path.normpath(s)[:-1] for s in ignore]  # remove LF

	# Get the list of files
	file_list = []
	for root, subdirs, files in os.walk(rootdir):
		for file in files:
			filePath = os.path.join(root,file)
			filePath = os.path.relpath(filePath, rootdir)
			skip = False
			for pattern in ignore:
				if fnmatch.fnmatch(filePath, os.path.join(pattern)):
					skip = True
					print("SKIP " + filePath)
			if (not skip):
				file_list.append(filePath)
	return file_list

def EncodeFile(output, rootdir, file, use_gzip=False):
	global symbols
	fname = file.replace("\\", "/")
	symbol = fname.replace("/", "_")
	symbol = symbol.replace(".", "_")
	output.write("static const char %s_name[] PROGMEM = \"/%s\";\n" % (symbol, fname))
	file = os.path.join(rootdir, file)

	fi = open(file, "rb")
	file_content = fi.read()
	fi.close()

	if use_gzip:
		file_content = gzip.compress(file_content)
			
	length = len(file_content)
	segment = 0
	cnt = 0

	for index in range(0, length):
		if cnt == segment_len:
			output.write("\n};\n")
			segment += 1
			cnt = 0
		if cnt == 0:
			output.write("static const char %s_data%d[] PROGMEM = {\n" % (symbol, segment))
		h = "0x%02X" % (file_content[index])
		if cnt == 0:
			output.write("\t" + h)
		else:
			if (cnt % 10) == 0:
				output.write(",\n\t" + h)
			else:
				output.write(", " + h)
		cnt += 1

	symbols.append((symbol, segment, length))
	output.write("\n};\n\n")

rootdir = "web"
fo = open("httpd-fsdata.h", "w")
file_list = GetFilesList(rootdir)
print(file_list)

for file in file_list:
	EncodeFile(fo, rootdir, file, use_gzip)

fo.write(declaration)
GenerateStructures(fo)
fo.write("\nconst int httpd_fsdata_segment_len = %d;\n" % (segment_len))
(root, segment, len) = symbols[-1] 
fo.write("const httpd_fsdata_file *httpd_fsroot = &%s_file;\n" % (root))
fo.close()