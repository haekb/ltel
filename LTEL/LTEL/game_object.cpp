#include "game_object.h"

#include <Godot.hpp>
#include <Quat.hpp>
#include <Vector3.hpp>

GameObject::GameObject(ClassDef* pClass, BaseClass* pBaseClass)
{
	//m_pClassDef = new ClassDef();
	//memcpy(m_pClassDef, pClass, sizeof(ClassDef));

	m_pClassDef = pClass;
	m_pBaseClass = pBaseClass;

	m_nObjectType = OT_NORMAL;
	m_nFlags = 0;
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
}

GameObject::~GameObject()
{
}

void GameObject::SetFromObjectCreateStruct(ObjectCreateStruct pStruct)
{
	m_nObjectType = pStruct.m_ObjectType;
	m_nFlags = pStruct.m_Flags;
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

void GameObject::Teleport(DVector vNewPos)
{
	m_vPos = vNewPos;
	// TODO: actually move the godot object
}
