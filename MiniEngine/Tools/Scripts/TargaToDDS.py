def CompileTGA( filename, normalMap=False ):
	import subprocess

	args = 'texconv.exe ' + filename + ' -nologo -vflip'
	if normalMap:
		args += ' -f BC1_UNORM'
	else:
		args += ' -srgbi -f BC1_UNORM_SRGB'

	print('Calling "{0}"'.format(args))
	subprocess.call(args.split())

if __name__ == "__main__":
	import os
	files = os.listdir()
	for file in files:
		if file.lower().endswith('.tga'):
			CompileTGA(file, file.find('normal') != -1)
