#include "shared.h"

// For ez casting
#define GD_STREAM_CAST(x) (godot::StreamPeerBuffer*) x

bool shared_SetStreamValue(godot::StreamPeerBuffer* pStream, godot::Variant pValue)
{
	pStream->put_var(pValue, true);
	return true;
}

HMESSAGEWRITE	shared_StartHMessageWrite()
{
	godot::StreamPeerBuffer* pStream = godot::StreamPeerBuffer::_new();

	return (HMESSAGEWRITE)pStream;
}

DRESULT shared_WriteToMessageFloat(HMESSAGEWRITE hMessage, float val)
{
	auto bVal = shared_SetStreamValue(GD_STREAM_CAST(hMessage), val);
	return bVal ? DE_OK : DE_ERROR;
}

DRESULT shared_WriteToMessageByte(HMESSAGEWRITE hMessage, DBYTE val)
{
	auto bVal = shared_SetStreamValue(GD_STREAM_CAST(hMessage), val);
	return bVal ? DE_OK : DE_ERROR;
}

DRESULT shared_WriteToMessageWord(HMESSAGEWRITE hMessage, D_WORD val)
{
	auto bVal = shared_SetStreamValue(GD_STREAM_CAST(hMessage), val);
	return bVal ? DE_OK : DE_ERROR;
}

DRESULT shared_WriteToMessageDWord(HMESSAGEWRITE hMessage, DDWORD val)
{
	auto bVal = shared_SetStreamValue(GD_STREAM_CAST(hMessage), (unsigned int)val);
	return bVal ? DE_OK : DE_ERROR;
}

DRESULT shared_WriteToMessageString(HMESSAGEWRITE hMessage, char* pStr)
{
	godot::String pString = pStr;
	auto bVal = shared_SetStreamValue(GD_STREAM_CAST(hMessage), pString);
	return bVal ? DE_OK : DE_ERROR;
}

DRESULT shared_WriteToMessageVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	godot::Vector3 pVector = godot::Vector3(pVal->x, pVal->y, pVal->z);
	auto bVal = shared_SetStreamValue(GD_STREAM_CAST(hMessage), pVector);
	return bVal ? DE_OK : DE_ERROR;
}

DRESULT shared_WriteToMessageCompVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageVector(hMessage, pVal);
}

DRESULT shared_WriteToMessageCompPosition(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageVector(hMessage, pVal);
}

DRESULT shared_WriteToMessageRotation(HMESSAGEWRITE hMessage, DRotation* pVal)
{
	godot::Quat pQuat = godot::Quat(pVal->m_Vec.x, pVal->m_Vec.y, pVal->m_Vec.z, pVal->m_Spin);
	auto bVal = shared_SetStreamValue(GD_STREAM_CAST(hMessage), pQuat);
	return bVal ? DE_OK : DE_ERROR;
}

DRESULT shared_WriteToMessageHString(HMESSAGEWRITE hMessage, HSTRING hString)
{
	LTELString* pString = (LTELString*)hString;

	return shared_WriteToMessageString(hMessage, (char*)pString->sData.c_str());
}

DRESULT shared_WriteToMessageHMessageWrite(HMESSAGEWRITE hMessage, HMESSAGEWRITE hDataMessage)
{
	godot::StreamPeerBuffer* pDestStream = GD_STREAM_CAST(hMessage);
	godot::StreamPeerBuffer* pSrcStream = GD_STREAM_CAST(hDataMessage);

	return pDestStream->put_data(pSrcStream->get_data(pSrcStream->get_size())) == godot::Error::OK ? DE_OK : DE_ERROR;
}

DRESULT shared_WriteToMessageHMessageRead(HMESSAGEWRITE hMessage, HMESSAGEREAD hDataMessage)
{
	return shared_WriteToMessageHMessageWrite(hMessage, hDataMessage);
}

DRESULT shared_WriteToMessageFormattedHString(HMESSAGEWRITE hMessage, int messageCode, ...)
{
	// Not implemented in shogo!
	return DE_ERROR;
}

DRESULT shared_WriteToMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObj)
{
	LTELObject* pObject = (LTELObject*)hObj;
	auto pStream = GD_STREAM_CAST(hMessage);

	shared_SetStreamValue(pStream, pObject->nObjectType);
	shared_SetStreamValue(pStream, pObject->nObjectFlags);
	shared_SetStreamValue(pStream, pObject->nUserFlags);

	// Serialize the node!
	switch (pObject->nObjectType)
	{
	case OT_POLYGRID:
	{
		shared_SetStreamValue(pStream, pObject->pData.pPolyGrid);
		LTELPolyGrid* pExtraData = (LTELPolyGrid*)pObject->pExtraData;

		// Extra data!
		shared_SetStreamValue(pStream, pExtraData->bLocked);
		shared_SetStreamValue(pStream, pExtraData->pData);
		shared_SetStreamValue(pStream, pExtraData->nWidth);
		shared_SetStreamValue(pStream, pExtraData->nHeight);

		// We don't send over the ImageTexture refs, they'll be re-init on read.

		// End!
	}
		break;
	case OT_CAMERA:
		shared_SetStreamValue(pStream, pObject->pData.pCamera);

		break;
	default:
		shared_SetStreamValue(pStream, pObject->pData.pNode);

	}

	return DE_OK;
}

DRESULT shared_WriteToLoadSaveMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObject)
{
	return shared_WriteToMessageObject(hMessage, hObject);
}

float shared_ReadFromMessageFloat(HMESSAGEREAD hMessage)
{
	return 0.0f;
}

DBYTE shared_ReadFromMessageByte(HMESSAGEREAD hMessage)
{
	return DBYTE();
}

D_WORD shared_ReadFromMessageWord(HMESSAGEREAD hMessage)
{
	return D_WORD();
}

DDWORD shared_ReadFromMessageDWord(HMESSAGEREAD hMessage)
{
	return DDWORD();
}

char* shared_ReadFromMessageString(HMESSAGEREAD hMessage)
{
	return nullptr;
}

void shared_ReadFromMessageVector(HMESSAGEREAD hMessage, DVector* pVal)
{
}

void shared_ReadFromMessageCompVector(HMESSAGEREAD hMessage, DVector* pVal)
{
}

void shared_ReadFromMessageCompPosition(HMESSAGEREAD hMessage, DVector* pVal)
{
}

void shared_ReadFromMessageRotation(HMESSAGEREAD hMessage, DRotation* pVal)
{
}

HOBJECT shared_ReadFromMessageObject(HMESSAGEREAD hMessage)
{
	return HOBJECT();
}

HSTRING shared_ReadFromMessageHString(HMESSAGEREAD hMessage)
{
	return HSTRING();
}

DRESULT shared_ReadFromLoadSaveMessageObject(HMESSAGEREAD hMessage, HOBJECT* hObject)
{
	return DRESULT();
}

HMESSAGEREAD shared_ReadFromMessageHMessageRead(HMESSAGEREAD hMessage)
{
	return HMESSAGEREAD();
}

void shared_EndHMessageRead(HMESSAGEREAD hMessage)
{
}

void shared_EndHMessageWrite(HMESSAGEWRITE hMessage)
{
}

void shared_ResetRead(HMESSAGEREAD hRead)
{
}
