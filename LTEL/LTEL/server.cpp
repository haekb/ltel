#include "server.h"

LTELServer::LTELServer(godot::Node* pGodotLink, HINSTANCE pSRes)
{
	m_pGodotLink = pGodotLink;
	m_pSRes = pSRes;
	m_fFrametime = 0.1f;
	m_pServerShell = nullptr;

	InitFunctionPointers();
}

LTELServer::~LTELServer()
{
	
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
	return HMESSAGEWRITE();
}
