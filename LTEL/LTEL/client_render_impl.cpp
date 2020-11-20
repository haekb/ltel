#include "client.h"

#include "helpers.h"
#include <vector>

// Godot imports
#include <Godot.hpp>
#include <Label.hpp>
#include <TextureRect.hpp>
#include <Image.hpp>
#include <ImageTexture.hpp>
#include <Texture.hpp>
#include <Control.hpp>
#include <ResourceLoader.hpp>
#include <ColorRect.hpp>
#include <DynamicFont.hpp>
#include <DynamicFontData.hpp>
#include <Material.hpp>
#include <SpatialMaterial.hpp>
#include <Engine.hpp>
#include <OS.hpp>


extern LTELClient* g_pLTELClient;
extern std::vector<GameObject*> g_pPolygridsToUpdate;

std::vector<LTELSurface*> g_pSurfacesQueuedForDeletion;

DRESULT impl_SetRenderMode(RMode* pMode)
{
	return DE_OK;
}

HSURFACE impl_CreateSurfaceFromBitmap(char* pBitmapName)
{
	std::string sBitmapName = g_pLTELClient->m_sGameDataDir + pBitmapName;

#ifdef BLOCK_FONTS
	if (sBitmapName.find("font") != std::string::npos || sBitmapName.find("Font") != std::string::npos)
	{
		godot::Godot::print("[impl_CreateSurfaceFromBitmap] Blocked bitmap font: {0}", sBitmapName.c_str());

		return nullptr;
	}
#endif

	bool isFont = (sBitmapName.find("font") != std::string::npos || sBitmapName.find("Font") != std::string::npos);

	godot::Ref<godot::Texture> pTexture = g_pLTELClient->LoadPCX(sBitmapName);

	if (pTexture.is_null())
	{
		godot::Godot::print("[impl_CreateSurfaceFromBitmap] Failed to get texture resource at: {0}", sBitmapName.c_str());
		return nullptr;
	}

	godot::TextureRect* pTextureRect = godot::TextureRect::_new();
	pTextureRect->set_name(sBitmapName.c_str());
	pTextureRect->set_texture(pTexture);
	pTextureRect->set_stretch_mode(godot::TextureRect::STRETCH_KEEP_ASPECT_COVERED);
	pTextureRect->set_expand(true);

	LTELSurface* pSurface = new LTELSurface();
	pSurface->pTextureRect = pTextureRect;
	pSurface->bIsFontImage = isFont;

	return (HSURFACE)pSurface;
}

HSURFACE impl_CreateSurfaceFromString(HDEFONT hFont, HSTRING hString,
	HDECOLOR hForeColor, HDECOLOR hBackColor,
	int extraPixelsX, int extraPixelsY)
{
	LTELSurface* pSurface = new LTELSurface();
	
	godot::Label* pLabel = godot::Label::_new();
	LTELString* pString = (LTELString*)hString;

	godot::Color oColor = LT2GodotColor(hForeColor);

	pLabel->set_name(pString->sData.c_str());
	pLabel->set_text(pString->sData.c_str());

	pLabel->add_color_override("font_color", oColor);

	/*
	godot::DynamicFont* pFont = (godot::DynamicFont*)hFont;
	pFont->set_spacing(godot::DynamicFont::SPACING_TOP, extraPixelsY);
	pFont->set_spacing(godot::DynamicFont::SPACING_BOTTOM, extraPixelsY);
	//? 
	pFont->set_spacing(godot::DynamicFont::SPACING_SPACE, extraPixelsX);
	pFont->set_spacing(godot::DynamicFont::SPACING_CHAR, extraPixelsX);
	*/

	pSurface->bIsText = true;
	pSurface->pLabel = pLabel;

	return (HSURFACE)pSurface;
}

DRESULT impl_DeleteSurface(HSURFACE hSurface)
{
	auto pSurface = (LTELSurface*)hSurface;

	// This stuff will be freed on ClearScreen.
	pSurface->bQueuedForDeletion = true;

	g_pSurfacesQueuedForDeletion.push_back(pSurface);

	return DE_OK;
}

