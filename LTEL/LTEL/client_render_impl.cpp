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


//#define CANVAS_NODE "/root/Scene/Camera/2D/Viewport/Canvas"

// This lets us fallback to system fonts while we develop bitmap font support
#define BLOCK_FONTS

#define CANVAS_NODE "/root/Scene/Canvas"

extern LTELClient* g_pLTELClient;
extern std::vector<LTELObject*> g_pPolygridsToUpdate;

struct LTELSurface {

	LTELSurface() {
		bIsText = false;
		bIsScreen = false;
		pTextureRect = nullptr;
		pLabel = nullptr;
	}

	~LTELSurface() {
		return;

		// Freeing the real object will occur on ClearScreen!
		if (pTextureRect)
		{
			pTextureRect->queue_free();
		}
		if (pLabel)
		{
			pLabel->queue_free();
		}
	}

	bool bIsText;
	bool bIsScreen;

	// If bIsText == false:
	godot::TextureRect* pTextureRect;
	// Else:
	godot::Label* pLabel;
};


DRESULT impl_SetRenderMode(RMode* pMode)
{
	return DE_OK;
}

HSURFACE impl_CreateSurfaceFromBitmap(char* pBitmapName)
{
	std::string sBitmapName = g_pLTELClient->m_sGameDataDir + pBitmapName;
#if 0
	if (!replace(sBitmapName, ".pcx", ".png"))
	{
		godot::Godot::print("[impl_CreateSurfaceFromBitmap] Failed to replace pcx with png! String: {0}", pBitmapName);
		return nullptr;
	}

	std::string sResourcePath = "res://shogo/" + sBitmapName;

	auto pResourceLoader = godot::ResourceLoader::get_singleton();

	godot::Ref<godot::Texture> pTexture = pResourceLoader->load(sResourcePath.c_str());
#else

#ifdef BLOCK_FONTS
	if (sBitmapName.find("font") != std::string::npos || sBitmapName.find("Font") != std::string::npos)
	{
		godot::Godot::print("[impl_CreateSurfaceFromBitmap] Blocked bitmap font: {0}", sBitmapName.c_str());

		return nullptr;
	}
#endif

	godot::Ref<godot::Texture> pTexture = g_pLTELClient->LoadPCX(sBitmapName);

#endif


	if (pTexture.is_null())
	{
		godot::Godot::print("[impl_CreateSurfaceFromBitmap] Failed to get texture resource at: {0}", sBitmapName.c_str());
		return nullptr;
	}

	//godot::Godot::print("[impl_CreateSurfaceFromBitmap] SUCCESS {0}", sBitmapName.c_str());



	godot::TextureRect* pTextureRect = godot::TextureRect::_new();
	pTextureRect->set_name(sBitmapName.c_str());
	pTextureRect->set_texture(pTexture);
	pTextureRect->set_stretch_mode(godot::TextureRect::STRETCH_KEEP_ASPECT_COVERED);
	//pTextureRect->set_expand(true);

	LTELSurface* pSurface = new LTELSurface();
	pSurface->pTextureRect = pTextureRect;

	return (HSURFACE)pSurface;
}

HSURFACE impl_CreateSurfaceFromString(HDEFONT hFont, HSTRING hString,
	HDECOLOR hForeColor, HDECOLOR hBackColor,
	int extraPixelsX, int extraPixelsY)
{
	LTELSurface* pSurface = new LTELSurface();
	
	godot::Label* pLabel = godot::Label::_new();
	LTELString* pString = (LTELString*)hString;

	godot::Color* oColor = (godot::Color*)hForeColor;

	pLabel->set_name(pString->sData.c_str());
	pLabel->set_text((char*)pString->sData.c_str());

	pLabel->add_color_override("font_color", *oColor);

	godot::DynamicFont* pFont = (godot::DynamicFont*)hFont;
	pFont->set_spacing(godot::DynamicFont::SPACING_TOP, extraPixelsY);
	pFont->set_spacing(godot::DynamicFont::SPACING_BOTTOM, extraPixelsY);
	//? 
	pFont->set_spacing(godot::DynamicFont::SPACING_SPACE, extraPixelsX);
	pFont->set_spacing(godot::DynamicFont::SPACING_CHAR, extraPixelsX);

	pSurface->bIsText = true;
	pSurface->pLabel = pLabel;

	return (HSURFACE)pSurface;
}

