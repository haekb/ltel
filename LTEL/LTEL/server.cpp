#include "server.h"
#include "shared.h"

#include <StreamPeer.hpp>
#include <StreamPeerBuffer.hpp>
#include <File.hpp>

#include "common.h"
#include "common_physics.h"
#include "client_physics.h"

LTELServer* g_pLTELServer = nullptr;

// Backwards compat
ServerDE* g_pServerDE = nullptr;

// I'm not sure where else to throw this, but if we don't have this some macro fails, and it sucks.
__ClassDefiner* __g_ClassDefinerHead = nullptr;

extern std::vector<godot::StreamPeerBuffer*> g_pStreamInUse;


std::vector<godot::StreamPeerBuffer*> g_pQueuedStreams;
//static LTELCommonPhysics g_CommonPhysics;

LTELServer::LTELServer(godot::Node* pGodotLink, HINSTANCE pSRes)
{
	g_pLTELServer = this;
	g_pServerDE = this;
	m_pGodotLink = pGodotLink;
	m_pSRes = pSRes;
	m_fFrametime = 0.1f;
	m_pServerShell = nullptr;
	m_pGameInfo = nullptr;
	m_nFlags = 0;
	m_nClassDefCount = 0;
	m_pClassDefList = nullptr;
	m_dvGlobalForce = DVector(0, -2000, 0);

	m_pCurrentObject = nullptr;

	m_bInWorld = false;

	InitFunctionPointers();

	m_pCommonLT = new LTELCommon();

	m_pPhysicsLT = new LTELCommonPhysics((LTELCommon*)m_pCommonLT);

	// This is dumb
	//m_pPhysicsLT = &g_CommonPhysics;
	//((LTELCommonPhysics*)m_pPhysicsLT)->Init();
}

LTELServer::~LTELServer()
{
	//((LTELCommonPhysics*)m_pPhysicsLT)->Term();

	delete m_pCommonLT;

	delete m_pPhysicsLT;
}

void LTELServer::SetGameInfo(void* pData, int pLen)
{
	if (m_pGameInfo)
	{
		free(m_pGameInfo);
		m_pGameInfo = nullptr;
	}

	m_pGameInfo = malloc(pLen);

	if (m_pGameInfo)
	{
		memcpy(m_pGameInfo, pData, pLen);
	}
}

struct DataVector {
	float x, y, z;
};

struct DataQuat {
	float x, y, z, w;
};

struct WorldProperty {
	std::string sName;
	int nType;

	union data {
		bool bBool;
		int nInt;
		float fFloat;
		DataVector vVector;
		DataQuat qQuat;
		short nStringLength;
	};
};

struct WorldObject {
	std::string sName;
	int nProperties;
	
};

void LTELServer::StartWorld(std::string sWorldName)
{

	m_pServerShell->PreStartWorld(true);

	// Temp world
	if (0) 
	{
		godot::Godot::print("Looking for placeholders...");
		auto pPlaceholders = g_pLTELServer->m_pGodotLink->get_node("/root/Scene/Placeholders");

		auto pChildren = pPlaceholders->get_children();
		godot::Godot::print("Processing {0} placeholders!", pChildren.size());
		for (int i = 0; i < pChildren.size(); i++)
		{
			auto pChild = GDCAST(godot::Spatial, pChildren[i]);

			godot::Vector3 vPos = pChild->get_translation();
			godot::Vector3 vRot = pChild->get_rotation();
			godot::Vector3 vScale = pChild->get_scale();

			godot::Quat qRot = godot::Quat();
			qRot.set_euler(vRot);

			godot::String sObjName = pChild->get("obj_name");
			godot::String sFileName = pChild->get("file_name");
			godot::String sSkinName = pChild->get("skin_name");
			godot::String sBaseClassName = pChild->get("base_class_name");

			ObjectCreateStruct ocs = { 0 };
			strcpy_s(ocs.m_Name, 100, sObjName.alloc_c_string());
			strcpy_s(ocs.m_Filename, 100, sFileName.alloc_c_string());
			strcpy_s(ocs.m_SkinName, 100, sSkinName.alloc_c_string());
			ocs.m_ObjectType = pChild->get("type");
			ocs.m_Pos = DVector(vPos.x, vPos.y, vPos.z);
			ocs.m_Rotation = DRotation(qRot.x, qRot.y, qRot.z, qRot.w);
			ocs.m_Scale = DVector(vScale.x, vScale.y, vScale.z);
			ocs.m_NextUpdate = pChild->get("next_update");
			ocs.m_fDeactivationTime = pChild->get("deactivation_time");
			ocs.m_Flags = (unsigned int)pChild->get("flags");
			ocs.m_ContainerCode = (unsigned int)pChild->get("container_code");

			auto pClass = g_pLTELServer->GetClass(sBaseClassName.alloc_c_string());

			if (!pClass)
			{
				godot::Godot::print("Failed to create placeholder");
				continue;
			}

			auto pBaseClass = g_pLTELServer->CreateObject(pClass, &ocs);
			auto pObj = (GameObject*)pBaseClass->m_hObject;
			
		}
	}
	else {

	}

}

