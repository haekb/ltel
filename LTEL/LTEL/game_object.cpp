#include "game_object.h"
#include "helpers.h"

#include <Godot.hpp>
#include <Quat.hpp>
#include <Vector3.hpp>

GameObject::GameObject(ClassDef* pClass, BaseClass* pBaseClass)
{
	m_pClassDef = pClass;
	m_pBaseClass = pBaseClass;

	m_nObjectType = OT_NORMAL;
	m_nFlags = 0;
	m_nUserFlags = 0;
	m_vPos = DVector(0,0,0);
	m_vScale = DVector(1,1,1);
	m_vRotation = DRotation(0,0,0,1);
	m_nContainerCode = 0;
	m_pUserData = nullptr;
	m_sFilename = "";
	m_sSkinname = "";
	m_sName = "";
	m_fNextUpdate = 0.0f;
	m_fDeactivationTime = 0.0f;

	m_nState = 0;

	m_pNode = nullptr;
	m_pCamera = nullptr;
	m_pPolyGrid = nullptr;

	m_pExtraData = nullptr;
	m_pServerObject = nullptr; // Needed??
}

GameObject::~GameObject()
{
	// Camera is a global object, we do not delete it!
	if (m_nObjectType == OT_CAMERA)
	{
		return;
	}

	
	if (m_pPolyGrid)
	{
		m_pPolyGrid->free();
		m_pPolyGrid = nullptr;

		// Clear any cached results
		m_pNode = nullptr;

		LTELPolyGrid* pExtraData = (LTELPolyGrid*)GetExtraData();

		if (pExtraData->pColorTable)
		{
			free(pExtraData->pColorTable);
			pExtraData->pColorTable = nullptr;
		}

		if (pExtraData->pData)
		{
			free(pExtraData->pData);
		}
		pExtraData->pData = nullptr;
	}

	if (m_pNode)
	{
		m_pNode->free();
		m_pNode = nullptr;
	}

}

void GameObject::SetFromObjectCreateStruct(ObjectCreateStruct pStruct)
{
	m_nObjectType = pStruct.m_ObjectType;
	SetFlags(pStruct.m_Flags);
	SetPosition(pStruct.m_Pos);
	SetScale(pStruct.m_Scale);
	SetRotation(pStruct.m_Rotation);
	m_nContainerCode = pStruct.m_ContainerCode;
	m_pUserData = (void*)pStruct.m_UserData;
	m_sFilename = pStruct.m_Filename;
	m_sSkinname = pStruct.m_SkinName;
	m_sName = pStruct.m_Name;
	m_fNextUpdate = pStruct.m_NextUpdate;
	m_fDeactivationTime = pStruct.m_fDeactivationTime;
}

bool GameObject::GetProperty(std::string sName, GenericProp* pProp)
{
	memset(pProp, 0, sizeof(GenericProp));

	PropDef* pPropDef = m_pClassDef->m_Props;
	bool bFound = false;
	int nFoundIndex = -1;

	for (int i = 0; i < m_pClassDef->m_nProps; i++)
	{
		if (sName.compare(pPropDef[i].m_PropName) == 0)
		{
			bFound = true;
			nFoundIndex = i;
			break;
		}
	}

	if (!bFound)
	{
		return false;
	}

	switch (pPropDef[nFoundIndex].m_PropType)
	{
	case PT_BOOL:
		pProp->m_Bool = pPropDef[nFoundIndex].m_DefaultValueFloat;
		break;
	case PT_REAL:
		pProp->m_Float = pPropDef[nFoundIndex].m_DefaultValueFloat;
		break;
	case PT_FLAGS:
		pProp->m_Long = pPropDef[nFoundIndex].m_PropFlags;
		break;
	case PT_LONGINT:
		pProp->m_Long = pPropDef[nFoundIndex].m_DefaultValueFloat;
		break;
	case PT_COLOR:
	case PT_VECTOR:
		pProp->m_Vec = pPropDef[nFoundIndex].m_DefaultValueVector;
		break;
	case PT_ROTATION:
	{
		pProp->m_Vec = pPropDef[nFoundIndex].m_DefaultValueVector;

		godot::Vector3 vVec3 = godot::Vector3(pProp->m_Vec.x, pProp->m_Vec.y, pProp->m_Vec.z);
		godot::Quat qQuat = godot::Quat();
		qQuat.set_euler(vVec3);

		pProp->m_Rotation = DRotation(qQuat.x, qQuat.y, qQuat.z, qQuat.w);
	}
		break;
	case PT_STRING:
		strcpy_s(pProp->m_String, pPropDef[nFoundIndex].m_DefaultValueString);
		break;
	}

	return true;
}

void GameObject::SetFlags(int nFlag)
{
	m_nFlags = nFlag;

	godot::Spatial* pNode = GetNode();

	if (!pNode)
	{
		return;
	}

	if (nFlag & FLAG_VISIBLE)
	{
		pNode->set_visible(true);
	}
	else
	{
		pNode->set_visible(false);
	}
}

void GameObject::SetPosition(DVector vPos)
{
	m_vPos = vPos;

	auto pNode = GetNode();

	if (pNode)
	{
		pNode->set_translation(LT2GodotVec3(vPos));
	}
}

DVector GameObject::GetPosition()
{
	auto pNode = GetNode();

	if (pNode)
	{
		auto vPos = pNode->get_translation();
		return DVector(vPos.x, vPos.y, vPos.z);
	}

	auto vPos = m_vPos;

	return vPos;
}

void GameObject::SetScale(DVector vScale)
{
	m_vScale = vScale;

	auto pNode = GetNode();

	if (pNode)
	{
		pNode->set_scale(LT2GodotVec3(vScale));
	}
}

DVector GameObject::GetScale()
{
	auto pNode = GetNode();

	if (pNode)
	{
		auto vScale = pNode->get_scale();
		return DVector(vScale.x, vScale.y, vScale.z);
	}

	auto vScale = m_vScale;

	return vScale;
}

void GameObject::SetRotation(DRotation qRot)
{
	m_vRotation = qRot;

	auto pNode = GetNode();

	if (pNode)
	{
		auto godotQuat = LT2GodotQuat(&qRot);
		pNode->set_rotation(godotQuat.get_euler());
	}
}

DRotation GameObject::GetRotation()
{
	auto pNode = GetNode();

	if (pNode)
	{
		auto vRot = pNode->get_rotation();
		auto qRot = godot::Quat();
		qRot.set_euler(vRot);
		return DRotation(qRot.x, qRot.y, qRot.z, qRot.w);
	}

	auto qRot = m_vRotation;

	return qRot;
}

godot::Spatial* GameObject::GetNode()
{
	godot::Spatial* pNode = nullptr;

	// Either it's a spatial, or this is a cached result.
	if (m_pNode)
	{
		return m_pNode;
	}

	switch (m_nObjectType)
	{
	case OT_POLYGRID:
		pNode = godot::Object::cast_to<godot::Spatial>(m_pPolyGrid);
		break;
	case OT_CAMERA:
		pNode = godot::Object::cast_to<godot::Spatial>(m_pCamera);
		break;
	case OT_MODEL:
	case OT_NORMAL:
	default:
		pNode = m_pNode;
		break;
	}

	m_pNode = pNode;

	return pNode;
}

void GameObject::Teleport(DVector vNewPos)
{
	m_vPos = vNewPos;

	auto pNode = GetNode();

	if (!pNode)
	{
		return;
	}

	auto vGDPos = LT2GodotVec3(vNewPos);
	pNode->translate(vGDPos);
}
