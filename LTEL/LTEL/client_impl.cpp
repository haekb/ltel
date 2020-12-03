#include "client.h"
#include <string>
#include <map>
#include <vector>
// Here be our accessible functions

#include "helpers.h"

// Godot stuff
#include <Godot.hpp>
#include <Reference.hpp>
#include <ResourceLoader.hpp>
#include <Resource.hpp>
#include <Texture.hpp>
#include <TextureRect.hpp>
#include <OS.hpp>
#include <Engine.hpp>
#include <Viewport.hpp>
#include <SceneTree.hpp>
#include <Node.hpp>
#include <Object.hpp>
#include <Camera.hpp>
#include <Math.hpp>
#include <Font.hpp>
#include <DynamicFont.hpp>
#include <DynamicFontData.hpp>
#include <Ref.hpp>
#include <File.hpp>
// End

#include "shared.h"

extern LTELClient* g_pLTELClient;

// TEMP!
struct oConsoleVariable {
	oConsoleVariable() {
		sName = "";
		sFullString = "";
	}
	~oConsoleVariable() {}
	std::string sName;
	std::string sFullString;
	std::string sValue;
	/*
	union {
		int nValue;
		float fValue;
		std::string sValue;
	};
	*/
};

std::map<std::string, oConsoleVariable> g_mConsoleVars;

// We should free this on destruction
std::vector<godot::Color*> g_vColours;


// end temp!

HCONSOLEVAR impl_GetConsoleVar(char* pName)
{
	auto pVar = g_mConsoleVars.find(pName);

	if (pVar != g_mConsoleVars.end())
	{
		return (HCONSOLEVAR)&pVar->second;
	}

	return nullptr;
}

void impl_RunConsoleString(char* pString)
{
	std::string sString = pString;
	auto nSpace = sString.find_first_of(" ");

	// Couldn't find a space, don't need to deal with it right now!
	if (nSpace == std::string::npos)
	{
		return;
	}

	// TODO: Check to see if space is the last char!!!
	auto sCommand = sString.substr(0, nSpace);
	auto sValue = sString.substr(nSpace + 1);

	// Check to see if we have already stored this command
	auto hVal = impl_GetConsoleVar((char*)sCommand.c_str());
	if (hVal)
	{
		auto pVar = (oConsoleVariable*)hVal;
		pVar->sValue = sValue;
		return;
	}

	oConsoleVariable oConsoleVar;

	oConsoleVar.sName = sCommand;
	oConsoleVar.sValue = sValue;

	auto pPair = std::make_pair(sCommand, oConsoleVar);
	
	g_mConsoleVars[sCommand] = oConsoleVar;
	//g_mConsoleVars.insert(pPair);
}

// Gets the value of a parameter .. returns 0/NULL if you pass in NULL.
float impl_GetVarValueFloat(HCONSOLEVAR hVar)
{
	if (!hVar)
	{
		return 0.0f;
	}

	auto pVar = (oConsoleVariable*)hVar;

	return ::atof(pVar->sValue.c_str());

}
char* impl_GetVarValueString(HCONSOLEVAR hVar)
{
	if (!hVar)
	{
		return nullptr;
	}

	auto pVar = (oConsoleVariable*)hVar;
	return (char*)pVar->sValue.c_str();
}

DRESULT impl_RegisterConsoleProgram(char* pName, ConsoleProgramFn fn)
{
	// Do nothing right now!
	return DE_OK;
}

DRESULT impl_SetModelHook(ModelHookFn fn, void* pUser)
{
	// Do nothing right now!
	return DE_OK;
}

// We don't support MP right now for OBVIOUS reasons
DRESULT impl_IsLobbyLaunched(char* sDriver)
{
	// I'm not sure the right error to use...
	return DE_ERROR;
}

DRESULT impl_ReadConfigFile(char* pFilename)
{
	godot::Godot::print("[impl_ReadConfigFile] Config wants to be read: {0}", pFilename);

	return DE_OK;
}

FileEntry* impl_GetFileList(char* pDirName)
{
	godot::Godot::print("[impl_GetFileList] File list requested for directory: {0}", pDirName);
	return nullptr;
}

void impl_FreeFileList(FileEntry* pHead)
{
	return;
}



float impl_GetTime()
{
	auto pOS = godot::OS::get_singleton();
	return (float)pOS->get_ticks_msec() / 1000.0f;
}

float impl_GetFrameTime()
{
	return g_pLTELClient->m_fFrametime;
}

void impl_CPrint(char* pMsg, ...)
{
	va_list list;
	char szMessage[256] = "";


	va_start(list, pMsg);

	vsnprintf(szMessage, 256, pMsg, list);

	va_end(list);

	godot::Godot::print("[CONSOLE PRINT]: {0}", szMessage);
}

DRESULT impl_GetGameMode(int* mode)
{
	*mode = g_pLTELClient->m_nGameMode;
	return DE_OK;
}



