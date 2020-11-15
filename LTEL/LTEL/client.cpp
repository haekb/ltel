
#include "client.h"
#include "shared.h"
#include "helpers.h"


#include <Basis.hpp>
#include <ResourceLoader.hpp>
#include <Script.hpp>
#include <ClassDB.hpp>
#include <Reference.hpp>

LTELClient* g_pLTELClient = nullptr;

LTELClient::LTELClient(godot::Node* pGodotLink, HINSTANCE pCRes)
{
	g_pLTELClient = this;
	m_pLTELServer = nullptr;

	m_pClientInfo = nullptr;

	m_nGameMode = GAMEMODE_NONE;
	m_bIsConnected = false;
	m_sGameDataDir = "";
	m_vFOV = godot::Vector2(0.0f, 0.0f);
	m_fFrametime = 0.1f;
	m_pCRes = pCRes;
	m_pGodotLink = pGodotLink;
	InitFunctionPointers();

	// New!
	InitRenderImpl();
	InitObjectImpl();
	InitStringImpl();
}

LTELClient::~LTELClient()
{
}


bool LTELClient::StartServerDLL(StartGameRequest* pRequest)
{
	typedef int f_GetServerShellVersion();
	typedef void f_GetServerShellFunctions(CreateServerShellFn* pCreate, DeleteServerShellFn* pDelete);
	typedef ClassDef** f_ObjectDLLSetup(int* nDefs, ServerDE* pServer, int* version);

	// CWD is the project folder...
	HINSTANCE hObjectLTO = LoadLibraryA("./bin/Object.lto");

	if (!hObjectLTO)
	{
		godot::Godot::print("Could not locate Object.lto!");
		return false;
	}

	HINSTANCE hSRes = LoadLibraryA("./bin/SRes.dll");

	if (!hSRes)
	{
		godot::Godot::print("Could not locate SRes.dll!");
		return false;
	}

	// Initial test, print out the version!
	f_GetServerShellVersion* pGetVersion = (f_GetServerShellVersion*)GetProcAddress(hObjectLTO, "GetServerShellVersion");

	if (!pGetVersion)
	{
		godot::Godot::print("Could not obtain Proc GetClientShellVersion!");
		return false;
	}

	// Print the version!
	int nVersion = pGetVersion();
	godot::Godot::print("Object.lto version: {0}", nVersion);

	if (nVersion != 3)
	{
		godot::Godot::print("Object interface version is not 3!");
		return false;
	}

	// Okay now the real fun begins x2!
	// we'll need to kick off our server impl in object.lto

	f_GetServerShellFunctions* pServerShellInitFunc = (f_GetServerShellFunctions*)GetProcAddress(hObjectLTO, "GetServerShellFunctions");

	if (!pServerShellInitFunc)
	{
		godot::Godot::print("Could not obtain Proc GetServerShellFunctions!");
		return false;
	}

	//
	int pnCreate = 0;
	int pnDelete = 0;

	pServerShellInitFunc((CreateServerShellFn*)&pnCreate, (DeleteServerShellFn*)&pnDelete);

	if (!pnCreate)
	{
		godot::Godot::print("Could not obtain CreateServerShellFn!");

		return false;
	}

	m_pLTELServer = new LTELServer(m_pGodotLink, hSRes);

	// Add a client, because we need one!
	m_pClientInfo = new ClientInfo(true, "Shogoer", this);
	m_pLTELServer->m_pClientList.push_back(m_pClientInfo);

	// We'll want to run CreateServerShellFn, to get the game's ObjectLTO instance
	CreateServerShellFn pCreate = (CreateServerShellFn)pnCreate;
	auto pServerShell = (CServerShellDE*)pCreate(m_pLTELServer);

	m_pLTELServer->m_pServerShell = pServerShell;

	// Ship over some important settings...
	m_pLTELServer->SetGameInfo(pRequest->m_pGameInfo, pRequest->m_GameInfoLen);

	// Need to run ObjectDLLSetup to get Class definitions!!
	f_ObjectDLLSetup* pObjectDLLSetup = (f_ObjectDLLSetup*)GetProcAddress(hObjectLTO, "ObjectDLLSetup");

	int nClassDefCount = 0;
	int nObjectDefVersion = 0;

	auto pClassList = pObjectDLLSetup(&nClassDefCount, m_pLTELServer, &nObjectDefVersion);

	godot::Godot::print("ObjectDLLSetup version: {0}", nObjectDefVersion);
	godot::Godot::print("Found {0} class definitions!", nClassDefCount);

	if (nObjectDefVersion != 1)
	{
		godot::Godot::print("Object class definition version is not 1!");
		return false;
	}

	m_pLTELServer->m_nClassDefCount = nClassDefCount;
	m_pLTELServer->m_pClassDefList = pClassList;

	// Maybe kick off the world stuff?
	//m_pLTELServer->StartWorld(pRequest->m_WorldName);

	return true;
}


