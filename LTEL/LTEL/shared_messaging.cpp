#include "shared.h"

// For ez casting
#define GD_STREAM_CAST(x) (godot::StreamPeerBuffer*) x

std::vector<godot::StreamPeerBuffer*> g_pStreamInUse;

// Guarenteed to be magic by our resident wizards
#define MAGIC_NUMBER 4528

// Only for ReadObject...
extern LTELServer* g_pLTELServer;

void shared_CleanupStream(godot::StreamPeerBuffer* pStream)
{

	std::vector<godot::StreamPeerBuffer*> vTemp;

	for (auto pBuffer : g_pStreamInUse)
	{
		if (pStream != pBuffer)
		{
			vTemp.push_back(pStream);
		}
	}

	g_pStreamInUse.clear();
	g_pStreamInUse = vTemp;
}

bool shared_SetStreamValue(godot::StreamPeerBuffer* pStream, godot::Variant pValue)
{
	pStream->put_var(pValue, false);
	return true;
}

HMESSAGEWRITE	shared_StartHMessageWrite()
{
	godot::StreamPeerBuffer* pStream = godot::StreamPeerBuffer::_new();

	g_pStreamInUse.push_back(pStream);

	return (HMESSAGEWRITE)pStream;
}

DRESULT shared_WriteToMessageFloat(HMESSAGEWRITE hMessage, float val)
{
	(GD_STREAM_CAST(hMessage))->put_float(val);
	return DE_OK;
}

DRESULT shared_WriteToMessageByte(HMESSAGEWRITE hMessage, DBYTE val)
{
	(GD_STREAM_CAST(hMessage))->put_8(val);
	return DE_OK;
}

DRESULT shared_WriteToMessageWord(HMESSAGEWRITE hMessage, D_WORD val)
{
	(GD_STREAM_CAST(hMessage))->put_16(val);
	return DE_OK;
}

DRESULT shared_WriteToMessageDWord(HMESSAGEWRITE hMessage, DDWORD val)
{
	(GD_STREAM_CAST(hMessage))->put_32(val);
	return DE_OK;
}

DRESULT shared_WriteToMessageString(HMESSAGEWRITE hMessage, char* pStr)
{
	godot::String pString = pStr;
	(GD_STREAM_CAST(hMessage))->put_string(pString);
	return DE_OK;
}

DRESULT shared_WriteToMessageVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	auto pStream = GD_STREAM_CAST(hMessage);

	pStream->put_float(pVal->x);
	pStream->put_float(pVal->y);
	pStream->put_float(pVal->z);

	bool bVal = true;
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
	auto pStream = GD_STREAM_CAST(hMessage);

	pStream->put_float(pVal->m_Vec.x);
	pStream->put_float(pVal->m_Vec.y);
	pStream->put_float(pVal->m_Vec.z);
	pStream->put_float(pVal->m_Spin);

	bool bVal = true;
	return bVal ? DE_OK : DE_ERROR;
}

DRESULT shared_WriteToMessageHString(HMESSAGEWRITE hMessage, HSTRING hString)
{
	LTELString* pString = (LTELString*)hString;

	if (!pString)
	{
		// Make sure to put something!
		(GD_STREAM_CAST(hMessage))->put_string("");
		return DE_OK;
	}

	godot::String pGDString = pString->sData.c_str();
	(GD_STREAM_CAST(hMessage))->put_string(pGDString);

	return DE_OK;
}

DRESULT shared_WriteToMessageGUID(HMESSAGEWRITE hMessage, GUID guid)
{
	auto pStream = GD_STREAM_CAST(hMessage);
	pStream->put_32(guid.Data1);
	pStream->put_16(guid.Data2);
	pStream->put_16(guid.Data3);

	auto nBytesSize = sizeof(GUID::Data4) / sizeof(GUID::Data4[0]);

	for (int i = 0; i < nBytesSize; i++)
	{
		pStream->put_8(guid.Data4[i]);
	}

	return DE_OK;
}

