# -*- coding: utf-8 -*-
'''
Copyright (c) Microsoft. All rights reserved.
This code is licensed under the MIT License (MIT).
THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.

Developed by Minigraph

Author:  James Stanard
'''

import os
import sys

def clean_line(line):
    '''Cleans the white space on a single line of text'''
    line = line.rstrip(' \n') + '\n'
    leading_tabs = 0
    while line[leading_tabs] == '\t':
        leading_tabs += 1
    return '    ' * leading_tabs + line[leading_tabs:]

def clean_file(filename):
    '''Cleans the white space in a file'''
    print('processing ' + filename)
    try:
        lines = open(filename, 'r').readlines()
        with open(filename, 'w') as outfile:
            for line in lines:
                outfile.write(clean_line(line))
    except OSError:
        print('  ***Failed')

if __name__ == "__main__":
    start_path = '.'
    if len(sys.argv) > 1:
        start_path = sys.argv[1]
    for root, dirs, files in os.walk(start_path):
        # Clean only files with these extensions
        for file in files:
            if file.lower().endswith(('.h', '.cpp', '.hlsl', '.hlsli', '.py')):
                clean_file(os.path.join(root, file))
        # Do not walk these subdirectories.  Remove them from the list
        i = 0
        while i < len(dirs):
            if dirs[i].startswith(('3rdParty', 'Tools', 'Build', 'Durango', 'Fonts', 'users')):
                del dirs[i]
            else:
                i += 1
