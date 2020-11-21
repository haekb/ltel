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
	m_vPos = DVector();
	m_vScale = DVector();
	m_vRotation = DRotation();
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
	m_vPos = pStruct.m_Pos;
	m_vScale = pStruct.m_Scale;
	m_vRotation = pStruct.m_Rotation;
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