DRESULT shared_WriteToMessageHMessageWrite(HMESSAGEWRITE hMessage, HMESSAGEWRITE hDataMessage)
{
	godot::StreamPeerBuffer* pDestStream = GD_STREAM_CAST(hMessage);
	godot::StreamPeerBuffer* pSrcStream = GD_STREAM_CAST(hDataMessage);

	if (!pSrcStream)
	{
		return DE_OK;
	}

	auto pData = pSrcStream->get_data_array();
	auto nSize = pData.size();

	// Put the size before our data blob,
	// so we can later extract the blob!
	pDestStream->put_32(nSize);
	auto nOk = pDestStream->put_data(pData);

#if _DEBUG
	// For later testing...
	pDestStream->put_64(MAGIC_NUMBER);
#endif

	return nOk == godot::Error::OK ? DE_OK : DE_ERROR;
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
	GameObject* pObject = (GameObject*)hObj;
	auto pStream = GD_STREAM_CAST(hMessage);

	if (!pObject)
	{
		// No GUID here
		shared_WriteToMessageByte(hMessage, 0);

		return DE_ERROR;
	}

	// GUID is written!
	shared_WriteToMessageByte(hMessage, 1);
	shared_WriteToMessageGUID(hMessage, pObject->GetID());

	return DE_OK;
}

DRESULT shared_WriteToLoadSaveMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObject)
{
	return shared_WriteToMessageObject(hMessage, hObject);
}

void shared_CheckAndReset(HMESSAGEREAD hMessage)
{
	auto pStream = GD_STREAM_CAST(hMessage);
	auto nPos = pStream->get_position();
	auto nSize = pStream->get_size();

	if (nPos >= nSize)
	{
		pStream->seek(0);
		pStream->get_8(); // Skip message id
	}
}

float shared_ReadFromMessageFloat(HMESSAGEREAD hMessage)
{
	auto fVal = (GD_STREAM_CAST(hMessage))->get_float();
	shared_CheckAndReset(hMessage);
	return fVal;
}

DBYTE shared_ReadFromMessageByte(HMESSAGEREAD hMessage)
{
	auto bVal = (GD_STREAM_CAST(hMessage))->get_8();
	shared_CheckAndReset(hMessage);
	return bVal;
}

D_WORD shared_ReadFromMessageWord(HMESSAGEREAD hMessage)
{
	auto nVal = (GD_STREAM_CAST(hMessage))->get_16();
	shared_CheckAndReset(hMessage);
	return nVal;
}

DDWORD shared_ReadFromMessageDWord(HMESSAGEREAD hMessage)
{
	auto nVal = (GD_STREAM_CAST(hMessage))->get_32();
	shared_CheckAndReset(hMessage);
	return nVal;
}

char* shared_ReadFromMessageString(HMESSAGEREAD hMessage)
{
	auto szVal = (GD_STREAM_CAST(hMessage))->get_string();

	auto nlen = szVal.length();

	shared_CheckAndReset(hMessage);
	return szVal.alloc_c_string();
}

void shared_ReadFromMessageVector(HMESSAGEREAD hMessage, DVector* pVal)
{
	auto pStream = (GD_STREAM_CAST(hMessage));

	float x = pStream->get_float();
	float y = pStream->get_float();
	float z = pStream->get_float();

	DVector dVec = DVector(x,y,z);
	*pVal = dVec;
	shared_CheckAndReset(hMessage);
}

void shared_ReadFromMessageCompVector(HMESSAGEREAD hMessage, DVector* pVal)
{
	shared_ReadFromMessageVector(hMessage, pVal);
}

void shared_ReadFromMessageCompPosition(HMESSAGEREAD hMessage, DVector* pVal)
{
	shared_ReadFromMessageVector(hMessage, pVal);
}

