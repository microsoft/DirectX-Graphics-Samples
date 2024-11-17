#pragma once
#include "Camera.h"
#include "VectorMath.h"

class VoxelCamera : public Math::BaseCamera
{
public:

	VoxelCamera() {}

	void UpdateMatrix(int i);
};
