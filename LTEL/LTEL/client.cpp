
#include "client.h"
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


bool LTELClient::StartServerDLL()
{
	typedef int f_GetServerShellVersion();
	typedef void f_GetServerShellFunctions(CreateServerShellFn* pCreate, DeleteServerShellFn* pDelete);

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

	// We'll want to run CreateServerShellFn, to get the game's ObjectLTO instance
	CreateServerShellFn pCreate = (CreateServerShellFn)pnCreate;
	auto pServerShell = (ServerShellDE*)pCreate(m_pLTELServer);

	m_pLTELServer->m_pServerShell = pServerShell;

#if 0
	//
	int pnCreate = 0;
	int pnDelete = 0;

	pClientShellInitFunc((CreateClientShellFn*)&pnCreate, (DeleteClientShellFn*)&pnDelete);

	if (!pnCreate)
	{
		Godot::print("Could not obtain CreateClientShellFn!");

		return false;
	}

	// Create our ClientDE instance
	g_pClient = new LTELClient(this, hCRes);

	// We'll want to run CreateClientShellFn, to get the game's GameClientShell instance
	CreateClientShellFn pCreate = (CreateClientShellFn)pnCreate;
	m_pGameClientShell = (CClientShellDE*)pCreate(g_pClient);

	if (!m_pGameClientShell)
	{
		Godot::print("Could not retrieve GameClientShell!");
		return false;
	}

	DGUID AppGUID = { 0 };

	g_pClient->m_sGameDataDir = sGameDataDir.alloc_c_string();

	// Kick off OnEngineInit
	try {
		// We should really populate RMode soon...
		auto hResult = m_pGameClientShell->OnEngineInitialized(nullptr, &AppGUID);
		Godot::print("OnEngineInit returned {0}", (int)hResult);
	}
	catch (const std::exception& e)
	{
		Godot::print("[OnEngineInitalized] Failed with exception: {0}", e.what());
	}

#endif

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
