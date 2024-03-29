#include "client.h"
#include "game_object.h"
#include "helpers.h"

#include <Godot.hpp>
#include <Quat.hpp>
#include <Vector3.hpp>
#include <Shape.hpp>
#include <BoxShape.hpp>

extern LTELClient* g_pLTELClient;

#define _DEBUG_POS_ROT

#ifdef _DEBUG_POS_ROT
#include <RichTextLabel.hpp>
godot::RichTextLabel* g_pPosLabel = nullptr;
godot::RichTextLabel* g_pRotLabel = nullptr;

void UpdatePos(DVector vPos)
{
	if (!g_pPosLabel)
	{
		return;
	}

	godot::String sString = "Player Position: <{0}, {1}, {2}>";
	godot::Array aValues;
	aValues.push_back(vPos.x);
	aValues.push_back(vPos.y);
	aValues.push_back(vPos.z);
	

	g_pPosLabel->set_text(sString.format(aValues));
}

void UpdateRot(DRotation vRot)
{
	if (!g_pRotLabel)
	{
		return;
	}

	godot::String sString = "Player Rotation: <{0}, {1}, {2}, {3}>";
	godot::Array aValues;
	aValues.push_back(vRot.m_Vec.x);
	aValues.push_back(vRot.m_Vec.y);
	aValues.push_back(vRot.m_Vec.z);
	aValues.push_back(vRot.m_Spin);

	g_pRotLabel->set_text(sString.format(aValues));
}

#endif

//
// Game Object - There's an object for everyone and everything!
// Basically our translation layer between Lithtech and Godot's node systems.
// Note: Server-side objects will not have instances of Nodes!
//
GameObject::GameObject(ClassDef* pClass, BaseClass* pBaseClass)
{
	m_pClassDef = pClass;
	m_pBaseClass = pBaseClass;

	m_nObjectType = OT_NORMAL;
	m_nFlags = 0;
	m_nUserFlags = 0;
	m_nClientFlags = 0;
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
	m_fPhysicsDeltaTime = 0.1f;

	m_nState = 0;

	m_pContainer = nullptr;
	m_pNode = nullptr;
	m_pCamera = nullptr;
	m_pPolyGrid = nullptr;
	m_pKinematicBody = nullptr;
	m_pLastCollision = nullptr;

	m_pExtraData = nullptr;
	m_pServerObject = nullptr; // Needed??

	m_fFrictionCoeff = 1.0f;
	m_fForceIgnoreLimit = 0.0f;
	m_vVelocity = DVector(0,0,0);
	m_vAccel = DVector(0,0,0);
	m_fMass = 0.0f;
	m_vDims = DVector(0,0,0);

	m_bQueuedForDeletion = false;
	m_bNotifyOnTouch = false;
	m_bNotifyOnModelKey = false;
	m_bClientNotifyOnModelKey = false;
	m_bDontSetDims = false;
	m_bInitialDimsSet = false;

	// Create an id for this fella
	// I'm currently too lazy to implement some type of unique id system
	CoCreateGuid(&m_gID);
}

GameObject::~GameObject()
{
	auto pBaseClass = GetBaseClass();
	auto pClassDef = GetClassDef();

	// Camera is a global object, we do not delete it!
	if (m_nObjectType == OT_CAMERA)
	{
		return;
	}

	if (m_pBaseClass)
	{
		free(m_pBaseClass);
		m_pBaseClass = nullptr;

		// Hacky, but for now don't delete any client-side objects, i'll make a flag later.
		return;
	}

	if (IsType(OT_SPRITE))
	{
		auto pContainer = GetNode()->get_parent();
		// Free the container too!
		//pContainer->queue_free();
	}

	if (IsType(OT_POLYGRID) && m_pPolyGrid)
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

	if (IsType(OT_MODEL))
	{
		// Just handle the extra data
		delete m_pExtraData;
		m_pExtraData = nullptr;
	}

	if (GetKinematicBody())
	{
		GetKinematicBody()->queue_free();

		// Kinematic has its own container too!
		auto pContainer = GetKinematicBody()->get_parent();
		pContainer->queue_free();

	}

	if (m_pNode)
	{
		m_pNode->queue_free();
		m_pNode = nullptr;
	}

	if (m_pContainer)
	{
		m_pContainer->queue_free();
		m_pContainer = nullptr;
	}

	// Don't delete the class def, it's not ours

}

