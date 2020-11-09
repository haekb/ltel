#include "server.h"
#include "shared.h"

#include <StreamPeer.hpp>
#include <StreamPeerBuffer.hpp>

LTELServer* g_pLTELServer = nullptr;

LTELServer::LTELServer(godot::Node* pGodotLink, HINSTANCE pSRes)
{
	g_pLTELServer = this;
	m_pGodotLink = pGodotLink;
	m_pSRes = pSRes;
	m_fFrametime = 0.1f;
	m_pServerShell = nullptr;
	m_pGameInfo = nullptr;

	m_nClassDefCount = 0;
	m_pClassDefList = nullptr;

	InitFunctionPointers();
}

LTELServer::~LTELServer()
{
	
}

void LTELServer::SetGameInfo(void* pData, int pLen)
{
	if (m_pGameInfo)
	{
		free(m_pGameInfo);
	}

	m_pGameInfo = malloc(pLen);

	if (m_pGameInfo)
	{
		memcpy(m_pGameInfo, pData, pLen);
	}
}

void LTELServer::StartWorld(std::string sWorldName)
{

	m_pServerShell->PreStartWorld(true);

	// Do stuff here...

	m_pServerShell->PostStartWorld();


}

bool LTELServer::ReceiveMessageFromClient(godot::StreamPeerBuffer* pStream, DDWORD flags)
{
	// No server shell, no message!
	if (!m_pServerShell)
	{
		return false;
	}

	pStream->seek(0);

	DBYTE pMessageId = pStream->get_8();

	m_pServerShell->OnMessage(nullptr, pMessageId, (HMESSAGEREAD)pStream);

	// Ok clean it up!
	pStream->free();
}


DRESULT LTELServer::GetGlobalForce(DVector* pVec)
{
	return DE_OK;
}

DRESULT LTELServer::SetGlobalForce(DVector* pVec)
{
	return DRESULT();
}

HMESSAGEWRITE LTELServer::StartSpecialEffectMessage(LPBASECLASS pObject)
{
	return HMESSAGEWRITE();
}

HMESSAGEWRITE LTELServer::StartInstantSpecialEffectMessage(DVector* pPos)
{
	return HMESSAGEWRITE();
}

HMESSAGEWRITE LTELServer::StartMessageToObject(LPBASECLASS pSender, HOBJECT hSendTo, DDWORD messageID)
{
	return HMESSAGEWRITE();
}

DRESULT LTELServer::StartMessageToServer(LPBASECLASS pSender, DDWORD messageID, HMESSAGEWRITE* hWrite)
{
	return DRESULT();
}

HMESSAGEWRITE LTELServer::StartMessage(HCLIENT hSendTo, DBYTE messageID)
{
	return HMESSAGEWRITE();
}

DRESULT LTELServer::EndMessage2(HMESSAGEWRITE hMessage, DDWORD flags)
{
	return DRESULT();
}

DRESULT LTELServer::EndMessage(HMESSAGEWRITE hMessage)
{
	return DRESULT();
}

DRESULT LTELServer::SetObjectSFXMessage(HOBJECT hObject, LMessage& msg)
{
	return DRESULT();
}

DRESULT LTELServer::SendToObject(LMessage& msg, DDWORD msgID, HOBJECT hSender, HOBJECT hSendTo, DDWORD flags)
{
	return DRESULT();
}

DRESULT LTELServer::SendToServer(LMessage& msg, DDWORD msgID, HOBJECT hSender, DDWORD flags)
{
	return DRESULT();
}

DRESULT LTELServer::SendToClient(LMessage& msg, DBYTE msgID, HCLIENT hSendTo, DDWORD flags)
{
	return DRESULT();
}

DRESULT LTELServer::SendSFXMessage(LMessage& msg, DVector& pos, DDWORD flags)
{
	return DRESULT();
}

DRESULT LTELServer::GetClientPing(HCLIENT hClient, float& ping)
{
	return DRESULT();
}

DRESULT LTELServer::SetupEuler(DRotation* pRotation, float pitch, float yaw, float roll)
{
	return DRESULT();
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
	return DRESULT();
}

void LTELServer::GetObjectDims(HOBJECT hObj, DVector* pNewDims)
{
}

DRESULT LTELServer::SetObjectDims(HOBJECT hObj, DVector* pNewDims)
{
	return DRESULT();
}

DRESULT LTELServer::SetObjectDims2(HOBJECT hObj, DVector* pNewDims)
{
	return DRESULT();
}

DRESULT LTELServer::GetVelocity(HOBJECT hObj, DVector* pVel)
{
	return DRESULT();
}

DRESULT LTELServer::SetVelocity(HOBJECT hObj, DVector* pVel)
{
	return DRESULT();
}

DRESULT LTELServer::GetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	return DRESULT();
}

DRESULT LTELServer::SetAcceleration(HOBJECT hObj, DVector* pAccel)
{
	return DRESULT();
}

DRESULT LTELServer::GetModelAnimUserDims(HOBJECT hObj, DVector* pDims, HMODELANIM hAnim)
{
	return DRESULT();
}

HOBJECT LTELServer::GetWorldObject()
{
	return HOBJECT();
}

DRESULT LTELServer::GetWorldBox(DVector& min, DVector& max)
{
	return DRESULT();
}

DRESULT LTELServer::GetRotationVectors(DRotation* pRotation, DVector* pUp, DVector* pRight, DVector* pForward)
{
	return DRESULT();
}

DRESULT LTELServer::SetFrictionCoefficient(HOBJECT hObj, float coeff)
{
	return DRESULT();
}

DRESULT LTELServer::MoveObject(HOBJECT hObj, DVector* pNewPos)
{
	return DRESULT();
}

DRESULT LTELServer::GetNetFlags(HOBJECT hObj, DDWORD& flags)
{
	return DRESULT();
}

DRESULT LTELServer::SetNetFlags(HOBJECT hObj, DDWORD flags)
{
	return DRESULT();
}

DRESULT LTELServer::GetPolyTextureFlags(HPOLY hPoly, DDWORD* pFlags)
{
	return DRESULT();
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
