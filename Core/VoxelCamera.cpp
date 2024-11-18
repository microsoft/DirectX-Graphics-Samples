#include "pch.h"
#include "VoxelCamera.h"

#include "DirectXMath.h"

using namespace Math;

// TODO: probably need params here that change the bounds
void VoxelCamera::UpdateMatrix(int i)
{
	constexpr float DUMMY = 2000;
	float right, left, top, bottom, _near, _far = 0;

	if (i == 0) {
			SetEyeAtUp(Vector3(0, 0, 0), Vector3(-1, 0, 0), Vector3(0, 1, 0));
			//Left->Right
	}
	else if (i == 1) {
			SetEyeAtUp(Vector3(0, 0, 0), Vector3(0, -1, 0), Vector3(0, 0, 1));
	}
	else {
			//Front->Back
			SetEyeAtUp(Vector3(0, 0, 0), Vector3(0, 0, -1), Vector3(0, 1, 0));
	}

	Matrix4 ortho(XMMatrixOrthographicRH(4000, 4000, -2000, 2000));

	SetProjMatrix(ortho);
	Update();
}