bool LTELServer::ReceiveMessageFromClient(ClientInfo* pClientInfo, godot::StreamPeerBuffer* pStream, DDWORD flags)
{
	// No server shell, no message!
	if (!m_pServerShell)
	{
		return false;
	}

	pStream->seek(0);

	DBYTE pMessageId = pStream->get_8();

	m_pServerShell->OnMessage((HCLIENT)pClientInfo, pMessageId, (HMESSAGEREAD)pStream);

	return true;
}

void LTELServer::Update(DFLOAT timeElapsed)
{
	if (!m_bInWorld)
	{
		m_fFrametime = 0.0f;
		return;
	}

	m_fFrametime = timeElapsed;
	m_fTime += timeElapsed;

	// Clear any items queued for deletion
	std::vector<GameObject*> vTemp;

	for (auto pObj : m_pObjectList)
	{
		if (!pObj->IsQueuedForDeletion())
		{
			vTemp.push_back(pObj);
			continue;
		}

		if (pObj == g_pLTELServer->m_pCurrentObject)
		{
			g_pLTELServer->m_pCurrentObject = nullptr;
		}

		delete pObj;
	}

	int nTestIndex = 0;
	int nSize = vTemp.size();
	for (auto pObj : vTemp)
	{
		// We always run the engine-side update!
		pObj->Update(timeElapsed);

		// Game dll update stuff:

		float fNextUpdate = pObj->GetNextUpdate();
		fNextUpdate -= timeElapsed;
		
		if (fNextUpdate > 0.001f)
		{
			nTestIndex++;
			pObj->SetNextUpdate(fNextUpdate);
			continue;
		}

		pObj->SetNextUpdate(0.0f);

		auto pClass = pObj->GetBaseClass();
		if (!pClass)
		{
			nTestIndex++;
			continue;
		}

		pObj->GetClassDef()->m_EngineMessageFn(pClass, MID_UPDATE, nullptr, timeElapsed);
		nTestIndex++;
	}

	m_pObjectList.clear();
	m_pObjectList = vTemp;
}

void LTELServer::HandleMessageQueue()
{
	for (auto pStream : g_pQueuedStreams)
	{
		auto pClientShell = (CClientShellDE*)m_pClientList[0]->GetClientShell();

		DBYTE pMessageId = pStream->get_8();

		if (pMessageId == MID_SFX_MSG || pMessageId == MID_SFX_INSTANT_MSG)
		{
			auto guid = shared_ReadFromMessageGUID((HMESSAGEREAD)pStream);
			GameObject* pObj = FindObjectByGUID(guid);

			if (pMessageId == MID_SFX_INSTANT_MSG)
			{
				// Pass over the vector
				DVector vTmp;
				shared_ReadFromMessageVector((HMESSAGEREAD)pStream, &vTmp);
				bool bTest = true;
			}

			pClientShell->SpecialEffectNotify((HLOCALOBJ)pObj, (HMESSAGEREAD)pStream);
		}
		else if (pMessageId == MID_SEND_TO_OBJ)
		{
			auto guidTo = shared_ReadFromMessageGUID((HMESSAGEREAD)pStream);
			auto guidFrom = shared_ReadFromMessageGUID((HMESSAGEREAD)pStream);
			auto nObjMessageId = shared_ReadFromMessageDWord((HMESSAGEREAD)pStream);

			GameObject* bTo = FindObjectByGUID(guidTo);
			GameObject* bFrom = FindObjectByGUID(guidFrom);

			if (!bTo /*|| !bFrom*/ || !bTo->GetBaseClass())
			{
				godot::Godot::print("[HandleMessageQueue] Missing base class!!");
			}
			else
			{
				bTo->GetBaseClass()->_ObjectMsgFn(bTo->GetBaseClass(), (HOBJECT)bFrom, nObjMessageId, (HMESSAGEREAD)pStream);
			}

		}
		else
		{
			pClientShell->OnMessage(pMessageId, (HMESSAGEREAD)pStream);

		}

		shared_CleanupStream(pStream);

		// Ok clean it up!
		pStream->free();
	}

	g_pQueuedStreams.clear();
}