void GameObject::Update(float fDelta)
{
	if (!IsType(OT_MODEL))
	{
		return;
	}
}

void on_animation_command_string(godot::String sCommandString)
{
	godot::Godot::print(sCommandString);
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

	//if (m_nObjectType == OT_CAMERA && GetNode())
	if (m_sName == "Sanjuro" && GetNode())
	{
		g_pPosLabel = GDCAST(godot::RichTextLabel, GetNode()->get_node("/root/Scene/Debug2D/PlayerPOS"));
		g_pRotLabel = GDCAST(godot::RichTextLabel, GetNode()->get_node("/root/Scene/Debug2D/PlayerROT"));
	}

	if (GetNode())
	{
		if (!m_sName.empty())
		{
			GetNode()->set_name(m_sName.c_str());
		}
		else if (!m_sFilename.empty())
		{
			GetNode()->set_name(m_sFilename.c_str());
		}
	}

	if (IsType(OT_MODEL))
	{
		// Connect this signal, so we can send a MID_MODELSTRINGKEY
		//GetNode()->connect("animation_command_string", nullptr, "on_animation_command_string");
	}

	// If we don't have these guys, then it's a client-side object
	if (!GetBaseClass() && !GetClassDef())
	{
		m_pServerObject = (GameObject*)g_pLTELClient->GetClientObject();
	}
}

void GameObject::CreateKinematicBody()
{
	if (m_sName == "Sanjuro")
	{
		return;
	}

	if (!GetNode())
	{
		return;
	}

	NodeLinker* pContainer = NodeLinker::_new();

	godot::String sContainerName = GetName().c_str();
	sContainerName += " - CONTAINER";
	pContainer->set_name(sContainerName);
	// We're setting the game object to this game object!
	pContainer->SetGameObject(this);

	godot::KinematicBody* pKinematicBody = godot::KinematicBody::_new();

	godot::Ref<godot::BoxShape> pBox = godot::BoxShape::_new();
	godot::CollisionShape* pShape = godot::CollisionShape::_new();
	pShape->set_shape(pBox);

	pKinematicBody->add_child(pShape);

	auto nOwnerID = pKinematicBody->create_shape_owner(pShape);
	pKinematicBody->shape_owner_add_shape(nOwnerID, pBox);

	auto sName = GetNode()->get_name();
	sName += " - ClientBody";
	pKinematicBody->set_name(sName);

	auto p3D = GetNode()->get_node("/root/Scene/3D");
	p3D->add_child(pContainer);
	pContainer->add_child(pKinematicBody);
	SetKinematicBody(pKinematicBody);
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
		pProp->m_Vec = DVector(vVec3.x, vVec3.y, vVec3.z);
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

	if (IsType(OT_MODEL))
	{
		LTELModel* pExtraData = (LTELModel*)GetExtraData();
		
		if (nFlag & FLAG_SHADOW)
		{
			pExtraData->pMesh->set_cast_shadows_setting(godot::MeshInstance::SHADOW_CASTING_SETTING_ON);
		}
		else
		{
			pExtraData->pMesh->set_cast_shadows_setting(godot::MeshInstance::SHADOW_CASTING_SETTING_OFF);
		}

		if (nFlag & FLAG_SOLID)
		{
			if (!GetKinematicBody())
			{
				CreateKinematicBody();
			}
			else
			{
				GetKinematicBody()->set_visible(true);
			}
		}
		else
		{
			if (GetKinematicBody())
			{
				GetKinematicBody()->set_visible(false);
				//GetKinematicBody()->queue_free();
			}
		}

		m_bApplyGravity = nFlag & FLAG_GRAVITY;
		m_bNotifyOnTouch = nFlag & FLAG_TOUCH_NOTIFY;
		m_bNotifyOnModelKey = nFlag & FLAG_MODELKEYS;
	}
}

