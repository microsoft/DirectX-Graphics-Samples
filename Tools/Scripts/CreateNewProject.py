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
import shutil
from glob import glob
from uuid import uuid4

TEMPLATES_FOLDER = "./Tools/Scripts/ProjectTemplates"

def copy_template_file(filename, project, guid):
    '''Copies one template file and replaces templated values'''
    template_filename = os.path.join(TEMPLATES_FOLDER, filename)
    output_filename = os.path.join(project, filename)
    output_filename = output_filename.replace('AppTemplate', project)
    output_filename = output_filename.replace('LibTemplate', project)
    with open(template_filename, 'r', encoding='utf-8') as infile:
        with open(output_filename, 'w', encoding='utf-8') as outfile:
            contents = infile.read()
            contents = contents.replace('TEMPLATE_NAME', project)
            contents = contents.replace('TEMPLATE_GUID', guid)
            outfile.write(contents)

def copy_app_template(project, guid):
    '''Instantiates a new solution from a template'''
    shutil.copy(os.path.join(TEMPLATES_FOLDER, 'packages.config'), project)
    shutil.copy(os.path.join(TEMPLATES_FOLDER, 'pch.h'), project)
    shutil.copy(os.path.join(TEMPLATES_FOLDER, 'pch.cpp'), project)
    copy_template_file('Main.cpp', project, guid)
    copy_template_file('AppTemplate.sln', project, guid)
    copy_template_file('AppTemplate.vcxproj', project, guid)
    copy_template_file('AppTemplate.vcxproj.filters', project, guid)
    for file in glob(os.path.join(TEMPLATES_FOLDER, '*.png')):
        shutil.copy(file, project)

def copy_lib_template(project, guid):
    '''Instantiates a new library project from a template'''
    shutil.copy(os.path.join(TEMPLATES_FOLDER, 'pch.h'), project)
    shutil.copy(os.path.join(TEMPLATES_FOLDER, 'pch.cpp'), project)
    copy_template_file('LibTemplate.vcxproj', project, guid)
    copy_template_file('LibTemplate.vcxproj.filters', project, guid)

def create_project():
    if len(sys.argv) != 3 or sys.argv[1].lower() != 'app' and sys.argv[1].lower() != 'lib':
        print('Usage:  {0} [app|lib] <ProjectName>'.format(sys.argv[0]))
        return

    project_name = sys.argv[2]
    folder_contents = set(os.listdir())
    expected_contents = set(['readme.md', 'Core', 'Tools'])
    if not expected_contents.issubset(folder_contents):
        print('Run this script from the root of MiniEngine')
    elif project_name in folder_contents:
        print('Project already exists')
    else:
        os.mkdir(project_name)
        if sys.argv[1].lower() == 'app':
            copy_app_template(project_name, str(uuid4()).upper())
        else:
            copy_lib_template(project_name, str(uuid4()).upper())

if __name__ == "__main__":
    create_project()
