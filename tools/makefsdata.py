import os
import sys
import fnmatch
import binascii

declaration="""
struct httpd_fsdata_file {
	const char *name;
	const int len;
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

def GenerateStructures(output):
	global symbols
	next_symbol = "NULL"
	for (symbol, segment, len) in symbols:
		next_segment = "NULL"
		for s in xrange(segment,-1,-1):

			output.write("static const struct httpd_fsdata_segment %s_segment%d PROGMEM = {%s_data%d, %s};\n" 
				% (symbol, s, symbol, s, next_segment))
			next_segment = "&%s_segment%d" % (symbol, s)
		output.write("static const struct httpd_fsdata_file %s_file PROGMEM = {%s_name, %d, &%s_segment0, %s};\n" 
				% (symbol, symbol, len, symbol, next_symbol))
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
					print "SKIP " + filePath
			if (not skip):
				file_list.append(filePath)
	return file_list

def EncodeFile(output, rootdir, file):
	global symbols
	fname = file.replace("\\", "/")
	symbol = fname.replace("/", "_")
	symbol = symbol.replace(".", "_")
	output.write("static const char %s_name[] PROGMEM = \"/%s\";\n" % (symbol, fname))
	file = os.path.join(rootdir, file)
	len = os.path.getsize(file)
	segment = 0;
	cnt = 0
	fi = open(file, "rb")
	remains = len
	while remains > 0:
		if cnt == segment_len:
			output.write("\n};\n")
			segment += 1
			cnt = 0
		if cnt == 0:
			output.write("static const char %s_data%d[] PROGMEM = {\n" % (symbol, segment))
		data = fi.read(1);
		h = "0x" + binascii.hexlify(data)
		if cnt == 0:
			output.write("\t" + h)
		else:
			if (cnt % 10) == 0:
				output.write(",\n\t" + h)
			else:
				output.write(" ," + h)
		cnt += 1
		remains -= 1
	fi.close()
	symbols.append((symbol, segment, len))
	output.write("\n};\n\n")

rootdir = "web"
fo = open("httpd-fsdata.h", "w")
file_list = GetFilesList(rootdir)
print file_list

for file in file_list:
	EncodeFile(fo, rootdir, file)

fo.write(declaration)
GenerateStructures(fo)
fo.write("\nconst int httpd_fsdata_segment_len = %d;\n" % (segment_len))
(root, segment, len) = symbols[-1] 
fo.write("const httpd_fsdata_file *httpd_fsroot = &%s_file;\n" % (root))
fo.close()