#pragma once

enum Descriptors
{
    RENDER_TARGET,
    TLAS,

    SCENE_CBV_0,    // Double buffered
    SCENE_CBV_1,    // Double buffered

    TREE_CBV_0,     // Double buffered
    TREE_CBV_1,     // Double buffered

    COUNT
};