void shared_ReadFromMessageRotation(HMESSAGEREAD hMessage, DRotation* pVal)
{
	auto pStream = (GD_STREAM_CAST(hMessage));

	float x = pStream->get_float();
	float y = pStream->get_float();
	float z = pStream->get_float();
	float w = pStream->get_float();

	DRotation dRot = DRotation(x,y,z,w);
	*pVal = dRot;
	shared_CheckAndReset(hMessage);
}

HOBJECT shared_ReadFromMessageObject(HMESSAGEREAD hMessage)
{
	auto nHasValidGUID = shared_ReadFromMessageByte(hMessage);

	if (nHasValidGUID == 0)
	{
		return nullptr;
	}

	auto guid = shared_ReadFromMessageGUID(hMessage);

	// FIXME: This will have to change later on, but so far Shogo only uses this function to read on the server
	return (HOBJECT)g_pLTELServer->FindObjectByGUID(guid);
}

HSTRING shared_ReadFromMessageHString(HMESSAGEREAD hMessage)
{
	LTELString* pString = new LTELString();

	pString->sData = shared_ReadFromMessageString(hMessage);
	shared_CheckAndReset(hMessage);
	return (HSTRING)pString;
}

GUID shared_ReadFromMessageGUID(HMESSAGEREAD hMessage)
{
	auto pStream = GD_STREAM_CAST(hMessage);

	GUID guid;

	guid.Data1 = pStream->get_32();
	guid.Data2 = pStream->get_16();
	guid.Data3 = pStream->get_16();

	auto nBytesSize = sizeof(GUID::Data4) / sizeof(GUID::Data4[0]);

	for (int i = 0; i < nBytesSize; i++)
	{
		guid.Data4[i] = pStream->get_8();
	}

	return guid;
}

DRESULT shared_ReadFromLoadSaveMessageObject(HMESSAGEREAD hMessage, HOBJECT* hObject)
{
	auto pObj = shared_ReadFromMessageObject(hMessage);

	if (!pObj)
	{
		return DE_ERROR;
	}

	*hObject = pObj;
	shared_CheckAndReset(hMessage);
	return DE_OK;
}

HMESSAGEREAD shared_ReadFromMessageHMessageRead(HMESSAGEREAD hMessage)
{
	auto pStream = GD_STREAM_CAST(hMessage);

	// Start our new stream!
	auto pNewStream = GD_STREAM_CAST(shared_StartHMessageWrite());

	// Get the size of the data blob
	int nSize = pStream->get_32();
	// Get the data blob, this function returns an array of two values,
	// Error code, and the actual data...
	auto pArr = pStream->get_data(nSize);

#if _DEBUG
	// If we don't get our magic value, then we did not actually get the right amount of data
	auto nMagicNumber = pStream->get_64();

	if (nMagicNumber != MAGIC_NUMBER)
	{
		godot::Godot::print("[shared_ReadFromMessageHMessageRead] ERROR! MAGIC NUMBER WAS NOT FOUND!!");
	}
#endif

	// Retrieve the error code, and the data
	int ErrorCode = pArr.pop_front();
	godot::PoolByteArray pData = pArr.pop_back();
	
	// Insert the data, and reset the stream position
	pNewStream->put_data(pData);
	pNewStream->seek(0);

	shared_CheckAndReset(hMessage);
	return (HMESSAGEREAD)pNewStream;
}

void shared_EndHMessageRead(HMESSAGEREAD hMessage)
{
	shared_CleanupStream(GD_STREAM_CAST(hMessage));
	(GD_STREAM_CAST(hMessage))->free();
}

void shared_EndHMessageWrite(HMESSAGEWRITE hMessage)
{
	shared_CleanupStream(GD_STREAM_CAST(hMessage));
	(GD_STREAM_CAST(hMessage))->free();
}

void shared_ResetRead(HMESSAGEREAD hRead)
{
	(GD_STREAM_CAST(hRead))->seek(0);
	(GD_STREAM_CAST(hRead))->get_8(); // Skip message id
}
