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
//#define BLOCK_FONTS

#define CANVAS_NODE "/root/Scene/Canvas"

extern LTELClient* g_pLTELClient;
extern std::vector<LTELObject*> g_pPolygridsToUpdate;

struct LTELSurface {

	LTELSurface() {
		bQueuedForDeletion = false;
		bIsFontImage = false;
		bOptimized = false;

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

	bool bOptimized;
	bool bQueuedForDeletion;
	bool bIsText;
	bool bIsScreen;
	bool bIsFontImage;

	// If bIsText == false:
	godot::TextureRect* pTextureRect;
	// Else:
	godot::Label* pLabel;
};

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
	
	godot::ColorRect* pColorRect = godot::ColorRect::_new();
	pColorRect->set_size(vSize);
	pColorRect->set_position(vPos);

	godot::Color oColor = LT2GodotColor(hColor);

	pColorRect->set_frame_color(oColor);

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
	auto nSize = pChildren.size();

	for (int i = 0; i < pChildren.size(); i++)
	{

		pControl->remove_child(pChildren[i]);

		//godot::Node* pNode = GDCAST(godot::Node, pChildren[i]);

		//if (pNode)
		{
			//godot::Godot::print("Deleting node: {0}", pNode->get_name());

			//pNode->free();
		}
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
				pSurface->pLabel->queue_free();
			}
		}
		else
		{
			if (!pSurface->pTextureRect->is_inside_tree())
			{
				pSurface->pTextureRect->queue_free();
			}
		}

		delete pSurface;
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
	LTELSurface* pDest = (LTELSurface*)hDest;
	LTELSurface* pSrc = (LTELSurface*)hSrc;

	bool bCanBlit = false;

	godot::Control* pControl = GDCAST(godot::Control, g_pLTELClient->m_pGodotLink->get_node(CANVAS_NODE));

	if (!pControl)
	{
		return DE_ERROR;
	}

	auto vPos = godot::Vector2(pDestRect->left, pDestRect->top);
	auto vSize = godot::Vector2(pDestRect->right - pDestRect->left, pDestRect->bottom - pDestRect->top);

	godot::Node* pNode = GDCAST(godot::Node, pControl);

	// Screen!
	if (!pDest->bIsScreen)
	{
		//return DE_OK;
		if (pDest->bIsText)
		{
			pNode = GDCAST(godot::Node, pDest->pLabel);
		}
		else
		{
			pNode = GDCAST(godot::Node, pDest->pTextureRect);
			bCanBlit = true;
		}
	}

	if (pSrc->bIsText)
	{
		pSrc->pLabel->set_position(vPos);
		pSrc->pLabel->set_size(vSize);
		if (!pSrc->pLabel->get_parent())
		{
			pNode->add_child(pSrc->pLabel);
		}
	}
	else
	{
		if (bCanBlit)
		{
			godot::Ref<godot::ImageTexture> pDestTexture = pDest->pTextureRect->get_texture();
			auto pSrcTexture = pSrc->pTextureRect->get_texture();

			/*
			if (pSrc->bIsFontImage)
			{
				pSrcTexture->get_data()->save_png("Font.png");
				DebugBreak();
			}
			*/

			if (!pDestTexture.is_null() && !pSrcTexture.is_null())
			{
				//x,y,w,h
				godot::Rect2 rSrcRect = godot::Rect2();

				if (pSrcRect)
				{
					float nWidth = pSrcRect->right - pSrcRect->left;
					float nHeight = pSrcRect->bottom - pSrcRect->top;
					float x = pSrcRect->left;
					float y = pSrcRect->top;

					rSrcRect.set_size(godot::Vector2(nWidth, nHeight));
					rSrcRect.set_position(godot::Vector2(x, y));
				}
				else
				{
					rSrcRect.set_size(godot::Vector2(vSize.width, vSize.height));
					rSrcRect.set_position(godot::Vector2(vPos.x, vPos.y));
				}

				auto pSrcImage = pSrcTexture->get_data();
				pSrcImage->resize(vSize.width, vSize.height);

				godot::Vector2 vDestRect = godot::Vector2(vPos.x, vPos.y);
				auto pImage = pDestTexture->get_data();
				pImage->blit_rect(pSrcImage, rSrcRect, vDestRect);
				pDestTexture->set_data(pImage);

				/*
				if (pSrc->bIsFontImage)
				{
					pImage->save_png("Font.png");
					DebugBreak();
				}
				*/
			}
			else
			{
				bCanBlit = false;
			}
		}


		// Not else, because above path can fallback here!
		if (!bCanBlit)
		{
			pSrc->pTextureRect->set_position(vPos);
			if (!pSrc->pTextureRect->get_parent())
			{
				pNode->add_child(pSrc->pTextureRect);
			}
		}
	}

	// Don't need this yet
	return DE_OK;
}