HSURFACE impl_GetScreenSurface()
{
	// We don't really need this yet, so just return a blank texture rect
	//static godot::TextureRect* pScreen = godot::TextureRect::_new();

	static LTELSurface* pScreen = nullptr;

	// Hacky init!
	if (!pScreen)
	{
		auto hSurface = g_pLTELClient->CreateSurface(1024, 768);
		pScreen = (LTELSurface*)hSurface;
	}

	pScreen->bIsScreen = true;

	return (HSURFACE)pScreen;
}

void impl_GetSurfaceDims(HSURFACE hSurf, DDWORD* pWidth, DDWORD* pHeight)
{
	auto pSurface = (LTELSurface*)hSurf;

	// This is expected if bitmap fonts fail to load!
	if (!pSurface)
	{
		godot::Godot::print("Failed to GetSurfaceDims, nullptr passed as hSurf!");
		return;
	}

	if (pSurface->bIsScreen)
	{
		*pWidth = 1024;
		*pHeight = 768;
		return;
	}

	if (pSurface->bIsText)
	{
		auto vSize = pSurface->pLabel->get_minimum_size();
		*pWidth = vSize.x;
		*pHeight = vSize.y;
		return;
	}

	if (!pSurface->pTextureRect)
	{
		godot::Godot::print("Failed to GetSurfaceDims, pTextureRect nullptr passed as hSurf!");
		*pWidth = 0;
		*pHeight = 0;
		return;
	}

	// TextureRect
	auto rTexture = pSurface->pTextureRect->get_texture();

	if (rTexture.is_null())
	{
		// Okay no texture...but is the texture rect dims set?
		auto vSize = pSurface->pTextureRect->get_size();

		*pWidth = vSize.width;
		*pHeight = vSize.height;
		return;
	}

	auto vSize = rTexture->get_size();
	*pWidth = vSize.x;
	*pHeight = vSize.y;
}

DRESULT impl_FillRect(HSURFACE hDest, DRect* pRect, HDECOLOR hColor)
{
	LTELSurface* pDest = (LTELSurface*)hDest;

	godot::Vector2 vPos;
	godot::Vector2 vSize;

	if (!pRect)
	{
		vPos = godot::Vector2(0, 0);
		
		unsigned long nWidth = 0;
		unsigned long nHeight = 0;

		impl_GetSurfaceDims(hDest, &nWidth, &nHeight);

		vSize = godot::Vector2(nWidth, nHeight);
	}
	else
	{
		vPos = godot::Vector2(pRect->left, pRect->top);
		vSize = godot::Vector2(pRect->right - pRect->left, pRect->bottom - pRect->top);
	}
	
	godot::Color oColor = LT2GodotColor(hColor);

	auto hSurface = g_pLTELClient->CreateSurface(vSize.width, vSize.height);
	auto pSurface = (LTELSurface*)hSurface;

	godot::Ref<godot::ImageTexture> pImageTexture = pSurface->pTextureRect->get_texture();
	godot::Ref<godot::Image> pImage = pImageTexture->get_data();
	pImage->fill(oColor);
	pImageTexture->set_data(pImage);

	// Set the position
	pSurface->pTextureRect->set_position(vPos);

	DRect pDestRect = DRect();

	if (!pRect)
	{
		pDestRect.left = 0;
		pDestRect.top = 0;

		pDestRect.right = vSize.width;
		pDestRect.bottom = vSize.height;
	}
	else
	{
		pDestRect = *pRect;
	}

	// Now scale it!
	g_pLTELClient->ScaleSurfaceToSurface(hDest, (HSURFACE)pSurface, &pDestRect, nullptr);

	g_pLTELClient->DeleteSurface((HSURFACE)pSurface);

	return DE_OK;
}

//
// Here be hacky!
//