ObjectList* LTELServer::CreateObjectListFromVector(std::vector<GameObject*> pList)
{
	auto pObjectList = new ObjectList();

	// Previous in the loop, technically the next pointer in the list,
	// since we loop backwards
	ObjectLink* pPreviousObjectLink = nullptr;
	for (auto it = pList.crbegin(); it != pList.crend(); it++)
	{
		// Retrieve our object, and create a new object link
		auto pObject = *it;
		auto pObjectLink = new ObjectLink();

		// Assign the object, and the next pointer in line (at the start it will be null!)
		pObjectLink->m_hObject = (HOBJECT)pObject;
		pObjectLink->m_pNext = pPreviousObjectLink;

		// Increment our counter
		pObjectList->m_nInList++;

		// Assign our current pointer as the previous (next in the list)
		pPreviousObjectLink = pObjectLink;
	}

	// And now we're at the beginning of the list, so assign the last used pointer as our first link!
	pObjectList->m_pFirstLink = pPreviousObjectLink;

	return pObjectList;
}

GameObject* LTELServer::FindObjectByGUID(GUID guid)
{
	// FIXME: This is probably slow!
	for (auto pObj : m_pObjectList)
	{
		if (pObj->GetID() == guid)
		{
			return pObj;
		}
	}

	return nullptr;
}


DRESULT LTELServer::GetGlobalForce(DVector* pVec)
{
	*pVec = m_dvGlobalForce;

	return DE_OK;
}

DRESULT LTELServer::SetGlobalForce(DVector* pVec)
{
	m_dvGlobalForce = *pVec;

	return DE_OK;
}

HMESSAGEWRITE LTELServer::StartSpecialEffectMessage(LPBASECLASS pObject)
{
	if (!pObject->m_hObject)
	{
		return nullptr;
	}

	auto pStream = (godot::StreamPeerBuffer*)StartHMessageWrite();

	// Try just using the id
	GameObject* pObj = (GameObject*)pObject->m_hObject;
	shared_WriteToMessageByte((HMESSAGEWRITE)pStream, MID_SFX_MSG);
	shared_WriteToMessageGUID((HMESSAGEWRITE)pStream, pObj->GetID());

	return (HMESSAGEWRITE)pStream;
}

HMESSAGEWRITE LTELServer::StartInstantSpecialEffectMessage(DVector* pPos)
{
	// Maybe it's to clients?
	GameObject* pObj = (GameObject*)m_pClientList[0]->GetObj();//m_pCurrentObject;

	// Try the current object?
	//GameObject* pObj = (GameObject*)m_pCurrentObject;

	if (!pObj)
	{
		return nullptr;
	}

	auto pStream = (godot::StreamPeerBuffer*)StartHMessageWrite();

	shared_WriteToMessageByte((HMESSAGEWRITE)pStream, MID_SFX_INSTANT_MSG);
	shared_WriteToMessageGUID((HMESSAGEWRITE)pStream, pObj->GetID());
	WriteToMessageVector((HMESSAGEWRITE)pStream, pPos);

	return (HMESSAGEWRITE)pStream;
}