godot::Ref<godot::ImageTexture> LTELClient::LoadPCX(std::string sPath)
{
	auto pResourceLoader = godot::ResourceLoader::get_singleton();
	auto pClassDB = godot::ClassDB::get_singleton();

	auto pNode = g_pLTELClient->m_pGodotLink->get_node("/root/Scene/Scripts/LoadPCX");

	auto bDoIHas = pNode->has_method("load_image");

	godot::Ref<godot::ImageTexture> pTexture = pNode->call("load_image", sPath.c_str());

	return pTexture;
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
	godot::Quat vQuat = LT2GodotQuat(pRotation);

	godot::Basis vBasis = godot::Basis(vQuat);

	godot::Vector3 vForward = -vBasis.z;
	godot::Vector3 vRight = vBasis.x;
	godot::Vector3 vUp = vBasis.y;

	*pForward = DVector(vForward.x, vForward.y, vForward.z);
	*pRight = DVector(vRight.x, vRight.y, vRight.z);
	*pUp = DVector(vUp.x, vUp.y, vUp.z);

	return DE_OK;
}

HMESSAGEWRITE LTELClient::StartMessage(DBYTE messageID)
{
	auto pStream = (godot::StreamPeerBuffer*)StartHMessageWrite();
	pStream->put_8(messageID);

	return (HMESSAGEWRITE)pStream;
}

DRESULT LTELClient::EndMessage(HMESSAGEWRITE hMessage)
{
	return EndMessage2(hMessage, MESSAGE_GUARANTEED);
}

DRESULT LTELClient::EndMessage2(HMESSAGEWRITE hMessage, DDWORD flags)
{
	if (!m_pLTELServer)
	{
		// Clean up...
		auto pStream = (godot::StreamPeerBuffer*)hMessage;
		pStream->free();

		return DE_SERVERERROR;
	}

	m_pLTELServer->ReceiveMessageFromClient(m_pClientInfo, (godot::StreamPeerBuffer*)hMessage, flags);

	return DE_OK;
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
	return shared_StartHMessageWrite();
}

DRESULT LTELClient::WriteToMessageFloat(HMESSAGEWRITE hMessage, float val)
{
	return shared_WriteToMessageFloat(hMessage, val);
}

DRESULT LTELClient::WriteToMessageByte(HMESSAGEWRITE hMessage, DBYTE val)
{
	return shared_WriteToMessageByte(hMessage, val);
}

DRESULT LTELClient::WriteToMessageWord(HMESSAGEWRITE hMessage, D_WORD val)
{
	return shared_WriteToMessageWord(hMessage, val);
}

DRESULT LTELClient::WriteToMessageDWord(HMESSAGEWRITE hMessage, DDWORD val)
{
	return shared_WriteToMessageDWord(hMessage, val);
}

