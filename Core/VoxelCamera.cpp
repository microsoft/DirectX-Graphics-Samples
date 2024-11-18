#include "pch.h"
#include "VoxelCamera.h"

#include "DirectXMath.h"

using namespace Math;

// TODO: probably need params here that change the bounds
void VoxelCamera::UpdateMatrix(int i)
{
	constexpr float DUMMY = 4000;

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

	Matrix4 ortho(XMMatrixOrthographicRH(DUMMY, DUMMY, -(DUMMY / 2), DUMMY / 2));

	SetProjMatrix(ortho);
	Update();
}