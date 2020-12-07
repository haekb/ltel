#include "client_physics.h"
#include "client.h"

LTELClientPhysics::LTELClientPhysics(LTELClient* pClient)
{
	m_ClientServerType = ClientType;
	m_pClient = pClient;
	m_pCommonPhysics = new LTELCommonPhysics((LTELCommon*)pClient->Common());
}

LTELClientPhysics::~LTELClientPhysics()
{
	delete m_pCommonPhysics;
	m_pCommonPhysics = nullptr;
}


//
// Client specific info
//
#include "helpers.h"
#include <RayCast.hpp>
// Updates the object's movement using its velocity, acceleration, and the
// time delta passed in (usually the frame time).  Fills in m_Offset with the
// position delta you should apply.
DRESULT LTELClientPhysics::UpdateMovement(MoveInfo* pInfo)
{
	if (!pInfo->m_hObject)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)pInfo->m_hObject;

	DVector accel;
	DVector velocity;
	DVector offset;

	accel = pObj->GetAccel();
	velocity = pObj->GetVelocity();
	offset.Init();

	if (accel.MagSqr() < 0.1f)
	{
		accel.Init();
	}

	if (velocity.MagSqr() < 0.1f)
	{
		velocity.Init();
	}

	if (pObj->IsAffectedByGravity())
	{
		// Apply any force (mostly gravity)
		accel += m_pCommonPhysics->m_vGlobalForce;
	}

	// New Velocity
	velocity += accel * pInfo->m_dt;

	// New Position
	pInfo->m_Offset = velocity * pInfo->m_dt;

	pObj->SetVelocity(velocity);
	pObj->SetPhysicsDeltaTime(pInfo->m_dt);

	return DE_OK;
}

// Move the specified object but only test for collisions/pushing on
// the objects specified.  It'll carry things standing on it.
DRESULT LTELClientPhysics::MovePushObjects(HOBJECT hToMove, DVector& newPos,
	HOBJECT* hPushObjects, DDWORD nPushObjects)
{
	return DE_OK;
}

// Rotate the specified object but only test for collisions/pushing on
// the objects specified.  It'll carry things standing on it.
// This only works on world models.
DRESULT LTELClientPhysics::RotatePushObjects(HOBJECT hToMove, DRotation& newRot,
	HOBJECT* hPushObjects, DDWORD nPushObjects)
{
	return DE_OK;
}

//
// These should simply pass to CommonPhysics
//



DRESULT LTELClientPhysics::GetFrictionCoefficient(HOBJECT hObj, float& coeff)
{
	return m_pCommonPhysics->GetFrictionCoefficient(hObj, coeff);
}

DRESULT LTELClientPhysics::SetFrictionCoefficient(HOBJECT hObj, float coeff)
{
	return m_pCommonPhysics->SetFrictionCoefficient(hObj, coeff);
}

DRESULT LTELClientPhysics::GetForceIgnoreLimit(HOBJECT hObj, float& limit)
{
	return m_pCommonPhysics->GetForceIgnoreLimit(hObj, limit);
}

DRESULT LTELClientPhysics::SetForceIgnoreLimit(HOBJECT hObj, float limit)
{
	return m_pCommonPhysics->SetForceIgnoreLimit(hObj, limit);
}

DRESULT LTELClientPhysics::GetVelocity(HOBJECT hObj, DVector* pVel)
{
	return m_pCommonPhysics->GetVelocity(hObj, pVel);
}

DRESULT LTELClientPhysics::SetVelocity(HOBJECT hObj, DVector* pVel)
{
	return m_pCommonPhysics->SetVelocity(hObj, pVel);
}

DRESULT LTELClientPhysics::GetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	return m_pCommonPhysics->GetAcceleration(hObj, pAccel);
}

DRESULT LTELClientPhysics::SetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	return m_pCommonPhysics->SetAcceleration(hObj, pAccel);
}

DRESULT LTELClientPhysics::GetObjectMass(HOBJECT hObj, float& mass)
{
	return m_pCommonPhysics->GetObjectMass(hObj, mass);
}

DRESULT LTELClientPhysics::SetObjectMass(HOBJECT hObj, float mass)
{
	return m_pCommonPhysics->SetObjectMass(hObj, mass);
}

DRESULT LTELClientPhysics::GetObjectDims(HOBJECT hObj, DVector* pNewDims)
{
	return m_pCommonPhysics->GetObjectDims(hObj, pNewDims);
}

DRESULT LTELClientPhysics::SetObjectDims(HOBJECT hObj, DVector* pNewDims, DDWORD flags)
{
	return m_pCommonPhysics->SetObjectDims(hObj, pNewDims, flags);
}

DRESULT LTELClientPhysics::MoveObject(HOBJECT hObj, DVector* pPos, DDWORD flags)
{
	return m_pCommonPhysics->MoveObject(hObj, pPos, flags);
}

DRESULT LTELClientPhysics::GetStandingOn(HOBJECT hObj, CollisionInfo* pInfo)
{
	return m_pCommonPhysics->GetStandingOn(hObj, pInfo);
}

DRESULT LTELClientPhysics::GetWorldObject(HOBJECT* hObj)
{
	return m_pCommonPhysics->GetWorldObject(hObj);
}

DRESULT LTELClientPhysics::GetGlobalForce(DVector& vec)
{
	return m_pCommonPhysics->GetGlobalForce(vec);
}

DRESULT LTELClientPhysics::SetGlobalForce(DVector& vec)
{
	return m_pCommonPhysics->SetGlobalForce(vec);
}