DRESULT LTELClient::WriteToMessageString(HMESSAGEWRITE hMessage, char* pStr)
{
	return shared_WriteToMessageString(hMessage, pStr);
}

DRESULT LTELClient::WriteToMessageVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageVector(hMessage, pVal);
}

DRESULT LTELClient::WriteToMessageCompVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageCompVector(hMessage, pVal);
}

DRESULT LTELClient::WriteToMessageCompPosition(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageCompPosition(hMessage, pVal);
}

DRESULT LTELClient::WriteToMessageRotation(HMESSAGEWRITE hMessage, DRotation* pVal)
{
	return shared_WriteToMessageRotation(hMessage, pVal);
}

DRESULT LTELClient::WriteToMessageHString(HMESSAGEWRITE hMessage, HSTRING hString)
{
	return shared_WriteToMessageHString(hMessage, hString);
}

DRESULT LTELClient::WriteToMessageHMessageWrite(HMESSAGEWRITE hMessage, HMESSAGEWRITE hDataMessage)
{
	return shared_WriteToMessageHMessageWrite(hMessage, hDataMessage);
}

DRESULT LTELClient::WriteToMessageHMessageRead(HMESSAGEWRITE hMessage, HMESSAGEREAD hDataMessage)
{
	return shared_WriteToMessageHMessageRead(hMessage, hDataMessage);
}

// Not used in Shogo!
DRESULT LTELClient::WriteToMessageFormattedHString(HMESSAGEWRITE hMessage, int messageCode, ...)
{
	return DE_ERROR;
}

DRESULT LTELClient::WriteToMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObj)
{
	return shared_WriteToMessageObject(hMessage, hObj);
}

DRESULT LTELClient::WriteToLoadSaveMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObject)
{
	return shared_WriteToLoadSaveMessageObject(hMessage, hObject);
}

float LTELClient::ReadFromMessageFloat(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageFloat(hMessage);
}

DBYTE LTELClient::ReadFromMessageByte(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageByte(hMessage);
}

D_WORD LTELClient::ReadFromMessageWord(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageWord(hMessage);
}

DDWORD LTELClient::ReadFromMessageDWord(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageDWord(hMessage);
}

char* LTELClient::ReadFromMessageString(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageString(hMessage);
}

void LTELClient::ReadFromMessageVector(HMESSAGEREAD hMessage, DVector* pVal)
{
	return shared_ReadFromMessageVector(hMessage, pVal);
}

void LTELClient::ReadFromMessageCompVector(HMESSAGEREAD hMessage, DVector* pVal)
{
	return shared_ReadFromMessageCompVector(hMessage, pVal);
}

void LTELClient::ReadFromMessageCompPosition(HMESSAGEREAD hMessage, DVector* pVal)
{
	return shared_ReadFromMessageCompPosition(hMessage, pVal);
}

void LTELClient::ReadFromMessageRotation(HMESSAGEREAD hMessage, DRotation* pVal)
{
	return shared_ReadFromMessageRotation(hMessage, pVal);
}

HOBJECT LTELClient::ReadFromMessageObject(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageObject(hMessage);
}

HSTRING LTELClient::ReadFromMessageHString(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageHString(hMessage);
}

DRESULT LTELClient::ReadFromLoadSaveMessageObject(HMESSAGEREAD hMessage, HOBJECT* hObject)
{
	return shared_ReadFromLoadSaveMessageObject(hMessage, hObject);
}

HMESSAGEREAD LTELClient::ReadFromMessageHMessageRead(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageHMessageRead(hMessage);
}

void LTELClient::EndHMessageRead(HMESSAGEREAD hMessage)
{
	shared_EndHMessageRead(hMessage);
}

void LTELClient::EndHMessageWrite(HMESSAGEWRITE hMessage)
{
	shared_EndHMessageWrite(hMessage);
}

void LTELClient::ResetRead(HMESSAGEREAD hRead)
{
	shared_ResetRead(hRead);
}
