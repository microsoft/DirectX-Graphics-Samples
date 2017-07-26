# -*- coding: utf-8 -*-
"""
Created on Wed May 27 10:42:38 2015

@author: jstanard
"""

from math import pi, sin, cos

def GenerateCircularSamplePattern( name, numSamples, radius = 1.0):
    print('static const float2 {}[{}] =\n{{'.format(name, numSamples))
    for sample in range(numSamples // 2):
        angle = sample / numSamples * 2.0 * pi
        x, y = radius * cos(angle), radius * sin(angle)
        print('    {{ {: 8f}, {: 8f} }}, {{ {: 8f}, {: 8f} }},'.format(x, y, -x, -y))
    print('}; // ', name)
    print();
        

GenerateCircularSamplePattern('s_Ring1', 8, 6.0)

GenerateCircularSamplePattern('s_Ring2', 16, 11.0)

GenerateCircularSamplePattern('s_Ring3', 24, 16.0)

def GenerateQuantizedCircularSamplePattern( name, numSamples, radius = 1.0):
    print('static const int {}[{}] =\n{{'.format(name, numSamples))
    for sample in range(numSamples // 2):
        angle = sample / numSamples * 2.0 * pi
        x, y = radius * cos(angle), radius * sin(angle)
        offset = round(x / 2) + round(y / 2) * 24
        print('    {: 4}, {: 4},'.format(offset, -offset))
    print('}; // ', name)
    print();


GenerateQuantizedCircularSamplePattern('s_Ring1Q', 8, 6.0)

GenerateQuantizedCircularSamplePattern('s_Ring2Q', 16, 11.0)

GenerateQuantizedCircularSamplePattern('s_Ring3Q', 24, 16.0)
