def ConvertFile( binaryFileName, headerFileName=None, symbolName=None ):

	# Open binary file for reading
	print('Attempting to convert binary file ' + binaryFileName)
	infile = open(binaryFileName, 'rb')

	# Create a header file name if none specified
	if headerFileName == None:
		suffixStart = binaryFileName.rfind('.')
		if suffixStart > 0:
			headerFileName = binaryFileName[:suffixStart]
		else:
			headerFileName = binaryFileName
		headerFileName += '.h'

	# If no symbol name was specified, derive it from the header file name
	if symbolName == None:
		symbolName = 'g_p' + headerFileName.rstrip('.h')
	
	# Read contents
	contents = infile.read()

	# Begin writing new file
	outfile = open(headerFileName, 'w')
	outfile.write('unsigned char {0}[{1}] =\n{{\n'.format(symbolName, len(contents)))

	bulkCount = len(contents)
	remainder = bulkCount % 16
	if remainder == 0:
		remainder = 16
	bulkCount -= remainder
	
	for idx in range(0, bulkCount, 16):
		outfile.write('\t')
		for c in contents[idx : idx + 16]:
			outfile.write( '{0:#0{1}x},'.format(int(c), 4) )
		outfile.write('\n')
	outfile.write('\t')
	for c in contents[-remainder : -1]:
		outfile.write( '{0:#0{1}x},'.format(int(c), 4) )
	outfile.write( '{0:#0{1}x}\n'.format(int(contents[-1]), 4) + '};\n' )
	
	outfile.close()
	
	print('Created binary symbol {0} in header file {1} '.format(symbolName, headerFileName))
	
if __name__ == "__main__":
	import sys
	ConvertFile(sys.argv[1])