HMESSAGEWRITE LTELServer::StartMessageToObject(LPBASECLASS pSender, HOBJECT hSendTo, DDWORD messageID)
{
	if (!pSender || !pSender->m_hObject || !hSendTo)
	{
		return nullptr;
	}

	auto pStream = (godot::StreamPeerBuffer*)StartHMessageWrite();

	// Try just using the id
	GameObject* pObj = (GameObject*)pSender->m_hObject;
	GameObject* pSendTo = (GameObject*)hSendTo;
	shared_WriteToMessageByte((HMESSAGEWRITE)pStream, MID_SEND_TO_OBJ);
	shared_WriteToMessageGUID((HMESSAGEWRITE)pStream, pSendTo->GetID()); // To
	shared_WriteToMessageGUID((HMESSAGEWRITE)pStream, pObj->GetID()); // From
	shared_WriteToMessageDWord((HMESSAGEWRITE)pStream, messageID); // Subject

	return (HMESSAGEWRITE)pStream;
}

DRESULT LTELServer::StartMessageToServer(LPBASECLASS pSender, DDWORD messageID, HMESSAGEWRITE* hWrite)
{
	return DE_ERROR;
}

HMESSAGEWRITE LTELServer::StartMessage(HCLIENT hSendTo, DBYTE messageID)
{
	auto pStream = (godot::StreamPeerBuffer*)StartHMessageWrite();
	pStream->put_8(messageID);

	return (HMESSAGEWRITE)pStream;
}

DRESULT LTELServer::EndMessage(HMESSAGEWRITE hMessage)
{
	return EndMessage2(hMessage, MESSAGE_GUARANTEED);
}

DRESULT LTELServer::EndMessage2(HMESSAGEWRITE hMessage, DDWORD flags)
{
	if (m_pClientList.size() == 0)
	{
		// Clean up...
		auto pStream = (godot::StreamPeerBuffer*)hMessage;
		pStream->free();

		return DE_SERVERERROR;
	}

	godot::StreamPeerBuffer* pStream = (godot::StreamPeerBuffer*)hMessage;

	g_pQueuedStreams.push_back(pStream);
	pStream->seek(0);

	return DE_OK;

	// Old immediate handling
#if 0
	DBYTE pMessageId = pStream->get_8();

	auto pClientShell = (CClientShellDE*)m_pClientList[0]->GetClientShell();

	pClientShell->OnMessage(pMessageId, hMessage);

	shared_CleanupStream((godot::StreamPeerBuffer*)hMessage);

	// Ok clean it up!
	pStream->free();

	return DE_OK;
#endif
}

DRESULT LTELServer::SetObjectSFXMessage(HOBJECT hObject, LMessage& msg)
{
	return DE_ERROR;
}

DRESULT LTELServer::SendToObject(LMessage& msg, DDWORD msgID, HOBJECT hSender, HOBJECT hSendTo, DDWORD flags)
{
	return DE_ERROR;
}

DRESULT LTELServer::SendToServer(LMessage& msg, DDWORD msgID, HOBJECT hSender, DDWORD flags)
{
	return DE_ERROR;
}

DRESULT LTELServer::SendToClient(LMessage& msg, DBYTE msgID, HCLIENT hSendTo, DDWORD flags)
{
	return DE_ERROR;
}

DRESULT LTELServer::SendSFXMessage(LMessage& msg, DVector& pos, DDWORD flags)
{
	return DE_ERROR;
}

DRESULT LTELServer::GetClientPing(HCLIENT hClient, float& ping)
{
	return DE_ERROR;
}

DRESULT LTELServer::SetupEuler(DRotation* pRotation, float pitch, float yaw, float roll)
{
	return m_pCommonLT->SetupEuler(*pRotation, pitch, yaw, roll);
}

float LTELServer::GetObjectMass(HOBJECT hObj)
{
	if (!hObj)
	{
		return 1.0f;
	}

	GameObject* pObj = (GameObject*)hObj;
	return pObj->GetMass();
}

void LTELServer::SetObjectMass(HOBJECT hObj, float mass)
{
	if (!hObj)
	{
		return;
	}

	GameObject* pObj = (GameObject*)hObj;
	pObj->SetMass(mass);
}

