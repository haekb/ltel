#include "common.h"
#include "helpers.h"

LTELCommon::LTELCommon()
{
}

LTELCommon::~LTELCommon()
{
}

DRESULT LTELCommon::Parse(ConParse* pParse)
{
	return DRESULT();
}

DRESULT LTELCommon::GetObjectType(HOBJECT hObj, DDWORD* type)
{
	return DRESULT();
}

DRESULT LTELCommon::GetModelAnimUserDims(HOBJECT hObject, DVector* pDims, HMODELANIM hAnim)
{
	return DRESULT();
}

DRESULT LTELCommon::GetRotationVectors(DRotation& rot, DVector& up, DVector& right, DVector& forward)
{
	godot::Quat vQuat = LT2GodotQuat(&rot);

	godot::Basis vBasis = godot::Basis(vQuat);

	godot::Vector3 vForward = -vBasis.z;
	godot::Vector3 vRight = vBasis.x;
	godot::Vector3 vUp = vBasis.y;

	forward = DVector(vForward.x, vForward.y, vForward.z);
	right = DVector(vRight.x, vRight.y, vRight.z);
	up = DVector(vUp.x, vUp.y, vUp.z);

	return DE_OK;
}

DRESULT LTELCommon::SetupEuler(DRotation& rot, float pitch, float yaw, float roll)
{
	godot::Quat qRot = godot::Quat();
	qRot.set_euler(godot::Vector3(yaw, pitch, roll));

	rot = DRotation(qRot.x, qRot.y, qRot.z, qRot.w);

	return DE_OK;
}

DRESULT LTELCommon::CreateMessage(LMessage*& pMsg)
{
	return DRESULT();
}

DRESULT LTELCommon::GetPointStatus(DVector* pPoint)
{
	return DRESULT();
}

DRESULT LTELCommon::GetPointShade(DVector* pPoint, DVector* pColor)
{
	return DRESULT();
}

DRESULT LTELCommon::GetPolyTextureFlags(HPOLY hPoly, DDWORD* pFlags)
{
	return DRESULT();
}