DRESULT impl_DrawSurfaceToSurface(HSURFACE hDest, HSURFACE hSrc,
	DRect* pSrcRect, int destX, int destY)
{
	LTELSurface* pDest = (LTELSurface*)hDest;
	LTELSurface* pSrc = (LTELSurface*)hSrc;

	bool bCanBlit = false;

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
		//return DE_OK;
		if (pDest->bIsText)
		{
			pNode = GDCAST(godot::Node, pDest->pLabel);
		}
		else
		{
			pNode = GDCAST(godot::Node, pDest->pTextureRect);
			bCanBlit = true;
		}
	}

	if (pSrc->bIsText)
	{
		pSrc->pLabel->set_position(vPos);
		if (!pSrc->pLabel->get_parent())
		{
			pNode->add_child(pSrc->pLabel);
		}
	}
	else
	{
		if (bCanBlit)
		{
			godot::Ref<godot::ImageTexture> pDestTexture = pDest->pTextureRect->get_texture();
			auto pSrcTexture = pSrc->pTextureRect->get_texture();

			if (!pDestTexture.is_null() && !pSrcTexture.is_null())
			{
				//x,y,w,h
				godot::Rect2 rSrcRect = godot::Rect2();
				
				if (pSrcRect)
				{
					float nWidth = pSrcRect->right - pSrcRect->left;
					float nHeight = pSrcRect->bottom - pSrcRect->top;
					float x = pSrcRect->left;
					float y = pSrcRect->top;

					rSrcRect.set_size(godot::Vector2(nWidth, nHeight));
					rSrcRect.set_position(godot::Vector2(x, y));
				}
				else
				{
					rSrcRect.set_size(godot::Vector2(pSrcTexture->get_data()->get_width(), pSrcTexture->get_data()->get_height()));
					rSrcRect.set_position(godot::Vector2(0, 0));
				}

				

				godot::Vector2 vDestRect = godot::Vector2(destX, destY);
				auto pImage = pDestTexture->get_data();
				pImage->blit_rect(pSrcTexture->get_data(), rSrcRect, vDestRect);
				//pSrcTexture->get_data()->save_png("Src.png");
				//pImage->save_png("Dest.png");

				pDestTexture->set_data(pImage);
			}
			else
			{
				bCanBlit = false;
			}
		}
		

		// This actually doesn't work...
		// Not else, because above path can fallback here!
		if (!bCanBlit)
		{
			pSrc->pTextureRect->set_position(vPos);
			pSrc->pTextureRect->set_size(godot::Vector2(pSrc->pTextureRect->get_texture()->get_data()->get_width(), pSrc->pTextureRect->get_texture()->get_height()));


			pSrc->pTextureRect->get_texture()->get_data()->save_png("CantBlitTex.png");

			if (pSrc->pTextureRect->get_parent())
			{
				pSrc->pTextureRect->get_parent()->remove_child(pSrc->pTextureRect);
			}

			//if (!pSrc->pTextureRect->get_parent())
			{
				pNode->add_child(pSrc->pTextureRect);
			}
		}
	}

	// Don't need this yet
	return DE_OK;
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
	/*
	auto pVS = godot::VisualServer::get_singleton();
	auto bIsRenderLoopEnabled = pVS->call("get_render_loop_enabled");


	if (!bIsRenderLoopEnabled)
	{
		pVS->force_sync();
		pVS->force_draw();
	}
	*/

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
				//pDetailImage->set_pixel(x, y, oDepthColor);

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
	DrawBitmapToSurface = impl_DrawBitmapToSurface;
	DrawStringToSurface = impl_DrawStringToSurface;
	FillRect = impl_FillRect;
	EndOptimized2D = impl_EndOptimized2D;
	End3D = impl_End3D;
	FlipScreen = impl_FlipScreen;

	RenderObjects = impl_RenderObjects;

	GetRenderMode = impl_GetRenderMode;
}