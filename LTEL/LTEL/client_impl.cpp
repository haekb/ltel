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



DBOOL impl_InitMusic(char* szMusicDLL)
{
	return TRUE;
}

DRESULT impl_InitSound(InitSoundInfo* pSoundInfo)
{
	// No sound settings yet!
	return DE_OK;
}

void impl_SetMusicVolume(short wVolume)
{
	godot::Godot::print("[impl_SetMusicVolume] Music volume is now {0}", wVolume);
	return;
}

void impl_SetSoundVolume(unsigned short nVolume)
{
	godot::Godot::print("[impl_SetSoundVolume] Sound volume is now {0}", nVolume);
	return;
}

DRESULT impl_SetReverbProperties(ReverbProperties* pReverbProperties)
{
	return DE_OK;
}

DRESULT impl_GetDeviceName(DDWORD nDeviceType, char* pStrBuffer, DDWORD nBufferSize)
{
	// Input not supported yet!
	return DE_NOTFOUND;
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

DeviceObject* impl_GetDeviceObjects(DDWORD nDeviceFlags)
{
	return nullptr;
}

void impl_FreeDeviceObjects(DeviceObject* pList)
{
	return;
}

DeviceBinding* impl_GetDeviceBindings(DDWORD nDevice)
{
	return nullptr;
}

void impl_FreeDeviceBindings(DeviceBinding* pBindings)
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

void impl_PauseSounds()
{
	godot::Godot::print("[impl_PauseSounds] Sounds are paused!");
}

void impl_ResumeSounds()
{
	godot::Godot::print("[impl_ResumeSounds] Sounds are unpaused!");
}

void impl_SetInputState(DBOOL bOn)
{
	godot::Godot::print("[impl_SetInputState] Input State: {0}", bOn);
}

// Pause music.  Can be resumed...
DBOOL impl_PauseMusic()
{
	godot::Godot::print("[impl_PauseMusic] Music is paused!");
	return DE_OK;
}

// Resume music...
DBOOL impl_ResumeMusic()
{
	godot::Godot::print("[impl_ResumeMusic] Music is unpaused!");
	return DE_OK;
}

DRESULT impl_PlaySound(PlaySoundInfo* pPlaySoundInfo)
{
	//void* pData = malloc(4);
	pPlaySoundInfo->m_hSound = nullptr;// (HSOUNDDE)pData;
	return DE_OK;
}

void impl_KillSound(HSOUNDDE pSoundHandle)
{
	//free(pSoundHandle);
	return;
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
	const int nMaxBuffer = 5 * (PARSE_MAXTOKENSIZE + 1);
	std::string sCurrent = "";
	bool bMoreData = false;
	bool bIgnoreSpace = false;

	char* pCurrentArg = argBuffer;

	*nArgs = 0;

	while (true)
	{
		char szCurrent = *pCommand++;

		// End of string!
		if (szCurrent == 0)
		{
			char szArg[nMaxBuffer];
			strcpy_s(szArg, nMaxBuffer, sCurrent.c_str());
			szArg[nMaxBuffer-1] = '\0';

			argPointers[*nArgs] = szArg;
			++(*nArgs);

			break;
		}

		// If we're in quotes, ignore any spaces!
		if (szCurrent == '"')
		{
			bIgnoreSpace = !bIgnoreSpace;
			continue;
		}

		// Seperator between key/value
		if (!bIgnoreSpace && szCurrent == ' ')
		{
			char szArg[nMaxBuffer];
			strcpy_s(szArg, nMaxBuffer, sCurrent.c_str());
			szArg[nMaxBuffer-1] = '\0';

			argPointers[*nArgs] = szArg;
			++(*nArgs);

			sCurrent = "";
			continue;
		}

		// End of value + this functions lifespan
		if (szCurrent == ';')
		{
			char szArg[nMaxBuffer];
			strcpy_s(szArg, nMaxBuffer, sCurrent.c_str());
			szArg[nMaxBuffer-1] = '\0';

			argPointers[*nArgs] = szArg;
			++(*nArgs);

			bMoreData = true;
			break;
		}

		// Append to our on-going string
		sCurrent += szCurrent;
	}

	*pNewCommandPos = pCommand;

	return bMoreData;
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

DRESULT impl_ClearInput()
{
	return DE_OK;
}

void impl_SetGlobalLightScale(DVector* pScale)
{
	return;
}

void impl_DestroyAllSongs()
{
	return;
}

DBOOL impl_PlayList(char* szPlayList, char* szTransition, DBOOL bLoop, DDWORD dwBoundaryFlags)
{
	godot::Godot::print("[impl_PlayList] Game wants to play {0} with transition {1}", szPlayList, szTransition);
	return DE_OK;
}

DBOOL impl_IsCommandOn(int commandNum)
{
	return FALSE;
}

void impl_GetAxisOffsets(DFLOAT* offsets)
{
	offsets[0] = 0.0f;
	offsets[1] = 0.0f;
	offsets[2] = 0.0f;


	POINT lpPoint;
	int deltaX = 0, deltaY = 0;

	deltaX = g_pLTELClient->m_vRelativeMouse.x;
	deltaY = g_pLTELClient->m_vRelativeMouse.y;

	static int m_iCurrentMouseX = 0;
	static int m_iCurrentMouseY = 0;
	static int m_iPreviousMouseX = 0;
	static int m_iPreviousMouseY = 0;
	static float m_fMouseSensitivity = 1.0025f;

	m_iCurrentMouseX += deltaX;
	m_iCurrentMouseY += deltaY;

	float nScaleX = m_fMouseSensitivity + (1.0f * m_fMouseSensitivity);

	// Nerf the sensitivity scale so it matches the OG games.
	nScaleX *= 0.10f;
	float nScaleY = nScaleX;

	offsets[0] = (float)(m_iCurrentMouseX - m_iPreviousMouseX) * nScaleX;
	offsets[1] = (float)(m_iCurrentMouseY - m_iPreviousMouseY) * (nScaleY);
	offsets[2] = 0.0f;

	//g_pLTELClient->CPrint((char*)"[GetAxisOffset] %f/%f/%f", offsets[0], offsets[1], offsets[2]);

	m_iPreviousMouseX = m_iCurrentMouseX;
	m_iPreviousMouseY = m_iCurrentMouseY;

	// Cache the results so it can be used again this frame
	//m_fOffsets[0] = offsets[0];
	//m_fOffsets[1] = offsets[1];
	//m_fOffsets[2] = offsets[2];

	//m_bGetAxisOffsetCalledThisFrame = true;
}

DDWORD impl_GetPointContainers(DVector* pPoint, HLOCALOBJ* pList, DDWORD maxListSize)
{
	*pList = nullptr;
	return 0;
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

	// Audio functionality
	InitMusic = impl_InitMusic;
	InitSound = impl_InitSound;
	SetMusicVolume = impl_SetMusicVolume;
	SetSoundVolume = impl_SetSoundVolume;
	SetReverbProperties = impl_SetReverbProperties;
	PauseSounds = impl_PauseSounds;
	ResumeSounds = impl_ResumeSounds;
	PauseMusic = impl_PauseMusic;
	ResumeMusic = impl_ResumeMusic;
	PlaySound = impl_PlaySound;
	KillSound = impl_KillSound;
	DestroyAllSongs = impl_DestroyAllSongs;
	PlayList = impl_PlayList;

	// Input functionality
	GetDeviceName = impl_GetDeviceName;
	GetDeviceObjects = impl_GetDeviceObjects;
	FreeDeviceObjects = impl_FreeDeviceObjects;
	GetDeviceBindings = impl_GetDeviceBindings;
	FreeDeviceBindings = impl_FreeDeviceBindings;
	SetInputState = impl_SetInputState;
	ClearInput = impl_ClearInput;
	IsCommandOn = impl_IsCommandOn;
	GetAxisOffsets = impl_GetAxisOffsets;

	// Network functionality
	IsLobbyLaunched = impl_IsLobbyLaunched;
	InitNetworking = impl_InitNetworking;
	GetServiceList = impl_GetServiceList;
}