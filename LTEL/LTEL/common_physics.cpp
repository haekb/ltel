#include "common_physics.h"

#include "helpers.h"


LTELCommonPhysics::LTELCommonPhysics(LTELCommon* pCommon)
{
	m_ClientServerType = ServerType;
	m_vGlobalForce = DVector(0, -1000, 0);
	m_pCommon = pCommon;
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
#include <KinematicCollision.hpp>
#include <Math.hpp>

DRESULT LTELCommonPhysics::MoveObject(HOBJECT hObj, DVector* pPos, DDWORD flags)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	godot::KinematicBody* pKinematicBody = pObj->GetKinematicBody();


	if (!m_pCommon)
	{
		return DE_ERROR;
	}

	if (!pKinematicBody)
	{
		godot::Godot::print("MoveObject: No kinematic body found, fetching one...");
		godot::KinematicBody* pPrefab = GDCAST(godot::KinematicBody, pObj->GetNode()->get_node("/root/Scene/Prefabs/ClientBody"));
		
		if (!pPrefab)
		{
			return DE_ERROR;
		}
		
		pKinematicBody = GDCAST(godot::KinematicBody, pPrefab->duplicate());
		//pObj->GetNode()->add_child(pKinematicBody);
		auto p3D = pObj->GetNode()->get_node("/root/Scene/3D");
		p3D->add_child(pKinematicBody);
		pObj->SetKinematicBody(pKinematicBody);
	}

#if 1

	auto pRelVelocity = *pPos - pObj->GetPosition();

	auto pCollisionInfo = pKinematicBody->move_and_collide(LT2GodotVec3(pRelVelocity), true, true, true);

	if (pCollisionInfo.is_null())
	{
		pObj->SetPosition(pRelVelocity + pObj->GetPosition());
		pObj->SetLastCollision(nullptr);
	}
	else
	{
		//auto vBodyPos = pKinematicBody->get_translation();
		//auto vBodyRot = pKinematicBody->get_rotation();

		auto vBodyPos = pCollisionInfo->get_travel() + LT2GodotVec3(pObj->GetPosition());

		pObj->SetPosition(DVector(vBodyPos.x, vBodyPos.y, vBodyPos.z));
		pObj->SetLastCollision(pCollisionInfo);
	}
	


	//godot::Godot::print("MoveObject {3}: <{0}, {1}, {2}>", vBodyPos.x, vBodyPos.y, vBodyPos.z, (unsigned int)flags);


#if 1
	// We're on the floor!
	if (!pCollisionInfo.is_null() && ::acos(pCollisionInfo->get_normal().dot(godot::Vector3(0, 1, 0))) <= 0.1)
	{
		auto vCurrentVelocity = pObj->GetVelocity();
		pObj->SetVelocity(DVector(vCurrentVelocity.x, 0.0f, vCurrentVelocity.z));
	}
#endif

#else

	DVector vUp = DVector(0, 0, 0);
	DVector vForward = DVector(0, 0, 0);
	DVector vRight = DVector(0, 0, 0);
	DRotation vRot = pObj->GetRotation();
	m_pCommon->GetRotationVectors(vRot, vUp, vRight, vForward);

	auto pRelVelocity = *pPos - pObj->GetPosition();

//	pRelVelocity *= vForward;

	pKinematicBody->move_and_slide(LT2GodotVec3(pRelVelocity), LT2GodotVec3(vUp), true);

	auto vBodyPos = pKinematicBody->get_translation();
	auto vBodyRot = pKinematicBody->get_rotation();
	auto qBodyRot = godot::Quat();
	qBodyRot.set_euler(vBodyRot);

	//godot::Godot::print("MoveObject {3}: <{0}, {1}, {2}>", pRelVelocity.x, pRelVelocity.y, pRelVelocity.z, (unsigned int)flags);

	pObj->SetPosition(DVector(vBodyPos.x, vBodyPos.y, vBodyPos.z));
	//pObj->SetRotation(DRotation(qBodyRot.x, qBodyRot.y, qBodyRot.z, qBodyRot.w));

	/*
	if (pKinematicBody->is_on_floor())
	{
		auto vVelocity = pObj->GetVelocity();
		vVelocity.y = 0.0f;
		pObj->SetVelocity(vVelocity);
	}
	*/
#endif

	
	return DE_OK;

}

DRESULT LTELCommonPhysics::GetStandingOn(HOBJECT hObj, CollisionInfo* pInfo)
{
	pInfo->m_hObject = nullptr;
	pInfo->m_hPoly = INVALID_HPOLY;
	pInfo->m_Plane = DPlane(0, 0, 0, 1.0);
	pInfo->m_vStopVel = DVector(0, 0, 0);

	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;

	auto pCollision = pObj->GetLastCollision();

	if (pCollision.is_null())
	{
		return DE_OK;
	}

	auto vNormal = pCollision->get_normal();
	auto vStop = pCollision->get_remainder();

	// Cheat a little
	pInfo->m_hObject = hObj;

	// What's distance? 
	pInfo->m_Plane = DPlane(vNormal.x, vNormal.y, vNormal.z, 1.0f);
	pInfo->m_vStopVel = DVector(vStop.x, vStop.y, vStop.z);


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