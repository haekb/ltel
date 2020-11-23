#include "common_physics.h"

LTELCommonPhysics::LTELCommonPhysics()
{
	m_ClientServerType = ServerType;
}

LTELCommonPhysics::~LTELCommonPhysics()
{
}

DRESULT LTELCommonPhysics::GetFrictionCoefficient(HOBJECT hObj, float& coeff)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::SetFrictionCoefficient(HOBJECT hObj, float coeff)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::GetForceIgnoreLimit(HOBJECT hObj, float& limit)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::SetForceIgnoreLimit(HOBJECT hObj, float limit)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::GetVelocity(HOBJECT hObj, DVector* pVel)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::SetVelocity(HOBJECT hObj, DVector* pVel)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::GetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::SetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::GetObjectMass(HOBJECT hObj, float& mass)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::SetObjectMass(HOBJECT hObj, float mass)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::GetObjectDims(HOBJECT hObj, DVector* pNewDims)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::SetObjectDims(HOBJECT hObj, DVector* pNewDims, DDWORD flags)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::MoveObject(HOBJECT hObj, DVector* pPos, DDWORD flags)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::GetStandingOn(HOBJECT hObj, CollisionInfo* pInfo)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::GetWorldObject(HOBJECT* hObj)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::GetGlobalForce(DVector& vec)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::SetGlobalForce(DVector& vec)
{
	return DE_ERROR;
}
