#include "common_physics.h"

#include "helpers.h"

LTELCommonPhysics::LTELCommonPhysics()
{
	m_ClientServerType = ServerType;
	m_vGlobalForce = DVector(0, -1000, 0);
}

LTELCommonPhysics::~LTELCommonPhysics()
{
}


DRESULT LTELCommonPhysics::GetFrictionCoefficient(HOBJECT hObj, float& coeff)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	coeff = pObj->GetFrictionCoeff();
	return DE_OK;
}

DRESULT LTELCommonPhysics::SetFrictionCoefficient(HOBJECT hObj, float coeff)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	pObj->SetFrictionCoeff(coeff);
	return DE_OK;
}

DRESULT LTELCommonPhysics::GetForceIgnoreLimit(HOBJECT hObj, float& limit)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	limit = pObj->GetForceIgnoreLimit();
	return DE_OK;
}

DRESULT LTELCommonPhysics::SetForceIgnoreLimit(HOBJECT hObj, float limit)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	pObj->SetForceIgnoreLimit(limit);
	return DE_OK;
}

DRESULT LTELCommonPhysics::GetVelocity(HOBJECT hObj, DVector* pVel)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	*pVel = pObj->GetVelocity();
	return DE_OK;
}

DRESULT LTELCommonPhysics::SetVelocity(HOBJECT hObj, DVector* pVel)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	pObj->SetVelocity(*pVel);
	return DE_OK;
}

DRESULT LTELCommonPhysics::GetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	*pAccel = pObj->GetAccel();
	return DE_OK;
}

DRESULT LTELCommonPhysics::SetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	pObj->SetAccel(*pAccel);
	return DE_OK;
}

DRESULT LTELCommonPhysics::GetObjectMass(HOBJECT hObj, float& mass)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	mass = pObj->GetMass();
	return DE_OK;
}

DRESULT LTELCommonPhysics::SetObjectMass(HOBJECT hObj, float mass)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	pObj->SetMass(mass);
	return DE_OK;
}

DRESULT LTELCommonPhysics::GetObjectDims(HOBJECT hObj, DVector* pNewDims)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	*pNewDims = pObj->GetDims();
	return DE_OK;
}

DRESULT LTELCommonPhysics::SetObjectDims(HOBJECT hObj, DVector* pNewDims, DDWORD flags)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	pObj->SetDims(*pNewDims);
	return DE_OK;
}

#include <RayCast.hpp>
DRESULT LTELCommonPhysics::MoveObject(HOBJECT hObj, DVector* pPos, DDWORD flags)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;

	DVector vOldPos = pObj->GetPosition();
	DVector vNewPos = *pPos + vOldPos;

	godot::Godot::print("MoveObject {3}: <{0}, {1}, {2}>", vNewPos.x, vNewPos.y, vNewPos.z, (unsigned int)flags);

	// Setup a new ray, and enable it
	godot::RayCast* pRayCast = godot::RayCast::_new();
	pObj->GetNode()->get_parent()->add_child(pRayCast, true);
	pRayCast->set_cast_to(LT2GodotVec3(vNewPos));
	pRayCast->set_translation(LT2GodotVec3(vOldPos));
	pRayCast->set_enabled(true);
	

	// Cast it!
	pRayCast->force_raycast_update();


	if (pRayCast->is_colliding())
	{
		auto vCollision = pRayCast->get_collision_point();
		auto vCollisionLT = DVector(vCollision.x, vCollision.y, vCollision.z);

		auto vAdjustedPoint = vNewPos - vCollisionLT;
		/*
		
		P <--- Player
		|
		|
		|
		_ <--- Collision Point
		| 
		| <--- Total vNewPos
		
		
		*/

		//if (vAdjustedPoint.y > 0)
		{
//			vNewPos -= vAdjustedPoint;
		}

		//if (vAdjustedPoint.MagSqr() < 0.1f)
		{
			pObj->SetAccel(DVector(0, 0, 0));
			pObj->SetVelocity(DVector(0, 0, 0));
			pRayCast->queue_free();
			return DE_OK;
		}


	}

	pRayCast->queue_free();

#if 0
	if (0) {
		DVector vOldPos = pObj->GetPosition();
		DVector vNewPos = vOldPos + velocity;

		// Setup a new ray, and enable it
		godot::RayCast* pRayCast = godot::RayCast::_new();
		pObj->GetNode()->get_parent()->add_child(pRayCast, true);
		pRayCast->set_cast_to(LT2GodotVec3(vNewPos));
		pRayCast->set_translation(LT2GodotVec3(vOldPos));
		pRayCast->set_enabled(true);

		// Cast it!
		pRayCast->force_raycast_update();


		if (pRayCast->is_colliding())
		{
#if 1
			auto vgNormal = pRayCast->get_collision_normal();
			DVector vNormal = DVector(vgNormal.x, vgNormal.y, vgNormal.z);

			//P = -N * (V * N)
			//a = P * t

			vPoint = -vNormal * (velocity * vNormal);
			accel = vPoint * pInfo->m_dt;

			//vNewPos = vOldPos;
#else
			auto vgCollidePoint = pRayCast->get_collision_point();
			DVector vLocalCollision = DVector(vgCollidePoint.x - vOldPos.x, vgCollidePoint.y - vOldPos.y, vgCollidePoint.z - vOldPos.z);
			vNewPos -= vLocalCollision;
#endif
		}


		pRayCast->queue_free();

	}
#endif
	pObj->SetPosition(vNewPos);
	pObj->GetNode()->force_update_transform();
	//pObj->SetAccel(DVector(0, 0, 0));

	return DE_OK;
}

DRESULT LTELCommonPhysics::GetStandingOn(HOBJECT hObj, CollisionInfo* pInfo)
{


	pInfo->m_hObject = nullptr;
	pInfo->m_hPoly = INVALID_HPOLY;
	pInfo->m_Plane = DPlane(0, 0, 0, 1.0);
	pInfo->m_vStopVel = DVector(0, 0, 0);



	return DE_OK;
}

DRESULT LTELCommonPhysics::GetWorldObject(HOBJECT* hObj)
{
	return DE_ERROR;
}

DRESULT LTELCommonPhysics::GetGlobalForce(DVector& vec)
{
	vec = m_vGlobalForce;

	return DE_OK;
}

DRESULT LTELCommonPhysics::SetGlobalForce(DVector& vec)
{
	m_vGlobalForce = vec;

	return DE_OK;
}


//
// Plz ignore
//

DRESULT PhysicsLT::GetFrictionCoefficient(HOBJECT hObj, float& coeff)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::SetFrictionCoefficient(HOBJECT hObj, float coeff)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::GetForceIgnoreLimit(HOBJECT hObj, float& limit)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::SetForceIgnoreLimit(HOBJECT hObj, float limit)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::GetVelocity(HOBJECT hObj, DVector* pVel)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::SetVelocity(HOBJECT hObj, DVector* pVel)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::GetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::SetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::GetObjectMass(HOBJECT hObj, float& mass)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::SetObjectMass(HOBJECT hObj, float mass)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::GetObjectDims(HOBJECT hObj, DVector* pNewDims)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::SetObjectDims(HOBJECT hObj, DVector* pNewDims, DDWORD flags)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::MoveObject(HOBJECT hObj, DVector* pPos, DDWORD flags)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::GetStandingOn(HOBJECT hObj, CollisionInfo* pInfo)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::GetWorldObject(HOBJECT* hObj)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::GetGlobalForce(DVector& vec)
{
	return DE_ERROR;
}

DRESULT PhysicsLT::SetGlobalForce(DVector& vec)
{
	return DE_ERROR;
}
