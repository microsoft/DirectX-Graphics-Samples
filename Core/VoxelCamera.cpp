#include "pch.h"
#include "VoxelCamera.h"

using namespace Math;

// TODO: probably need params here that change the bounds
void VoxelCamera::UpdateMatrix(int i)
{
	constexpr float DUMMY = 2000;
	float right, left, top, bottom, _near, _far = 0;

	if (i == 0) {
			SetEyeAtUp(Vector3(3000, 0, 0), Vector3(0, 0, 0), Vector3(0, 1, 0));
			//Left->Right
			right = DUMMY;
			left = -DUMMY;
			top = DUMMY;
			bottom = -DUMMY;
			_near = -DUMMY;
			_far = DUMMY;
	}
	else if (i == 1) {
			SetEyeAtUp(Vector3(0, 3000, 0), Vector3(0, 0, 0), Vector3(0, 0, -1));
			//Top->Down
			right = DUMMY;
			left = -DUMMY;
			top = DUMMY;
			bottom = -DUMMY;
			_near = DUMMY;
			_far = -DUMMY;
	}
	else {
			//Front->Back
			SetEyeAtUp(Vector3(0, 0, 3000), Vector3(0, 0, 0), Vector3(0, 1, 0));
			right = DUMMY;
			left = -DUMMY;
			top = DUMMY;
			bottom = -DUMMY;
			_near = -DUMMY;
			_far = DUMMY;
	}


	float rml = right - left;
	float rpl = right + left;
	float tmb = top - bottom;
	float tpb = top + bottom;
	float fmn = _far - _near;
	float fpn = _far + _near;

	// looking in a positive x direction for now
	

	// create an orthographic matrix
	// https://wikimedia.org/api/rest_v1/media/math/render/svg/8ea4e438d7439b8fa504fb53fd7fafd678007243
	Matrix4 ortho = Matrix4::MakeScale(Vector3(1., 1., 1.));
	{
		ortho.SetX(Vector4(2. / rml, 0, 0, 0));
		ortho.SetY(Vector4(0, 2. / tmb, 0, 0));
		ortho.SetZ(Vector4(0, 0, -2. / fmn, 0));
		ortho.SetW(Vector4(-(rpl / rml), -(tpb / tmb), -(fpn / fmn), 1.));
	}

	SetProjMatrix(ortho);
	Update();
}