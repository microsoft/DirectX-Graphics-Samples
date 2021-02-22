//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:   James Stanard
//

#pragma once

#include <vector>
#include <cstdint>

//
// An animation curve describes how a value (or values) change over time.
// Key frames punctuate the curve, and times inbetween key frames are interpolated
// using the selected method.  Note that a curve does not have to be defined for
// the entire animation.  Some property might only be animated for a portion of time.
//
struct AnimationCurve
{
    enum { kTranslation, kRotation, kScale, kWeights }; // targetPath
    enum { kLinear, kStep, kCatmullRomSpline, kCubicSpline }; // interpolation
    enum { kSNorm8, kUNorm8, kSNorm16, kUNorm16, kFloat }; // format

    uint32_t targetNode : 28;           // Which node is being animated
    uint32_t targetPath : 2;            // What aspect of the transform is animated
    uint32_t interpolation : 2;         // The method of interpolation
    uint32_t keyFrameOffset : 26;       // Byte offset to first key frame
    uint32_t keyFrameFormat : 3;        // Data format for the key frames
    uint32_t keyFrameStride : 3;        // Number of 4-byte words for one key frame
    float numSegments;                  // Number of evenly-spaced gaps between keyframes
    float startTime;                    // Time stamp of the first key frame
    float rangeScale;                   // numSegments / (endTime - startTime)
};

//
// An animation is composed of multiple animation curves.
//
struct AnimationSet
{
    float duration;             // Time to play entire animation
    uint32_t firstCurve;        // Index of the first curve in this set (stored separately)
    uint32_t numCurves;         // Number of curves in this set
};

//
// Animation state indicates whether an animation is playing and keeps track of current
// position within the animation's playback.
//
struct AnimationState
{
    enum eMode { kStopped, kPlaying, kLooping };
    eMode state;
    float time;
    AnimationState() : state(kStopped), time(0.0f) {}
};