DRESULT impl_DeleteSurface(HSURFACE hSurface)
{
	auto pSurface = (LTELSurface*)hSurface;
	delete pSurface;
	return DE_OK;
}

HSURFACE impl_GetScreenSurface()
{
	// We don't really need this yet, so just return a blank texture rect
	//static godot::TextureRect* pScreen = godot::TextureRect::_new();

	static LTELSurface* pScreen = new LTELSurface();
	pScreen->bIsScreen = true;

	return (HSURFACE)pScreen;
}

void impl_GetSurfaceDims(HSURFACE hSurf, DDWORD* pWidth, DDWORD* pHeight)
{
	auto pSurface = (LTELSurface*)hSurf;

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

	// TextureRect
	auto rTexture = pSurface->pTextureRect->get_texture();

	if (rTexture.is_null())
	{
		*pWidth = 0;
		*pHeight = 0;
		return;
	}

	auto vSize = rTexture->get_size();
	*pWidth = vSize.x;
	*pHeight = vSize.y;
}

DRESULT impl_FillRect(HSURFACE hDest, DRect* pRect, HDECOLOR hColor)
{
	LTELSurface* pDest = (LTELSurface*)hDest;

	godot::Control* pControl = godot::Object::cast_to<godot::Control>(g_pLTELClient->m_pGodotLink->get_node(CANVAS_NODE));

	if (!pControl)
	{
		return DE_ERROR;
	}

	godot::Vector2 vPos;
	godot::Vector2 vSize;

	if (!pRect)
	{
		vPos = godot::Vector2(0, 0);
		vSize = godot::Vector2(1024, 768);
	}
	else
	{
		vPos = godot::Vector2(pRect->left, pRect->top);
		vSize = godot::Vector2(pRect->right, pRect->bottom);
	}
	
	godot::ColorRect* pColorRect = godot::ColorRect::_new();
	pColorRect->set_size(vSize);
	pColorRect->set_position(vPos);

	auto pColor = (godot::Color*)hColor;

	// It's black!
	if (!pColor)
	{
		pColorRect->set_frame_color(godot::Color());
	}
	else
	{
		pColorRect->set_frame_color(*pColor);
	}

	

	if (pDest->bIsText)
	{
		pDest->pLabel->add_child(pColorRect);
		return DE_OK;
	}
	else if (pDest->bIsScreen)
	{
		pControl->add_child(pColorRect);
		return DE_OK;
	}

	pDest->pTextureRect->add_child(pColorRect);
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

	for (int i = 0; i < pChildren.size(); i++)
	{
		pControl->remove_child(pChildren[i]);
	}

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
	LTELSurface* pDest = (LTELSurface*)hDest;
	LTELSurface* pSrc = (LTELSurface*)hSrc;

	godot::Control* pControl = GDCAST(godot::Control, g_pLTELClient->m_pGodotLink->get_node(CANVAS_NODE));

	if (!pControl)
	{
		return DE_ERROR;
	}

	auto vPos = godot::Vector2(pDestRect->left, pDestRect->top);
	auto vSize = godot::Vector2(pDestRect->left - pDestRect->right, pDestRect->top - pDestRect->bottom);

	godot::Node* pNode = GDCAST(godot::Node, pControl);

	// Screen!
	if (!pDest->bIsScreen)
	{
		if (pDest->bIsText)
		{
			pNode = GDCAST(godot::Node, pDest->pLabel);
		}
		else
		{
			pNode = GDCAST(godot::Node, pDest->pTextureRect);
		}
	}

	if (pSrc->bIsText)
	{
		pSrc->pLabel->set_position(vPos);
		pSrc->pLabel->set_size(vSize);
		pNode->add_child(pSrc->pLabel);
	}
	else
	{
		pSrc->pTextureRect->set_position(vPos);
		pSrc->pTextureRect->set_size(vSize);
		pNode->add_child(pSrc->pTextureRect);
	}

	// Don't need this yet
	return DE_OK;
}