DRESULT impl_ClearScreen(DRect* pClearRect, DDWORD flags)
{
	godot::Control* pControl = GDCAST(godot::Control, g_pLTELClient->m_pGodotLink->get_node(CANVAS_NODE));

	if (!pControl)
	{
		return DE_ERROR;
	}

	auto pChildren = pControl->get_children();
	auto nSize = pChildren.size();

	// First pass just remove them from the scene tree!
	for (int i = 0; i < pChildren.size(); i++)
	{
		pControl->remove_child(pChildren[i]);
	}


	// Incase a few surfaces sneaked in, we'll hold a list of ones that AREN'T queued for deletion
	std::vector<LTELSurface*> pTemp;

	// Loop through and free!
	for (auto pSurface : g_pSurfacesQueuedForDeletion)
	{
		if (!pSurface->bQueuedForDeletion)
		{
			pTemp.push_back(pSurface);
			continue;
		}

		if (pSurface->bIsText)
		{
			if (!pSurface->pLabel->is_inside_tree())
			{
				pSurface->pLabel->free();
			}
			else
			{
				pTemp.push_back(pSurface);
				continue;
			}
		}
		else
		{
			if (!pSurface->pTextureRect->is_inside_tree())
			{
				pSurface->pTextureRect->free();
			}
			else
			{
				pTemp.push_back(pSurface);
				continue;
			}
		}

		delete pSurface;
		pSurface = nullptr;
	}


	g_pSurfacesQueuedForDeletion.clear();
	g_pSurfacesQueuedForDeletion = pTemp;


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
	bool bRet = g_pLTELClient->BlitSurfaceToSurface((LTELSurface*)hDest, (LTELSurface*)hSrc, pDestRect, pSrcRect, true);
	return bRet ? DE_OK : DE_ERROR;
}

DRESULT impl_DrawSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc,
	DRect* pSrcRect, int destX, int destY)
{
	DRect DestRect = { destX, destY, 0, 0 };
	bool bRet = g_pLTELClient->BlitSurfaceToSurface((LTELSurface*)hDest, (LTELSurface*)hSrc, &DestRect, pSrcRect, false);
	return bRet ? DE_OK : DE_ERROR;
}

DRESULT impl_DrawSurfaceToSurfaceTransparent(HSURFACE hDest, HSURFACE hSrc,
	DRect* pSrcRect, int destX, int destY, HDECOLOR hColor)
{

	// Auto optimize surface
	g_pLTELClient->OptimizeSurface(hSrc, hColor);

	auto hResult =  impl_DrawSurfaceToSurface(hDest, hSrc, pSrcRect, destX, destY);

	return hResult;
}

DBOOL impl_DrawBitmapToSurface(HSURFACE hDest, char* pSourceBitmapName,
	DRect* pSrcRect, int destX, int destY)
{
	auto hSurf = impl_CreateSurfaceFromBitmap(pSourceBitmapName);

	if (!hSurf)
	{
		return DFALSE;
	}

	auto pRet = impl_DrawSurfaceToSurface(hDest, hSurf, pSrcRect, destX, destY);

	impl_DeleteSurface(hSurf);

	return pRet;
}