void GameObject::SetClientFlags(int nFlag)
{
	m_bClientNotifyOnModelKey = nFlag & CF_NOTIFYMODELKEYS;
	m_bDontSetDims = nFlag & CF_DONTSETDIMS;

	m_nClientFlags = nFlag;
}

void GameObject::SetPosition(DVector vPos, bool bLocalUpdate)
{
	m_vPos = vPos;

	if (bLocalUpdate)
	{
		return;
	}

	auto pNode = GetNode();
	auto vPosition = LT2GodotVec3(vPos);

	if (pNode)
	{
		pNode->set_translation(vPosition);

		if (m_sName == "Sanjuro" && GetNode())
		{
			UpdatePos(vPos);
		}
		
	}

	if (GetKinematicBody())
	{
		GetKinematicBody()->set_translation(vPosition);
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
	if (IsType(OT_SPRITE))
	{
		vScale *= 100.0f;
	}

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
	auto godotQuat = LT2GodotQuat(&qRot);
	if (pNode)
	{
		pNode->set_rotation(godotQuat.get_euler());

		if (m_sName == "Sanjuro" && GetNode())
		{
			UpdateRot(qRot);
		}
	}

	if (m_pKinematicBody)
	{
		auto vEuler = godotQuat.get_euler();
		vEuler.x = 0.0f;

		m_pKinematicBody->set_rotation(vEuler);
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


void GameObject::SetKinematicBody(godot::KinematicBody* pBody)
{
	// 
	m_pKinematicBody = pBody;

	// Reset dims
	SetDims(GetDims());
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

void GameObject::SetDims(DVector vVal)
{
	if (m_bInitialDimsSet && m_bDontSetDims)
	{
		//return;
	}

	m_vDims = vVal;
	
	if (GetKinematicBody())
	{
		// Set the proper dims
		godot::Ref<godot::BoxShape> pShape = m_pKinematicBody->shape_owner_get_shape(0, 0);
		pShape->set_extents(LT2GodotVec3(GetDims()));

		m_bInitialDimsSet = true;
	}
}

bool GameObject::AddAttachment(LTELAttachment* pObj)
{
	if (!pObj)
	{
		return false;
	}

	m_pAttachments.push_back(pObj);

	return true;
}

bool GameObject::RemoveAttachment(LTELAttachment* pObj)
{
	if (!pObj)
	{
		return false;
	}

	std::vector<LTELAttachment*> vTemp;
	for (auto pAttachment : m_pAttachments)
	{
		if (pAttachment != pObj)
		{
			vTemp.push_back(pAttachment);
		}
	}

	m_pAttachments.clear();
	m_pAttachments = vTemp;

	return true;
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


#include "client.h"
extern LTELClient* g_pLTELClient;
void GameObject::SendAnimationCommandString(godot::String sCommandString)
{
	// If we don't have the flag
	if (!NotifyOnModelKey() && !NotifyClientOnModelKey())
	{
		return;
	}

	ArgList argList;
	argList.argc = 1;

	//char szString[PARSE_MAXTOKENSIZE];
	//strcpy_s(szString, PARSE_MAXTOKENSIZE, sCommandString.alloc_c_string());
	char* szCommands[PARSE_MAXTOKENSIZE];
	memset(szCommands, 0, sizeof(char*) * PARSE_MAXTOKENSIZE);

	szCommands[0] = sCommandString.alloc_c_string();
	//strcpy_s(szCommands[0], PARSE_MAXTOKENSIZE, sCommandString.alloc_c_string());

	argList.argv = szCommands;

	if (NotifyOnModelKey() && m_pBaseClass)
	{
		m_pBaseClass->_EngineMsgFn(m_pBaseClass, MID_MODELSTRINGKEY, &argList, 0.0f);
	}

	if (NotifyClientOnModelKey())
	{
		CClientShellDE* pClientShell = (CClientShellDE*)g_pLTELClient->GetClientShell();
		pClientShell->OnModelKey((HLOCALOBJ)this, &argList);
	}
}