DRESULT impl_GetEngineHook(char* pName, void** pData)
{
	godot::Godot::print("[impl_GetEngineHook] Requesting {0}", pName);

	if (_stricmp(pName, "cres_hinstance") == 0)
	{
		*pData = g_pLTELClient->m_pCRes;
		return DE_OK;
	}
	return DE_NOTFOUND;
}

void impl_Shutdown()
{
	g_pLTELClient->m_pGodotLink->get_tree()->quit();
}

DRESULT impl_GetWorldInfoString(char* pFilename, char* pInfoString, DDWORD maxLen, DDWORD* pActualLen)
{
	// Reference!
	struct ShogoWorldHeader {
		uint32_t Version;
		uint32_t ObjectDataPosition;
		uint32_t RenderDataPosition;

		int WorldInfoLength;
		//LTString WorldInfo;
	};

	std::string sPath = g_pLTELClient->m_sGameDataDir + pFilename;

	godot::Ref<godot::File> pFile = godot::File::_new();
		
	auto hError = pFile->open(sPath.c_str(), godot::File::READ);

	if (hError == godot::Error::ERR_FILE_NOT_FOUND)
	{
		return DE_NOTFOUND;
	}

	int nVersion = pFile->get_32();

	if (nVersion != 56)
	{
		godot::Godot::print("[impl_GetWorldInfoString] Map {0} is not version 56! Version {1} detected.", pFilename, nVersion);
	}

	int nObjectDataPosition = pFile->get_32();
	int nRenderDataPosition = pFile->get_32();

	int nWorldInfoLength = pFile->get_32();

	*pActualLen = nWorldInfoLength;

	auto pByteBuffer = pFile->get_buffer(nWorldInfoLength);

	char* szWorldInfo = (char*)pByteBuffer.read().ptr();
	szWorldInfo[nWorldInfoLength - 1] = '\0';

	strcpy_s(pInfoString, maxLen, szWorldInfo);

	pFile->close();

	bool bTest = true;
	return DE_OK;
}

//
// This is horrible, needs a good re-work, but I hate tokenizing strings.
//
int	impl_Parse(char* pCommand, char** pNewCommandPos, char* argBuffer, char** argPointers, int* nArgs)
{
	return shared_Parse(pCommand, pNewCommandPos, argBuffer, argPointers, nArgs);
}

DBOOL impl_IsConnected()
{
	return g_pLTELClient->m_bIsConnected;
}

DRESULT impl_StartGame(StartGameRequest* pRequest)
{
	bool bHi = false;

	g_pLTELClient->StartServerDLL(pRequest);

	return DE_OK;
}

HLOCALOBJ impl_GetClientObject()
{
	return (HLOCALOBJ)g_pLTELClient->m_pClientInfo->GetObj();
}

DRESULT impl_InitNetworking(char* pDriver, DDWORD dwFlags)
{
	godot::Godot::print("[impl_InitNetworking] Networking Init requested!");
	return DE_ERROR;
}

DRESULT impl_GetServiceList(NetService*& pListHead)
{
	pListHead = nullptr;
	return DE_OK;
}

LPCLIENTSHELLDE impl_GetClientShell()
{
	return (LPCLIENTSHELLDE)g_pLTELClient->m_pClientShell;
}


void impl_SetGlobalLightScale(DVector* pScale)
{
	return;
}


DDWORD impl_GetPointContainers(DVector* pPoint, HLOCALOBJ* pList, DDWORD maxListSize)
{
	*pList = nullptr;
	return 0;
}

DRESULT impl_GetLocalClientID(DDWORD* pID)
{
	*pID = 0;
	return DE_OK;
}

//
// Setup our struct!
//
void LTELClient::InitFunctionPointers()
{
	GetClientShell = impl_GetClientShell;

	GetConsoleVar = impl_GetConsoleVar;
	RunConsoleString = impl_RunConsoleString;
	GetVarValueFloat = impl_GetVarValueFloat;
	GetVarValueString = impl_GetVarValueString;

	SetModelHook = impl_SetModelHook;

	RegisterConsoleProgram = impl_RegisterConsoleProgram;

	// Container functionality
	GetPointContainers = impl_GetPointContainers;

	// System/IO functionality
	ReadConfigFile = impl_ReadConfigFile;
	GetFileList = impl_GetFileList;
	FreeFileList = impl_FreeFileList;
	GetTime = impl_GetTime;
	GetFrameTime = impl_GetFrameTime;
	CPrint = impl_CPrint;
	GetEngineHook = impl_GetEngineHook;
	Shutdown = impl_Shutdown;
	Parse = impl_Parse;

	// World functionality
	GetWorldInfoString = impl_GetWorldInfoString;
	SetGlobalLightScale = impl_SetGlobalLightScale;

	// Game state functionality
	StartGame = impl_StartGame;
	GetGameMode = impl_GetGameMode;
	IsConnected = impl_IsConnected;
	GetClientObject = impl_GetClientObject;
	GetLocalClientID = impl_GetLocalClientID;

	// Network functionality
	IsLobbyLaunched = impl_IsLobbyLaunched;
	InitNetworking = impl_InitNetworking;
	GetServiceList = impl_GetServiceList;
}