void impl_DrawStringToSurface(HSURFACE hDest, HDEFONT hFont, HSTRING hString,
	DRect* pRect, HDECOLOR hForeColor, HDECOLOR hBackColor)
{
	auto hSurf = impl_CreateSurfaceFromString(hFont, hString, hForeColor, hBackColor, 0, 0);

	if (!hSurf)
	{
		return;
	}

	auto pRet = impl_DrawSurfaceToSurface(hDest, hSurf, nullptr, pRect->left, pRect->top);

	impl_DeleteSurface(hSurf);

	return;
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
#include <VisualServer.hpp>
DRESULT impl_FlipScreen(DDWORD flags)
{
#if 0
	auto pVS = godot::VisualServer::get_singleton();
	auto bIsRenderLoopEnabled = pVS->call("get_render_loop_enabled");


	if (!bIsRenderLoopEnabled)
	{
		pVS->force_sync();
		pVS->force_draw();
	}
#endif

	return DE_OK;
}

DRESULT impl_OptimizeSurface(HSURFACE hSurface, HDECOLOR hTransparentColor)
{

	if (!hSurface)
	{
		return DE_OK;
	}

	LTELSurface* pSurface = (LTELSurface*)hSurface;

	// We don't optimize the screen or text...or things that are already optimized!
	if (pSurface->bOptimized || pSurface->bIsScreen || pSurface->bIsText)
	{
		return DE_OK;
	}

	// Null check
	if (!pSurface->pTextureRect || pSurface->pTextureRect->get_texture().is_null() || pSurface->pTextureRect->get_texture()->get_data().is_null())
	{
		// We don't know if this is a jake error, or a game error yet!
		return DE_OK;
	}

	godot::Color oColor = godot::Color();

	if (hTransparentColor)
	{
		oColor = LT2GodotColor(hTransparentColor);
	}

	oColor.a = 1.0f;

	godot::Ref<godot::ImageTexture> pTexture = pSurface->pTextureRect->get_texture();

	auto pImage = pTexture->get_data();

	pImage->lock();
	
	// Loop through the image, check the pixel and set it to transparent!
	for (int x = 0; x < pImage->get_width(); x++)
	{
		for (int y = 0; y < pImage->get_height(); y++)
		{
			auto pPixel = pImage->get_pixel(x, y);
			pPixel.a = 1.0f;

			if (pPixel == oColor)
			{
				// Just swap the alpha, and put it back!
				pPixel.a = 0.0f;
				pImage->set_pixel(x, y, pPixel);
			}

		}
	}
	
	pImage->unlock();

	// Now re-set the image!
	pTexture->set_data(pImage);

	// Cool, let's not do this again
	pSurface->bOptimized = true;

	return DE_OK;
}

DRESULT impl_GetRenderMode(RMode* pMode)
{
	// First off, let's clear it!
	memset(pMode, 0, sizeof(RMode));

	pMode->m_bHardware = true;
	pMode->m_BitDepth = 32;
	strcpy_s(pMode->m_Description, "GPU");
	pMode->m_Height = 768;
	pMode->m_Width = 1024;
	strcpy_s(pMode->m_InternalName, "GPU");
	strcpy_s(pMode->m_RenderDLL, "OpenGL ES 3.0 Renderer");
	pMode->m_pNext = nullptr;
	return DE_OK;
}

HSURFACE impl_CreateSurface(DDWORD width, DDWORD height)
{
	LTELSurface* pSurface = new LTELSurface();
	godot::TextureRect* pTextureRect = godot::TextureRect::_new();

	static int nSurfaceCounter = 0;

	std::string sEmpty = "Surface: " + std::to_string(nSurfaceCounter);
	nSurfaceCounter++;

	pTextureRect->set_name(sEmpty.c_str());
	pTextureRect->set_size(godot::Vector2(width, height));

	// Create a blank image surface
	godot::Ref<godot::Image> pImage = godot::Image::_new();
	// This should maybe be RGBA8...
	pImage->create(width, height, false, godot::Image::FORMAT_RGBA8);
	// debug:
	// pImage->fill(godot::Color(1.0, 0.4, 0.2));

	// Create the ImageTexture...from the image.
	godot::Ref<godot::ImageTexture> pImageTexture = godot::ImageTexture::_new();
	pImageTexture->create_from_image(pImage);
	pTextureRect->set_texture(pImageTexture);

	// Set it to the surface, and let's roll out!
	pSurface->pTextureRect = pTextureRect;
	return (HSURFACE)pSurface;
}

DRESULT impl_GetBorderSize(HSURFACE hSurface, HDECOLOR hColor, DRect* pRect)
{
	unsigned long nWidth = 0;
	unsigned long nHeight = 0;

	impl_GetSurfaceDims(hSurface, &nWidth, &nHeight);

	pRect->bottom = 0;
	pRect->left = 0;
	pRect->right = 0;
	pRect->top = 0;

	return DE_OK;
}

DRESULT impl_RenderObjects(HLOCALOBJ hCamera, HLOCALOBJ* pObjects, int nObjects)
{

	// Because of how game code updates polygrid's height map, 
	// we need to finish up and apply the new depth texture!
	for (auto pObj : g_pPolygridsToUpdate)
	{
		// Oops, somehow it's empty? Skip!
		if (!pObj)
		{
			continue;
		}

		LTELPolyGrid* pExtraData = (LTELPolyGrid*)pObj->GetExtraData();
		auto pDetailImage = pExtraData->pColormap->get_data();
		auto pDepthImage = pExtraData->pHeightmap->get_data();
		auto pData = pExtraData->pData;

		pDetailImage->lock();
		pDepthImage->lock();

		// Detail and depth are both 16x, so we can do it in the same loop!
		// Loop through and set each pixel, it's only 16x16 so not too slow...
		for (int x = 0; x < pDepthImage->get_width(); x++)
		{
			for (int y = 0; y < pDepthImage->get_height(); y++)
			{
				//
				// DETAIL - Not used yet, might be brightness per vertex height?
				//
				auto pColourValues = pExtraData->pColorTable[x * y];
				godot::Color oDetailColor = godot::Color((float)pColourValues.x / 255.0f, (float)pColourValues.y / 255.0f, (float)pColourValues.z / 255.0f, (float)pColourValues.a / 255.0f);
				//pDetailImage->set_pixel(x, y, oDetailColor);

				//
				// DEPTH
				//

				// Get the height data, create a greyscaled pixel, and set it!
				float fVal = (float)pData[x * y] / 255;
				godot::Color oDepthColor = godot::Color(fVal, fVal, fVal, 1.0f);
				pDepthImage->set_pixel(x, y, oDepthColor);
			}
		}

		pDepthImage->unlock();
		pDetailImage->unlock();

		// Now we need to re-set the data, otherwise it will never update!
		pExtraData->pColormap->set_data(pDetailImage);
		pExtraData->pHeightmap->set_data(pDepthImage);

		//pDetailImage->save_png("detail.png");

		// Clear the data
		memset(pData, 0, sizeof(pDepthImage->get_data().size()));
	}

	// Clean up our list of pointers!
	g_pPolygridsToUpdate.clear();

	impl_ClearScreen(nullptr, 0);

	return DE_OK;
}

DRESULT impl_ShutdownRender(DDWORD flags)
{
	if (flags & RSHUTDOWN_MINIMIZEWINDOW)
	{
		godot::OS::get_singleton()->set_window_minimized(true);
	}

	if (flags & RSHUTDOWN_HIDEWINDOW)
	{
		// ?
	}

	godot::Godot::print("[impl_ShutdownRender] Render shutdown requested with flags: {0}", (int)flags);

	return DE_OK;
}

// This must be last!
void LTELClient::InitRenderImpl()
{
	// Render functionality
	SetRenderMode = impl_SetRenderMode;
	CreateSurfaceFromBitmap = impl_CreateSurfaceFromBitmap;
	CreateSurfaceFromString = impl_CreateSurfaceFromString;
	GetScreenSurface = impl_GetScreenSurface;
	GetSurfaceDims = impl_GetSurfaceDims;
	GetBorderSize = impl_GetBorderSize;

	CreateSurface = impl_CreateSurface;
	DeleteSurface = impl_DeleteSurface;

	OptimizeSurface = impl_OptimizeSurface;

	ClearScreen = impl_ClearScreen;
	Start3D = impl_Start3D;
	RenderCamera = impl_RenderCamera;
	StartOptimized2D = impl_StartOptimized2D;
	ScaleSurfaceToSurface = impl_ScaleSurfaceToSurface;
	DrawSurfaceToSurface = impl_DrawSurfaceToSurface;
	DrawSurfaceToSurfaceTransparent = impl_DrawSurfaceToSurfaceTransparent;
	DrawBitmapToSurface = impl_DrawBitmapToSurface;
	DrawStringToSurface = impl_DrawStringToSurface;
	FillRect = impl_FillRect;
	EndOptimized2D = impl_EndOptimized2D;
	End3D = impl_End3D;
	FlipScreen = impl_FlipScreen;

	RenderObjects = impl_RenderObjects;

	GetRenderMode = impl_GetRenderMode;

	ShutdownRender = impl_ShutdownRender;
}