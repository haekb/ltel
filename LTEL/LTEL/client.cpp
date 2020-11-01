
#include "client.h"

LTELClient* g_pLTELClient = nullptr;

LTELClient::LTELClient(godot::Node* pGodotLink)
{
	g_pLTELClient = this;

	m_pGodotLink = pGodotLink;
	InitFunctionPointers();
}

LTELClient::~LTELClient()
{
}



//
// STUB FUNCTIONS
// If you wish to implement functions please place it in their own sectioned off .cpp file!
//

DRESULT LTELClient::GetPointStatus(DVector* pPoint)
{
	return DRESULT();
}

DRESULT LTELClient::GetPointShade(DVector* pPoint, DVector* pColor)
{
	return DRESULT();
}

DRESULT LTELClient::OpenFile(char* pFilename, DStream** pStream)
{
	return DRESULT();
}

DRESULT LTELClient::GetSConValueFloat(char* pName, float& val)
{
	return DRESULT();
}

DRESULT LTELClient::GetSConValueString(char* pName, char* valBuf, DDWORD bufLen)
{
	return DRESULT();
}

float LTELClient::GetServerConVarValueFloat(char* pName)
{
	return 0.0f;
}

char* LTELClient::GetServerConVarValueString(char* pName)
{
	return nullptr;
}

DRESULT LTELClient::SetupEuler(DRotation* pRotation, float pitch, float yaw, float roll)
{
	return DRESULT();
}

DRESULT LTELClient::GetRotationVectors(DRotation* pRotation, DVector* pUp, DVector* pRight, DVector* pForward)
{
	return DRESULT();
}

HMESSAGEWRITE LTELClient::StartMessage(DBYTE messageID)
{
	return HMESSAGEWRITE();
}

DRESULT LTELClient::EndMessage(HMESSAGEWRITE hMessage)
{
	return DRESULT();
}

DRESULT LTELClient::EndMessage2(HMESSAGEWRITE hMessage, DDWORD flags)
{
	return DRESULT();
}

DRESULT LTELClient::SendToServer(LMessage& msg, DBYTE msgID, DDWORD flags)
{
	return DRESULT();
}

DRESULT LTELClient::ProcessAttachments(HOBJECT hObj)
{
	return DRESULT();
}

DEParticle* LTELClient::AddParticle(HLOCALOBJ hObj, DVector* pPos, DVector* pVelocity, DVector* pColor, float lifeTime)
{
	return nullptr;
}

DRESULT LTELClient::GetSpriteControl(HLOCALOBJ hObj, SpriteControl*& pControl)
{
	return DRESULT();
}

DRESULT LTELClient::StartQuery(char* pInfo)
{
	return DRESULT();
}

DRESULT LTELClient::UpdateQuery()
{
	return DRESULT();
}

DRESULT LTELClient::GetQueryResults(NetSession*& pListHead)
{
	return DRESULT();
}

DRESULT LTELClient::EndQuery()
{
	return DRESULT();
}

HMESSAGEWRITE LTELClient::StartHMessageWrite()
{
	return HMESSAGEWRITE();
}

DRESULT CSBase::WriteToMessageFloat(HMESSAGEWRITE hMessage, float val)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageByte(HMESSAGEWRITE hMessage, DBYTE val)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageWord(HMESSAGEWRITE hMessage, D_WORD val)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageDWord(HMESSAGEWRITE hMessage, DDWORD val)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageString(HMESSAGEWRITE hMessage, char* pStr)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageCompVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageCompPosition(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageRotation(HMESSAGEWRITE hMessage, DRotation* pVal)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageHString(HMESSAGEWRITE hMessage, HSTRING hString)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageHMessageWrite(HMESSAGEWRITE hMessage, HMESSAGEWRITE hDataMessage)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageHMessageRead(HMESSAGEWRITE hMessage, HMESSAGEREAD hDataMessage)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageFormattedHString(HMESSAGEWRITE hMessage, int messageCode, ...)
{
	return DRESULT();
}

DRESULT CSBase::WriteToMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObj)
{
	return DRESULT();
}

DRESULT CSBase::WriteToLoadSaveMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObject)
{
	return DRESULT();
}

float CSBase::ReadFromMessageFloat(HMESSAGEREAD hMessage)
{
	return 0.0f;
}

DBYTE CSBase::ReadFromMessageByte(HMESSAGEREAD hMessage)
{
	return DBYTE();
}

D_WORD CSBase::ReadFromMessageWord(HMESSAGEREAD hMessage)
{
	return D_WORD();
}

DDWORD CSBase::ReadFromMessageDWord(HMESSAGEREAD hMessage)
{
	return DDWORD();
}

char* CSBase::ReadFromMessageString(HMESSAGEREAD hMessage)
{
	return nullptr;
}

void CSBase::ReadFromMessageVector(HMESSAGEREAD hMessage, DVector* pVal)
{
}

void CSBase::ReadFromMessageCompVector(HMESSAGEREAD hMessage, DVector* pVal)
{
}

void CSBase::ReadFromMessageCompPosition(HMESSAGEREAD hMessage, DVector* pVal)
{
}

void CSBase::ReadFromMessageRotation(HMESSAGEREAD hMessage, DRotation* pVal)
{
}

HOBJECT CSBase::ReadFromMessageObject(HMESSAGEREAD hMessage)
{
	return HOBJECT();
}

HSTRING CSBase::ReadFromMessageHString(HMESSAGEREAD hMessage)
{
	return HSTRING();
}

DRESULT CSBase::ReadFromLoadSaveMessageObject(HMESSAGEREAD hMessage, HOBJECT* hObject)
{
	return DRESULT();
}

HMESSAGEREAD CSBase::ReadFromMessageHMessageRead(HMESSAGEREAD hMessage)
{
	return HMESSAGEREAD();
}

void CSBase::EndHMessageRead(HMESSAGEREAD hMessage)
{
}

void CSBase::EndHMessageWrite(HMESSAGEWRITE hMessage)
{
}

void CSBase::ResetRead(HMESSAGEREAD hRead)
{
}
