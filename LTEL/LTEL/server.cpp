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

	m_pCurrentObject = nullptr;

	m_bInWorld = false;

	InitFunctionPointers();

	m_pCommonLT = new LTELCommon();
	m_pPhysicsLT = new LTELCommonPhysics();
}

LTELServer::~LTELServer()
{
	
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

	// This is probably wrong
	for (auto pClient : m_pClientList)
	{
		if (!pClient->GetClientShell())
		{
			continue;
		}

		CClientShellDE* pClientShell = (CClientShellDE*)pClient->GetClientShell();

		pClientShell->OnEnterWorld();
	}


	auto pClass = m_pServerShell->OnClientEnterWorld((HCLIENT)m_pClientList[0], m_pClientList[0], sizeof(m_pClientList[0]));

	auto pGameObj = new GameObject(nullptr, pClass);

	// Setup the client object
	m_pClientList[0]->SetObj(pGameObj);

	m_pServerShell->PostStartWorld();

	m_bInWorld = true;
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

	for (auto pObj : m_pObjectList)
	{
		float fNextUpdate = pObj->GetNextUpdate();
		fNextUpdate -= timeElapsed;
		
		if (fNextUpdate > 0.001f)
		{
			pObj->SetNextUpdate(fNextUpdate);
			continue;
		}

		pObj->SetNextUpdate(0.0f);

		auto pClass = pObj->GetBaseClass();
		if (!pClass)
		{
			continue;
		}

		pObj->GetClassDef()->m_EngineMessageFn(pClass, MID_UPDATE, nullptr, timeElapsed);
	}
}


DRESULT LTELServer::GetGlobalForce(DVector* pVec)
{
	return DE_ERROR;
}

DRESULT LTELServer::SetGlobalForce(DVector* pVec)
{
	return DE_ERROR;
}

HMESSAGEWRITE LTELServer::StartSpecialEffectMessage(LPBASECLASS pObject)
{
	return nullptr;
}

HMESSAGEWRITE LTELServer::StartInstantSpecialEffectMessage(DVector* pPos)
{
	return nullptr;
}

HMESSAGEWRITE LTELServer::StartMessageToObject(LPBASECLASS pSender, HOBJECT hSendTo, DDWORD messageID)
{
	return nullptr;
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

	pStream->seek(0);
	DBYTE pMessageId = pStream->get_8();

	auto pClientShell = (CClientShellDE*)m_pClientList[0]->GetClientShell();

	pClientShell->OnMessage(pMessageId, hMessage);

	shared_CleanupStream((godot::StreamPeerBuffer*)hMessage);

	// Ok clean it up!
	pStream->free();

	return DE_OK;
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
	return 0.0f;
}

void LTELServer::SetObjectMass(HOBJECT hObj, float mass)
{
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
	return DE_ERROR;
}

DRESULT LTELServer::SetObjectDims2(HOBJECT hObj, DVector* pNewDims)
{
	return DE_ERROR;
}

DRESULT LTELServer::GetVelocity(HOBJECT hObj, DVector* pVel)
{
	return DE_ERROR;
}

DRESULT LTELServer::SetVelocity(HOBJECT hObj, DVector* pVel)
{
	return DE_ERROR;
}

DRESULT LTELServer::GetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	return DE_ERROR;
}

DRESULT LTELServer::SetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	return DE_ERROR;
}

DRESULT LTELServer::GetModelAnimUserDims(HOBJECT hObj, DVector* pDims, HMODELANIM hAnim)
{
	return DE_ERROR;
}

HOBJECT LTELServer::GetWorldObject()
{
	return nullptr;
}

DRESULT LTELServer::GetWorldBox(DVector& min, DVector& max)
{
	return DE_ERROR;
}

DRESULT LTELServer::GetRotationVectors(DRotation* pRotation, DVector* pUp, DVector* pRight, DVector* pForward)
{
	return DE_ERROR;
}

DRESULT LTELServer::SetFrictionCoefficient(HOBJECT hObj, float coeff)
{
	return DE_ERROR;
}

DRESULT LTELServer::MoveObject(HOBJECT hObj, DVector* pNewPos)
{
	return DE_ERROR;
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
	return DE_ERROR;
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