float LTELServer::GetForceIgnoreLimit(HOBJECT hObj, float& limit)
{
	return 0.0f;
}

void LTELServer::SetForceIgnoreLimit(HOBJECT hObj, float limit)
{
}

DRESULT LTELServer::GetStandingOn(HOBJECT hObj, CollisionInfo* pInfo)
{
	return DE_ERROR;
}

void LTELServer::GetObjectDims(HOBJECT hObj, DVector* pNewDims)
{
	DVector pDims = DVector(0.0f, 0.0f, 0.0f);
	*pNewDims = pDims;
}

DRESULT LTELServer::SetObjectDims(HOBJECT hObj, DVector* pNewDims)
{
	return m_pPhysicsLT->SetObjectDims(hObj, pNewDims, 0);
}

DRESULT LTELServer::SetObjectDims2(HOBJECT hObj, DVector* pNewDims)
{
	return m_pPhysicsLT->SetObjectDims(hObj, pNewDims, 0);
}

DRESULT LTELServer::GetVelocity(HOBJECT hObj, DVector* pVel)
{
	if (!hObj)
	{
		*pVel = DVector(0, 0, 0);
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;

	*pVel = pObj->GetVelocity();

	return DE_OK;
}

DRESULT LTELServer::SetVelocity(HOBJECT hObj, DVector* pVel)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;

	pObj->SetVelocity(*pVel);

	return DE_OK;
}

DRESULT LTELServer::GetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	if (!hObj)
	{
		*pAccel = DVector(0, 0, 0);
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;

	*pAccel = pObj->GetAccel();

	return DE_OK;
}

DRESULT LTELServer::SetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;

	pObj->SetAccel(*pAccel);

	return DE_OK;
}

DRESULT LTELServer::GetModelAnimUserDims(HOBJECT hObj, DVector* pDims, HMODELANIM hAnim)
{
	return Common()->GetModelAnimUserDims(hObj, pDims, hAnim);
}

HOBJECT LTELServer::GetWorldObject()
{
	return (HOBJECT)g_pLTELServer->m_pWorldObject;
}

DRESULT LTELServer::GetWorldBox(DVector& min, DVector& max)
{
	min = DVector(-1000000, -1000000, -1000000);
	max = DVector(1000000, 1000000, 1000000);

	return DE_OK;
}

DRESULT LTELServer::GetRotationVectors(DRotation* pRotation, DVector* pUp, DVector* pRight, DVector* pForward)
{
	return m_pCommonLT->GetRotationVectors(*pRotation, *pUp, *pRight, *pForward);
}

DRESULT LTELServer::SetFrictionCoefficient(HOBJECT hObj, float coeff)
{
	return DE_ERROR;
}

DRESULT LTELServer::MoveObject(HOBJECT hObj, DVector* pNewPos)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;

	pObj->SetPosition(*pNewPos, true);

	return DE_OK;
}

DRESULT LTELServer::GetNetFlags(HOBJECT hObj, DDWORD& flags)
{
	return DE_ERROR;
}

DRESULT LTELServer::SetNetFlags(HOBJECT hObj, DDWORD flags)
{
	return DE_ERROR;
}

DRESULT LTELServer::GetPolyTextureFlags(HPOLY hPoly, DDWORD* pFlags)
{
	*pFlags = 0;// ST_UNKNOWN

	int nFlag = (int)hPoly;

	if (hPoly != INVALID_HPOLY)
	{
		*pFlags = hPoly;
	}

	return DE_OK;
}

HMESSAGEWRITE LTELServer::StartHMessageWrite()
{
	return shared_StartHMessageWrite();
}

DRESULT LTELServer::WriteToMessageFloat(HMESSAGEWRITE hMessage, float val)
{
	return shared_WriteToMessageFloat(hMessage, val);
}

DRESULT LTELServer::WriteToMessageByte(HMESSAGEWRITE hMessage, DBYTE val)
{
	return shared_WriteToMessageByte(hMessage, val);
}

DRESULT LTELServer::WriteToMessageWord(HMESSAGEWRITE hMessage, D_WORD val)
{
	return shared_WriteToMessageWord(hMessage, val);
}