DRESULT impl_DrawSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc,
	DRect* pSrcRect, int destX, int destY)
{
	LTELSurface* pDest = (LTELSurface*)hDest;
	LTELSurface* pSrc = (LTELSurface*)hSrc;

	if (!pDest || !pSrc)
	{
		return DE_ERROR;
	}

	godot::Control* pControl = GDCAST(godot::Control,g_pLTELClient->m_pGodotLink->get_node(CANVAS_NODE));

	if (!pControl)
	{
		return DE_ERROR;
	}

	auto vPos = godot::Vector2(destX, destY);

	godot::Node* pNode = GDCAST(godot::Node, pControl);

	// Screen!
	if (!pDest->bIsScreen)
	{
		if (pDest->bIsText)
		{
			pNode = GDCAST(godot::Node, pDest->pLabel);
		}
		else
		{
			pNode = GDCAST(godot::Node, pDest->pTextureRect);
		}
	}

	if (pSrc->bIsText)
	{
		pSrc->pLabel->set_position(vPos);
		pNode->add_child(pSrc->pLabel);
	}
	else
	{
		pSrc->pTextureRect->set_position(vPos);
		pNode->add_child(pSrc->pTextureRect);
	}

	// Don't need this yet
	return DE_OK;
}

DRESULT impl_DrawSurfaceToSurfaceTransparent(HSURFACE hDest, HSURFACE hSrc,
	DRect* pSrcRect, int destX, int destY, HDECOLOR hColor)
{
	return impl_DrawSurfaceToSurface(hDest, hSrc, pSrcRect, destX, destY);
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

DRESULT impl_OptimizeSurface(HSURFACE hSurface, HDECOLOR hTransparentColor)
{
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
	pTextureRect->set_name("Empty");
	pTextureRect->set_size(godot::Vector2(width, height));
	pSurface->pTextureRect = pTextureRect;
	return (HSURFACE)pSurface;
}

DRESULT impl_GetBorderSize(HSURFACE hSurface, HDECOLOR hColor, DRect* pRect)
{
	pRect = { 0 };
	return DE_OK;
}

DRESULT impl_RenderObjects(HLOCALOBJ hCamera, HLOCALOBJ* pObjects, int nObjects)
{
	// This should make everything but the objects in this list invisible, but for now we do nothing!
	// This is mainly for interfaces, like the main menu.

	// Because of how game code updates polygrid's height map, 
	// we need to finish up and apply the new depth texture!
	for (auto pObj : g_pPolygridsToUpdate)
	{
		// Oops, somehow it's empty? Skip!
		if (!pObj)
		{
			continue;
		}

		LTELPolyGrid* pExtraData = (LTELPolyGrid*)pObj->pExtraData;
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
				// DETAIL
				//
				auto pColourValues = pExtraData->pColorTable[x * y];
				godot::Color oDetailColor = godot::Color((float)pColourValues.x / 255.0f, (float)pColourValues.y / 255.0f, (float)pColourValues.z / 255.0f, (float)pColourValues.a / 255.0f);
				pDetailImage->set_pixel(x, y, oDetailColor);

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
	FillRect = impl_FillRect;
	EndOptimized2D = impl_EndOptimized2D;
	End3D = impl_End3D;
	FlipScreen = impl_FlipScreen;

	RenderObjects = impl_RenderObjects;

	GetRenderMode = impl_GetRenderMode;
}