#include "client.h"
#include <string>
#include <map>
// Here be our accessible functions

// Godot stuff
#include <Godot.hpp>
#include <Reference.hpp>
#include <ResourceLoader.hpp>
#include <Resource.hpp>
#include <Texture.hpp>
#include <TextureRect.hpp>
#include <Engine.hpp>
#include <Viewport.hpp>
#include <SceneTree.hpp>
#include <Node.hpp>
#include <GodotGlobal.hpp>

// End

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

// helpers
bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}


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

DRESULT impl_SetRenderMode(RMode* pMode)
{
	return DE_OK;
}

HSURFACE impl_CreateSurfaceFromBitmap(char* pBitmapName)
{
	std::string sBitmapName = pBitmapName;
	if (!replace(sBitmapName, ".pcx", ".png"))
	{
		godot::Godot::print("[impl_CreateSurfaceFromBitmap] Failed to replace pcx with png! String: {0}", pBitmapName);
		return nullptr;
	}

	std::string sResourcePath = "res://shogo/" + sBitmapName;

	auto pResourceLoader = godot::ResourceLoader::get_singleton();

	godot::Ref<godot::Texture> pTexture = pResourceLoader->load(sResourcePath.c_str());

	if (pTexture.is_null())
	{
		godot::Godot::print("[impl_CreateSurfaceFromBitmap] Failed to get texture resource at: {0}", sResourcePath.c_str());
		return nullptr;
	}

	godot::TextureRect* pTextureRect = godot::TextureRect::_new();
	pTextureRect->set_texture(pTexture);
	pTextureRect->set_stretch_mode(godot::TextureRect::STRETCH_KEEP_ASPECT_COVERED);
	pTextureRect->set_expand(true);

	return (HSURFACE)pTextureRect;
}

DRESULT impl_DeleteSurface(HSURFACE hSurface)
{
	godot::TextureRect* pTextureRect = (godot::TextureRect*)hSurface;
	if (!pTextureRect)
	{
		godot::Godot::print("[impl_DeleteSurface] Failed to retrieve TextureRect from HSURFACE");
		return DE_ERROR;
	}

	pTextureRect->queue_free();
	pTextureRect = nullptr;
	hSurface = nullptr;
	return DE_OK;
}

HSURFACE impl_GetScreenSurface()
{
	// We don't really need this yet, so just return a blank texture rect
	static godot::TextureRect* pScreen = godot::TextureRect::_new();

	return (HSURFACE)pScreen;
}

void impl_GetSurfaceDims(HSURFACE hSurf, DDWORD* pWidth, DDWORD* pHeight)
{
	godot::TextureRect* pTextureRect = (godot::TextureRect*)hSurf;
	if (!pTextureRect)
	{
		godot::Godot::print("[impl_GetSurfaceDims] Failed to retrieve TextureRect from HSURFACE");
		return;
	}

	auto rTexture = pTextureRect->get_texture();

	// Oh this must be the screen...
	if (rTexture.is_null())
	{
		*pWidth = 1024;
		*pHeight = 768;
		return;
	}

	auto vSize = rTexture->get_size();
	*pWidth = vSize.x;
	*pHeight = vSize.y;
}

//
// Here be hacky!
//

DRESULT impl_ClearScreen(DRect* pClearRect, DDWORD flags)
{
	// We don't need to actually implement this right now
	return DE_OK;
}

DRESULT impl_Start3D()
{
	return DE_OK;
}

// We don't control this right now
DRESULT impl_RenderCamera(HLOCALOBJ hCamera)
{
	return DE_OK;
}

DRESULT impl_StartOptimized2D()
{
	return DE_OK;
}

DRESULT impl_ScaleSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc,
	DRect* pDestRect, DRect* pSrcRect)
{
	// Don't need this yet
	return DE_OK;
}

DRESULT impl_EndOptimized2D()
{
	return DE_OK;
}

// Returns LT_OK or LT_NOTINITIALIZED or LT_NOTIN3D.
DRESULT impl_End3D()
{
	return DE_OK;
}

DRESULT impl_FlipScreen(DDWORD flags)
{
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

	// Render functionality
	SetRenderMode = impl_SetRenderMode;
	CreateSurfaceFromBitmap = impl_CreateSurfaceFromBitmap;
	GetScreenSurface = impl_GetScreenSurface;
	GetSurfaceDims = impl_GetSurfaceDims;
	DeleteSurface = impl_DeleteSurface;

	ClearScreen = impl_ClearScreen;
	Start3D = impl_Start3D;
	RenderCamera = impl_RenderCamera;
	StartOptimized2D = impl_StartOptimized2D;
	ScaleSurfaceToSurface = impl_ScaleSurfaceToSurface;
	EndOptimized2D = impl_EndOptimized2D;
	End3D = impl_End3D;
	FlipScreen = impl_FlipScreen;

	// Network functionality
	IsLobbyLaunched = impl_IsLobbyLaunched;
}