DRESULT LTELServer::WriteToMessageDWord(HMESSAGEWRITE hMessage, DDWORD val)
{
	return shared_WriteToMessageDWord(hMessage, val);
}

DRESULT LTELServer::WriteToMessageString(HMESSAGEWRITE hMessage, char* pStr)
{
	return shared_WriteToMessageString(hMessage, pStr);
}

DRESULT LTELServer::WriteToMessageVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageVector(hMessage, pVal);
}

DRESULT LTELServer::WriteToMessageCompVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageCompVector(hMessage, pVal);
}

DRESULT LTELServer::WriteToMessageCompPosition(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageCompPosition(hMessage, pVal);
}

DRESULT LTELServer::WriteToMessageRotation(HMESSAGEWRITE hMessage, DRotation* pVal)
{
	return shared_WriteToMessageRotation(hMessage, pVal);
}

DRESULT LTELServer::WriteToMessageHString(HMESSAGEWRITE hMessage, HSTRING hString)
{
	return shared_WriteToMessageHString(hMessage, hString);
}

DRESULT LTELServer::WriteToMessageHMessageWrite(HMESSAGEWRITE hMessage, HMESSAGEWRITE hDataMessage)
{
	return shared_WriteToMessageHMessageWrite(hMessage, hDataMessage);
}

DRESULT LTELServer::WriteToMessageHMessageRead(HMESSAGEWRITE hMessage, HMESSAGEREAD hDataMessage)
{
	return shared_WriteToMessageHMessageRead(hMessage, hDataMessage);
}

// Not used in Shogo!
DRESULT LTELServer::WriteToMessageFormattedHString(HMESSAGEWRITE hMessage, int messageCode, ...)
{
	return DE_ERROR;
}

DRESULT LTELServer::WriteToMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObj)
{
	return shared_WriteToMessageObject(hMessage, hObj);
}

DRESULT LTELServer::WriteToLoadSaveMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObject)
{
	return shared_WriteToLoadSaveMessageObject(hMessage, hObject);
}

float LTELServer::ReadFromMessageFloat(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageFloat(hMessage);
}

DBYTE LTELServer::ReadFromMessageByte(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageByte(hMessage);
}

D_WORD LTELServer::ReadFromMessageWord(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageWord(hMessage);
}

DDWORD LTELServer::ReadFromMessageDWord(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageDWord(hMessage);
}

char* LTELServer::ReadFromMessageString(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageString(hMessage);
}

void LTELServer::ReadFromMessageVector(HMESSAGEREAD hMessage, DVector* pVal)
{
	return shared_ReadFromMessageVector(hMessage, pVal);
}

void LTELServer::ReadFromMessageCompVector(HMESSAGEREAD hMessage, DVector* pVal)
{
	return shared_ReadFromMessageCompVector(hMessage, pVal);
}

void LTELServer::ReadFromMessageCompPosition(HMESSAGEREAD hMessage, DVector* pVal)
{
	return shared_ReadFromMessageCompPosition(hMessage, pVal);
}

void LTELServer::ReadFromMessageRotation(HMESSAGEREAD hMessage, DRotation* pVal)
{
	return shared_ReadFromMessageRotation(hMessage, pVal);
}

HOBJECT LTELServer::ReadFromMessageObject(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageObject(hMessage);
}

HSTRING LTELServer::ReadFromMessageHString(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageHString(hMessage);
}

DRESULT LTELServer::ReadFromLoadSaveMessageObject(HMESSAGEREAD hMessage, HOBJECT* hObject)
{
	return shared_ReadFromLoadSaveMessageObject(hMessage, hObject);
}

HMESSAGEREAD LTELServer::ReadFromMessageHMessageRead(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageHMessageRead(hMessage);
}

void LTELServer::EndHMessageRead(HMESSAGEREAD hMessage)
{
	shared_EndHMessageRead(hMessage);
}

void LTELServer::EndHMessageWrite(HMESSAGEWRITE hMessage)
{
	shared_EndHMessageWrite(hMessage);
}

void LTELServer::ResetRead(HMESSAGEREAD hRead)
{
	shared_ResetRead(hRead);
}
