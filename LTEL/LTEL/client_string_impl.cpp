#include "client.h"
#include <string>
#include <map>
#include <vector>
// Here be our accessible functions

#include "shared.h"
#include "helpers.h"

// Godot stuff
#include <ResourceLoader.hpp>
#include <DynamicFont.hpp>
#include <DynamicFontData.hpp>
// End

extern LTELClient* g_pLTELClient;


HSTRING impl_CreateString(char* pString)
{
	return (HSTRING)shared_CreateString(pString);
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

	va_list list;
	va_start(list, messageCode);

	char* szOutBuffer = nullptr;

	if (!FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		szBuffer,
		0,
		0,
		(char*)&szOutBuffer,
		1,
		&list))
	{
		godot::Godot::print("Format message failed with 0x{0}\n", (int)GetLastError());
		return (HSTRING)impl_CreateString(szBuffer);
	}

	va_end(list);

	return (HSTRING)impl_CreateString(szOutBuffer);
}

void impl_FreeString(HSTRING hString)
{
	shared_FreeString(hString);
}

char* impl_GetStringData(HSTRING hString)
{
	return shared_GetStringData(hString);
}

void impl_GetStringDimensions(HDEFONT hFont, HSTRING hString, int* sizeX, int* sizeY)
{
	auto hSurf = g_pLTELClient->CreateSurfaceFromString(hFont, hString, 0, 0, 0, 0);

	if (!hSurf)
	{
		*sizeX = 0;
		*sizeY = 0;
		return;
	}

	unsigned long nWidth = 0;
	unsigned long nHeight = 0;

	g_pLTELClient->GetSurfaceDims(hSurf, &nWidth, &nHeight);

	*sizeX = (int)nWidth;
	*sizeY = (int)nHeight;

	g_pLTELClient->DeleteSurface(hSurf);
}

HDEFONT impl_CreateFont(char* pFontName, int width, int height,
	DBOOL bItalic, DBOOL bUnderline, DBOOL bBold)
{
	auto pResourceLoader = godot::ResourceLoader::get_singleton();

	godot::Ref<godot::DynamicFontData> pFontData = pResourceLoader->load("res://fonts/arial.ttf");

	godot::DynamicFont* pFont = godot::DynamicFont::_new();
	pFont->set_name(pFontName);
	pFont->set_font_data(pFontData);
	pFont->set_size(height);

	return (HDEFONT)pFont;
}

void impl_DeleteFont(HDEFONT hFont)
{
	godot::DynamicFont* pFont = (godot::DynamicFont*)hFont;
	pFont->free();
}

HDECOLOR impl_SetupColor1(float r, float g, float b, DBOOL bTransparent)
{
	if (bTransparent)
	{
		return SETRGB_F(r, g, b);
	}

	return SETRGB_F(r, g, b);
}

HDECOLOR impl_SetupColor2(float r, float g, float b, DBOOL bTransparent)
{
	return impl_SetupColor1(r, g, b, bTransparent);
}

//
// Setup our struct!
//
void LTELClient::InitStringImpl()
{

	// String functionality
	FormatString = impl_FormatString;
	FreeString = impl_FreeString;
	GetStringData = impl_GetStringData;
	CreateFont = impl_CreateFont;
	DeleteFont = impl_DeleteFont;
	CreateString = impl_CreateString;
	GetStringDimensions = impl_GetStringDimensions;
	SetupColor1 = impl_SetupColor1;
	SetupColor2 = impl_SetupColor2;

}

