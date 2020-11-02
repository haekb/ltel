#include "client.h"
#include "helpers.h"

#include <Godot.hpp>
#include <Spatial.hpp>
#include <Quat.hpp>

#define USRFLG_VISIBLE					(1<<0)
#define USRFLG_NIGHT_INFRARED			(1<<1)
#define USRFLG_IGNORE_PROJECTILES		(1<<2)

extern LTELClient* g_pLTELClient;

HLOCALOBJ impl_CreateObject(ObjectCreateStruct* pStruct)
{
	godot::Spatial* pNode = nullptr;

	godot::Spatial* p3DNode = godot::Object::cast_to<godot::Spatial>(g_pLTELClient->m_pGodotLink->get_node("/root/Scene/Camera"));


	if (pStruct->m_ObjectType == OT_CAMERA)
	{
		// Camera doesn't require any other settings
		return (HLOCALOBJ)p3DNode->get_parent();
	}
	if (pStruct->m_ObjectType == OT_NORMAL)
	{
		pNode = godot::Spatial::_new();

		// Add it into the world!
		p3DNode->add_child(pNode);

		pNode->set_translation(godot::Vector3(pStruct->m_Pos.x, pStruct->m_Pos.y, pStruct->m_Pos.z));

		godot::Quat vQuat = godot::Quat(pStruct->m_Rotation.m_Vec.x, pStruct->m_Rotation.m_Vec.y, pStruct->m_Rotation.m_Vec.z, pStruct->m_Rotation.m_Spin);
		auto vEuler = vQuat.get_euler();
		pNode->set_rotation(vEuler);
		pNode->set_scale(godot::Vector3(pStruct->m_Scale.x, pStruct->m_Scale.y, pStruct->m_Scale.z));
		pNode->set_visible(pStruct->m_Flags & USRFLG_VISIBLE);

		return (HLOCALOBJ)pNode;
	}

	godot::Godot::print("Trying to make some other object! aaaaaaaaaaaaa");


	return (HLOCALOBJ)pNode;
}

void impl_GetObjectPos(HLOCALOBJ hObj, DVector* pPos)
{
	godot::Spatial* pNode = (godot::Spatial*)hObj;

	if (!pNode)
	{
		return;
	}

	auto vPos = pNode->get_translation();

	pPos->x = vPos.x;
	pPos->y = vPos.y;
	pPos->z = vPos.z;
}

void impl_GetObjectRotation(HLOCALOBJ hObj, DRotation* pRotation)
{
	godot::Spatial* pNode = (godot::Spatial*)hObj;

	if (!pNode)
	{
		return;
	}

	auto vRot = pNode->get_rotation();
	godot::Quat vQuat;
	vQuat.set_euler(vRot);

	pRotation->m_Vec.x = vQuat.x;
	pRotation->m_Vec.y = vQuat.y;
	pRotation->m_Vec.z = vQuat.z;
	pRotation->m_Spin = vQuat.w;//1.0f;
}

void impl_EulerRotateX(DRotation* pRotation, float amount)
{
	godot::Quat vQuat = godot::Quat(pRotation->m_Vec.x, pRotation->m_Vec.y, pRotation->m_Vec.z, pRotation->m_Spin);
	auto vEuler = vQuat.get_euler();

	vEuler = vEuler.rotated(godot::Vector3(1.0f, 0.0f, 0.0f), amount);

	vQuat.set_euler(vEuler);
	pRotation->m_Vec.x = vQuat.x;
	pRotation->m_Vec.y = vQuat.y;
	pRotation->m_Vec.z = vQuat.z;
	pRotation->m_Spin = vQuat.w;//1.0f;
}

DRESULT impl_SetObjectUserFlags(HLOCALOBJ hObj, DDWORD flags)
{
	godot::Spatial* pNode = (godot::Spatial*)hObj;

	if (!pNode)
	{
		return DE_ERROR;
	}

	// We only support this right now!
	pNode->set_visible(flags & USRFLG_VISIBLE);
	return DE_OK;
}

// This must be last!
void LTELClient::InitObjectImpl()
{
	// Object functionality
	CreateObject = impl_CreateObject;
	GetObjectPos = impl_GetObjectPos;
	GetObjectRotation = impl_GetObjectRotation;
	EulerRotateX = impl_EulerRotateX;
	SetObjectUserFlags = impl_SetObjectUserFlags;
}