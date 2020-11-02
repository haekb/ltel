#include "client.h"
#include <string>
#include <map>
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


HLOCALOBJ impl_CreateObject(ObjectCreateStruct* pStruct)
{
	godot::Node* pNode = nullptr;

	godot::Node* p3DNode = godot::Object::cast_to<godot::Node>(g_pLTELClient->m_pGodotLink->get_node("/root/Scene/Camera"));


	if (pStruct->m_ObjectType == OT_CAMERA)
	{
		//godot::Camera* pCamera = godot::Camera::_new();

		//p3DNode->add_child(pCamera);

		// Camera doesn't require any other settings
		return (HLOCALOBJ)p3DNode->get_parent();
	}


	return (HLOCALOBJ)pNode;
}

void impl_GetCameraRect(HLOCALOBJ hObj, DBOOL* bFullscreen,
	int* left, int* top, int* right, int* bottom)
{
	*left = 0;
	*top = 0;
	*right = 1024;
	*bottom = 768;
}

void impl_SetCameraRect(HLOCALOBJ hObj, DBOOL bFullscreen,
	int left, int top, int right, int bottom)
{
	// Uhhh not right now
	return;
}

void impl_SetCameraFOV(HLOCALOBJ hObj, float fovX, float fovY)
{
	godot::Camera* pCamera = (godot::Camera*)hObj;

	if (!pCamera)
	{
		return;
	}

	float fFOV = godot::Math::rad2deg(fovX);
	float fIgnore = godot::Math::rad2deg(fovY);

	godot::Godot::print("[impl_SetCameraFOV] Setting FOV to {0}, ignoring Y value {1}", fFOV, fIgnore);

	pCamera->set_fov(fFOV);
}

HSTRING impl_FormatString(int messageCode, ...)
{
	if (!g_pLTELClient->m_pCRes)
	{
		return nullptr;
	}

	TCHAR szBuffer[2048];

	auto nRet = LoadString(g_pLTELClient->m_pCRes, messageCode, szBuffer, sizeof(szBuffer) / sizeof(TCHAR));

	// Not found!
	if (nRet == 0)
	{
		return nullptr;
	}

	return (HSTRING)szBuffer;
}

void impl_FreeString(HSTRING hString)
{
	hString = nullptr;
}

char* impl_GetStringData(HSTRING hString)
{
	return (char*)hString;
}

HDEFONT impl_CreateFont(char* pFontName, int width, int height,
	DBOOL bItalic, DBOOL bUnderline, DBOOL bBold)
{
	godot::Godot::print("[impl_CreateFont] Wants to create {0}", pFontName);

	// TODO: Need to implement dynamic / bitmap fonts
	//auto pFont = godot::DynamicFont::_new();
	

	auto pFont = godot::Font::_new();

	return (HDEFONT)pFont;
}

void impl_DeleteFont(HDEFONT hFont)
{
	godot::Font* pFont = (godot::Font*)hFont;
	pFont->free();
}

HDECOLOR impl_SetupColor1(float r, float g, float b, DBOOL bTransparent)
{
	float a = bTransparent ? 0.5f : 1.0f;
	godot::Color oColor = { r, g, b, a };
	return (HDECOLOR)&oColor;
}

HDECOLOR impl_SetupColor2(float r, float g, float b, DBOOL bTransparent)
{
	return impl_SetupColor1(r, g, b, bTransparent);
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

HSTRING impl_CreateString(char* pString)
{
	return (HSTRING)pString;
}

float impl_GetTime()
{
	auto pOS = godot::OS::get_singleton();
	return (float)pOS->get_ticks_msec() / 1000.0f;
}

void impl_CPrint(char* pMsg, ...)
{
	godot::Godot::print(pMsg);
}

DRESULT impl_GetGameMode(int* mode)
{
	*mode = GAMEMODE_NONE;
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
	char pFake[16] = "Lol";
	pPlaySoundInfo->m_hSound = (HSOUNDDE)pFake;
	return DE_OK;
}

//
// Setup our struct!
//
void LTELClient::InitFunctionPointers()
{
	GetConsoleVar = impl_GetConsoleVar;
	RunConsoleString = impl_RunConsoleString;
	GetVarValueFloat = impl_GetVarValueFloat;
	GetVarValueString = impl_GetVarValueString;

	SetModelHook = impl_SetModelHook;

	RegisterConsoleProgram = impl_RegisterConsoleProgram;

	// System/IO functionality
	ReadConfigFile = impl_ReadConfigFile;
	GetFileList = impl_GetFileList;
	FreeFileList = impl_FreeFileList;
	GetTime = impl_GetTime;
	CPrint = impl_CPrint;

	// Game state functionality
	GetGameMode = impl_GetGameMode;

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

	// Input functionality
	GetDeviceName = impl_GetDeviceName;
	GetDeviceObjects = impl_GetDeviceObjects;
	FreeDeviceObjects = impl_FreeDeviceObjects;
	GetDeviceBindings = impl_GetDeviceBindings;
	FreeDeviceBindings = impl_FreeDeviceBindings;
	SetInputState = impl_SetInputState;

	// Object functionality
	CreateObject = impl_CreateObject;

	// Camera functionality
	GetCameraRect = impl_GetCameraRect;
	SetCameraRect = impl_SetCameraRect;
	SetCameraFOV = impl_SetCameraFOV;

	// String functionality
	FormatString = impl_FormatString;
	FreeString = impl_FreeString;
	GetStringData = impl_GetStringData;
	CreateFont = impl_CreateFont;
	DeleteFont = impl_DeleteFont;
	CreateString = impl_CreateString;


	SetupColor1 = impl_SetupColor1;
	SetupColor2 = impl_SetupColor2;

	// Network functionality
	IsLobbyLaunched = impl_IsLobbyLaunched;
}