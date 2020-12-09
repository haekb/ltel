#include "client.h"
#include "helpers.h"
#include <vector>

#include <Godot.hpp>
#include <Camera.hpp>
#include <Spatial.hpp>
#include <Mesh.hpp>
#include <MeshInstance.hpp>
#include <Quat.hpp>
#include <Image.hpp>
#include <ImageTexture.hpp>
#include <Texture.hpp>
#include <Material.hpp>
#include <SpatialMaterial.hpp>
#include <ResourceLoader.hpp>
#include <JSON.hpp>
#include <JSONParseResult.hpp>
#include <PackedScene.hpp>
#include <Animation.hpp>
#include <AnimationPlayer.hpp>

#include "shared.h"
#include "model_helper.h"
#include "node_linker.h"

#define USRFLG_VISIBLE					(1<<0)
#define USRFLG_NIGHT_INFRARED			(1<<1)
#define USRFLG_IGNORE_PROJECTILES		(1<<2)

extern LTELClient* g_pLTELClient;

std::vector<GameObject*> g_pPolygridsToUpdate;

#define INVALID_ANI				((HMODELANIM)-1)


HLOCALOBJ impl_CreateObject(ObjectCreateStruct* pStruct)
{
	GameObject* pObject = new GameObject(nullptr, nullptr);
	
	godot::Spatial* p3DNode = godot::Object::cast_to<godot::Spatial>(g_pLTELClient->m_pGodotLink->get_node("/root/Scene/3D"));
	NodeLinker* pContainer = NodeLinker::_new();

	godot::String sContainerName = pStruct->m_Name;
	sContainerName += " - CONTAINER";
	pContainer->set_name(sContainerName);
	
	// Setup our circular references
	pContainer->SetGameObject(pObject);
	pObject->SetContainer(pContainer);

	p3DNode->add_child(pContainer);

	switch (pStruct->m_ObjectType)
	{
	case OT_CAMERA:
		pObject->SetCamera(godot::Object::cast_to<godot::Camera>(g_pLTELClient->m_pGodotLink->get_node("/root/Scene/Camera")));
		break;
		// For now we don't care about sprites
	case OT_WORLDMODEL:
	{
		auto pSpatial = GDCAST(godot::Spatial, g_pLTELClient->m_pGodotLink->get_node("/root/Scene/DemoScene/Floor"));
		pObject->SetNode(pSpatial);
	}
		break;
	case OT_SPRITE:
	{
		std::string sSprite = g_pLTELClient->m_sGameDataDir + pStruct->m_Filename;

		auto pSprite = g_pLTELClient->LoadSPR(sSprite);
		auto pNode = godot::Spatial::_new();
		pNode->set_name("Sprite Container");

		// Add it into the world!
		pContainer->add_child(pNode);
		pObject->SetNode(pNode);

		pNode->add_child(pSprite);
	}
		break;
	case OT_NORMAL:
	{
		auto pNode = godot::Spatial::_new();

		// Add it into the world!
		pContainer->add_child(pNode);
		pObject->SetNode(pNode);
	}
		break;
	case OT_POLYGRID:
	{
		// Grab our in-scene prefab
		auto pMeshInstance = godot::Object::cast_to<godot::MeshInstance>(g_pLTELClient->m_pGodotLink->get_node("/root/Scene/Prefabs/PolyGrid"));

		// Duplicate it!
		pMeshInstance = godot::Object::cast_to<godot::MeshInstance>(pMeshInstance->duplicate());

		pContainer->add_child(pMeshInstance);

		pObject->SetPolyGrid(pMeshInstance);
	}
		break;
	case OT_MODEL:
	{
		std::string sABC = g_pLTELClient->m_sGameDataDir + pStruct->m_Filename;
		std::string sDTX = g_pLTELClient->m_sGameDataDir + pStruct->m_SkinName;

		auto pScene = g_pLTELClient->LoadABC(sABC.c_str());
		
		if (pScene.is_null())
		{
			delete pObject;
			return nullptr;
		}

		godot::Spatial* pModel = GDCAST(godot::Spatial, pScene->instance());

		if (!pModel)
		{
			delete pObject;
			return nullptr;
		}

		auto pTexture = g_pLTELClient->LoadDTX(sDTX.c_str());

		// TODO: Don't fail, replace with white texture
		if (pTexture.is_null())
		{
			delete pObject;
			return nullptr;
		}

		// ABC Scene
		pModel->set_name("ABC");
		pContainer->add_child(pModel, false);
		pObject->SetNode(pModel);

		// Init extra data
		LTELModel* pExtraData = new LTELModel();
		pObject->SetExtraData(pExtraData);

		// Skeleton
		auto pSkeleton = pModel->get_child(0);
		pSkeleton->set_name("Skeleton");
		pExtraData->pSkeleton = GDCAST(godot::Skeleton, pSkeleton);

		// Root -> Skeleton -> MeshInstance
		// Messy, but get_node isn't working??
		godot::MeshInstance* pPiece = GDCAST(godot::MeshInstance, pSkeleton->get_child(0));

		pExtraData->pMesh = pPiece;

		auto pMat = godot::SpatialMaterial::_new();
		pMat->set_texture(godot::SpatialMaterial::TEXTURE_ALBEDO, pTexture);
		pPiece->set_surface_material(0, pMat);

		godot::AnimationPlayer* pAnimationPlayer = GDCAST(godot::AnimationPlayer, pModel->get_child(1));

		if (pAnimationPlayer)
		{
			// Handy for later!
			pExtraData->pAnimationPlayer = pAnimationPlayer;

			// Setup animation list - This should maybe be handled by LTELModel
			auto sAnimList = pAnimationPlayer->get_animation_list();

			for (int i = 0; i < sAnimList.size(); i++)
			{
				pExtraData->vAnimationList.push_back(sAnimList[i].alloc_c_string());
			}
			// End

			// Kick things off by playing the first animation from the list
			auto pList = pAnimationPlayer->get_animation_list();

			if (!pAnimationPlayer->get_animation(pList[0]).is_null())
			{
				pAnimationPlayer->get_animation(pList[0])->set_loop(false);
				pAnimationPlayer->play(pList[0]);
			}
		}

	}
		break;
	default:
		pContainer->queue_free();
		delete pObject;

		godot::Godot::print("Trying to make some other object! aaaaaaaaaaaaa");
		return nullptr;
	}

	pObject->SetFromObjectCreateStruct(*pStruct);

	if (pObject->IsType(OT_MODEL))
	{
		ModelHelper* pModelHelper = ModelHelper::_new();
		pModelHelper->SetGameObject(pObject);
		pObject->GetNode()->add_child(pModelHelper);
	}

	return (HLOCALOBJ)pObject;
}

DRESULT impl_DeleteObject(HLOCALOBJ hObj)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObject = (GameObject*)hObj;

	godot::Spatial* pNode = pObject->GetNode();

	if (!pNode)
	{
		return DE_ERROR;
	}

	delete pObject;
	pObject = nullptr;
	return DE_OK;
}

void impl_EulerRotateX(DRotation* pRotation, float amount)
{
	godot::Quat vQuat = godot::Quat(pRotation->m_Vec.x, pRotation->m_Vec.y, pRotation->m_Vec.z, pRotation->m_Spin);
	auto vEuler = vQuat.get_euler();

	vQuat.set_axis_angle(godot::Vector3(1.0f, 0.0f, 0.0f), amount);
	pRotation->m_Vec.x = vQuat.x;
	pRotation->m_Vec.y = vQuat.y;
	pRotation->m_Vec.z = vQuat.z;
	pRotation->m_Spin = vQuat.w;//1.0f;
}

DDWORD impl_GetObjectFlags(HLOCALOBJ hObj)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		return 0;
	}

	return pObj->GetFlags();
}

void impl_SetObjectFlags(HLOCALOBJ hObj, DDWORD flags)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		return;
	}

	pObj->SetFlags(flags);
}

DRESULT impl_GetObjectUserFlags(HLOCALOBJ hObj, DDWORD* pFlags)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		return DE_ERROR;
	}

	*pFlags = pObj->GetUserFlags();
	return DE_OK;
}

DRESULT impl_SetObjectUserFlags(HLOCALOBJ hObj, DDWORD flags)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		return DE_ERROR;
	}

	godot::Spatial* pNode = pObj->GetNode();

	if (!pNode)
	{
		return DE_ERROR;
	}

	// We only support this right now!
	pNode->set_visible(flags & USRFLG_VISIBLE);
	pObj->SetUserFlags(flags);
	return DE_OK;
}

//
// Camera stuff
//
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
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		return;
	}

	godot::Camera* pCamera = pObj->GetCamera();

	if (!pCamera)
	{
		return;
	}

	float fFOV = godot::Math::rad2deg(fovX);
	float fIgnore = godot::Math::rad2deg(fovY);

	godot::Godot::print("[impl_SetCameraFOV] Setting FOV to {0}, ignoring Y value {1}", fFOV, fIgnore);

	g_pLTELClient->m_vFOV = godot::Vector2(fFOV, fIgnore);

	pCamera->set_fov(fFOV);
}

void impl_GetCameraFOV(HLOCALOBJ hObj, float* pX, float* pY)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		*pX = 0.0f;
		*pY = 0.0f;
		return;
	}

	godot::Camera* pCamera = pObj->GetCamera();

	if (!pCamera)
	{
		*pX = 0.0f;
		*pY = 0.0f;
		return;
	}

	auto vFov = g_pLTELClient->m_vFOV;

	*pX = godot::Math::deg2rad(vFov.x);
	*pY = godot::Math::deg2rad(vFov.y);
}

DBOOL impl_SetupPolyGrid(HLOCALOBJ hObj, DDWORD width, DDWORD height, DBOOL bHalfTrianges)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		return DE_ERROR;
	}

	auto pExtraData = new LTELPolyGrid(width, height);
	pObj->SetExtraData(pExtraData);

	return DE_OK;
}

DRESULT impl_FitPolyGrid(HLOCALOBJ hObj, DVector* pMin, DVector* pMax, DVector* pPos, DVector* pScale)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj || !pObj->GetExtraData())
	{
		return DE_ERROR;
	}

	LTELPolyGrid* pExtraData = (LTELPolyGrid*)pObj->GetExtraData();

	DVector pNewPos = (*pMax - *pMin) + *pMin;

	*pPos = pNewPos;

	// Just fake it for now.
	*pScale = DVector(15.0f, 15.0f, 15.0f);

	return DE_OK;
}

DRESULT impl_SetPolyGridTexture(HLOCALOBJ hObj, char* pFilename)
{
	// This will take in a .spr
	godot::Godot::print("[impl_SetPolyGridTexture] Set texture to {0}", pFilename);

	auto pObj = HObject2GameObject(hObj);

	if (!pObj || !pObj->GetPolyGrid())
	{
		return DE_ERROR;
	}

	// TODO: Move this into a helper function..
	std::string sBitmapName = pFilename;
	if (!replace(sBitmapName, ".spr", ".png") || !replace(sBitmapName, "Sprites", "spritetexture"))
	{
		godot::Godot::print("[impl_SetPolyGridTexture] Failed to replace spr with png! String: {0}", pFilename);
		return DE_ERROR;
	}

	std::string sResourcePath = "res://shogo/" + sBitmapName;

	auto pResourceLoader = godot::ResourceLoader::get_singleton();
	
	godot::Ref<godot::SpatialMaterial> pMat = godot::SpatialMaterial::_new();
	LTELPolyGrid* pExtraData = (LTELPolyGrid*)pObj->GetExtraData();

	// Albedo
	godot::Ref<godot::Texture> pTexture = pResourceLoader->load(sResourcePath.c_str());
	pMat->set_texture(godot::SpatialMaterial::TEXTURE_ALBEDO, pTexture);

	// I'm not sure if this actually is a blend texture...
	// Detail (Colour Table)
	godot::Ref<godot::ImageTexture> pDetail = godot::ImageTexture::_new();
	godot::Ref<godot::Image> pDetailImage = godot::Image::_new();

	pDetail->create(16, 16, godot::Image::FORMAT_RGB8);
	pDetailImage->create(16, 16, false, godot::Image::FORMAT_RGB8);

	// Small hack: This isn't how the effect is recreated, but I'm at a loss right now.
	pDetailImage->fill(godot::Color(0.25f, 0.25f, 0.25f, 1.0f));

	pDetail->set_data(pDetailImage);
	
	// Disable this for now...
	pMat->set_feature(godot::SpatialMaterial::FEATURE_DETAIL, true);
	pMat->set_texture(godot::SpatialMaterial::TEXTURE_DETAIL_ALBEDO, pDetail);

	pMat->set_detail_blend_mode(godot::SpatialMaterial::BLEND_MODE_SUB);
	
	pExtraData->pColormap = pDetail;

	// Depth
	godot::Ref<godot::ImageTexture> pDepth = godot::ImageTexture::_new();
	godot::Ref<godot::Image> pDepthImage = godot::Image::_new();

	pDepth->create(16, 16, godot::Image::FORMAT_RGB8);
	pDepthImage->create(16, 16, false, godot::Image::FORMAT_RGB8);

	pDepth->set_data(pDepthImage);

	pMat->set_feature(godot::SpatialMaterial::FEATURE_DEPTH_MAPPING, true);
	pMat->set_texture(godot::SpatialMaterial::TEXTURE_DEPTH, pDepth);
	pMat->set_texture(godot::SpatialMaterial::TEXTURE_DETAIL_MASK, pDepth);

	pExtraData->pHeightmap = pDepth;

	// Setup the game code's image buffer sandbox
	pExtraData->pData = (char*)malloc(pDepthImage->get_data().size());
	memset(pExtraData->pData, 0, sizeof(pDepthImage->get_data().size()));

	// Make sure it's un-lit
	pMat->set_specular(0.0f);
	pMat->set_flag(godot::SpatialMaterial::FLAG_USE_VERTEX_LIGHTING, true);
	//pMat->set_flag(godot::SpatialMaterial::FLAG_UNSHADED, true);

	// Set the material to the mesh
	pObj->GetPolyGrid()->set_surface_material(0, pMat);
	
	return DE_OK;
}

DRESULT impl_GetPolyGridTextureInfo(HLOCALOBJ hObj, float* xPan, float* yPan, float* xScale, float* yScale)
{
	*xPan = 1.0f;
	*yPan = 1.0f;
	*xScale = 1.0f;
	*yScale = 1.0f;

	return DE_OK;
}

DRESULT impl_SetPolyGridTextureInfo(HLOCALOBJ hObj, float xPan, float yPan, float xScale, float yScale)
{
	//godot::Godot::print("xPan: {0}\nyPan: {1}\nxScale: {2}\nyScale: {3}", xPan, yPan, xScale, yScale);
	return DE_OK;
}

DRESULT impl_GetPolyGridInfo(HLOCALOBJ hObj, char** pBytes, DDWORD* pWidth, DDWORD* pHeight, PGColor** pColorTable)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj || !pObj->GetExtraData())
	{
		return DE_ERROR;
	}

	LTELPolyGrid* pExtraData = (LTELPolyGrid*)pObj->GetExtraData();

	// We want to get the image data, and copy it to our bare char* pointer. 
	// This allows the game code to modify it
	auto pImage = pExtraData->pHeightmap->get_data();
	auto pData = pImage->get_data();
	memcpy(pExtraData->pData, pData.read().ptr(), pData.size());

	// Heightmap info - basic heightmap image that can be passed into Depth pass of a SpatialShader!
	*pBytes = pExtraData->pData;

	*pWidth = pExtraData->nWidth;
	*pHeight = pExtraData->nHeight;
	*pColorTable = pExtraData->pColorTable;

	// We need to keep track of this polygrid so we can upload the texture later...
	g_pPolygridsToUpdate.push_back(pObj);

	return DE_OK;
}

// RGB 0-1.
void impl_GetObjectColor(HLOCALOBJ hObject, float* r, float* g, float* b, float* a)
{
	auto pObj = HObject2GameObject(hObject);

	if (!pObj)
	{
		return;
	}

	godot::Spatial* pNode = pObj->GetNode();

	if (!pNode)
	{
		return;
	}

	*r = 1.0f;
	*g = 1.0f;
	*b = 1.0f;
	*a = 1.0f;
}

void impl_SetObjectColor(HLOCALOBJ hObject, float r, float g, float b, float a)
{
	auto pObj = HObject2GameObject(hObject);

	if (!pObj)
	{
		return;
	}

	bool bHeloo = true;
// Not right now!
}

DBOOL impl_GetModelLooping(HLOCALOBJ hObj)
{
	if (!hObj)
	{
		return FALSE;
	}

	GameObject* pObj = (GameObject*)hObj;
	LTELModel* pExtraData = (LTELModel*)pObj->GetExtraData();

	if (!pExtraData || !pExtraData->pAnimationPlayer)
	{
		return FALSE;
	}

	auto sAnim = pExtraData->vAnimationList.at(pExtraData->nCurrentAnimIndex);

	// Loops are set per animation
	auto pAnim = pExtraData->pAnimationPlayer->get_animation(sAnim.c_str());
	return pAnim->has_loop();
}
void impl_SetModelLooping(HLOCALOBJ hObj, DBOOL bLoop)
{
	shared_SetModelLooping(hObj, bLoop);
}

DDWORD impl_GetObjectClientFlags(HLOCALOBJ hObj)
{
	if (!hObj)
	{
		return 0;
	}

	GameObject* pObj = (GameObject*)hObj;

	return pObj->GetClientFlags();
}

void impl_SetObjectClientFlags(HLOCALOBJ hObj, DDWORD flags)
{
	if (!hObj)
	{
		return;
	}

	GameObject* pObj = (GameObject*)hObj;

	pObj->SetClientFlags(flags);
}

// Returns the animation the model is currently on.  (DDWORD)-1 if none.
DDWORD impl_GetModelAnimation(HLOCALOBJ hObj)
{
	return shared_GetModelAnimation(hObj);
}
void impl_SetModelAnimation(HLOCALOBJ hObj, DDWORD iAnim)
{
	shared_SetModelAnimation(hObj, iAnim);
}

HMODELANIM impl_GetAnimIndex(HOBJECT hObj, char* pAnimName)
{
	return shared_GetAnimIndex(hObj, pAnimName);
}

DRESULT impl_GetAttachments(HLOCALOBJ hObj, HLOCALOBJ* inList, DDWORD inListSize,
	DDWORD* outListSize, DDWORD* outNumAttachments)
{
	*outListSize = 0;
	*outNumAttachments = 0;

	// Clear the heckin' array
	for (int i = 0; i < inListSize; i++)
	{
		inList[i] = nullptr;
	}

	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;

	auto pAttachments = pObj->GetAttachments();

	// Set our number of attachments
	int nLength = pAttachments.size();
	*outNumAttachments = nLength;

	// If we have more attachments than the in list size,
	// set the for loop length to that in size.
	if (nLength > inListSize)
	{
		nLength = inListSize;
	}

	for (int i = 0; i < nLength; i++)
	{
		inList[i] = (HLOCALOBJ)pAttachments[i]->pObj;
		(*outListSize)++;
	}

	return DE_OK;
}

//
// Object Get/Sets
//

void impl_GetObjectPos(HLOCALOBJ hObj, DVector* pPos)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		*pPos = DVector(0, 0, 0);
		return;
	}

	auto vPos = pObj->GetPosition();

	pPos->x = vPos.x;
	pPos->y = vPos.y;
	pPos->z = vPos.z;
}

void impl_SetObjectPos(HLOCALOBJ hObj, DVector* pPos)
{
	auto pObj = HObject2GameObject(hObj);

	if (!pObj)
	{
		return;
	}

	pObj->SetPosition(*pPos);
}

DRESULT impl_SetObjectScale(HLOCALOBJ hObj, DVector* pScale)
{
	return shared_SetObjectScale(hObj, pScale);
}

void impl_GetObjectRotation(HLOCALOBJ hObj, DRotation* pRotation)
{
	shared_GetObjectRotation(hObj, pRotation);
}

void impl_SetObjectRotation(HLOCALOBJ hObj, DRotation* pRotation)
{
	if (!hObj)
	{
		return;
	}

	GameObject* pObj = (GameObject*)hObj;

	pObj->SetRotation(*pRotation);
}

void impl_SetObjectPosAndRotation(HLOCALOBJ hObj, DVector* pPos, DRotation* pRotation)
{
	impl_SetObjectPos(hObj, pPos);
	impl_SetObjectRotation(hObj, pRotation);
}

//
// End Object Get/Sets
//

DDWORD impl_GetModelPlaybackState(HLOCALOBJ hObj)
{
	return shared_GetModelPlaybackState(hObj);
}

DRESULT impl_ResetModelAnimation(HLOCALOBJ hObj)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;
	LTELModel* pExtraData = (LTELModel*)pObj->GetExtraData();

	if (!pExtraData || !pExtraData->pAnimationPlayer || pExtraData->nCurrentAnimIndex == -1)
	{
		return DE_ERROR;
	}

	pExtraData->pAnimationPlayer->seek(0, true);

	return DE_OK;
}

// This must be last!
void LTELClient::InitObjectImpl()
{
	// Object functionality
	CreateObject = impl_CreateObject;
	DeleteObject = impl_DeleteObject;
	GetObjectColor = impl_GetObjectColor;
	SetObjectColor = impl_SetObjectColor;
	GetObjectPos = impl_GetObjectPos;
	SetObjectPos = impl_SetObjectPos;
	SetObjectScale = impl_SetObjectScale;
	GetObjectRotation = impl_GetObjectRotation;
	SetObjectRotation = impl_SetObjectRotation;
	SetObjectPosAndRotation = impl_SetObjectPosAndRotation;
	EulerRotateX = impl_EulerRotateX;

	SetObjectFlags = impl_SetObjectFlags;
	GetObjectFlags = impl_GetObjectFlags;
	GetObjectUserFlags = impl_GetObjectUserFlags;
	SetObjectUserFlags = impl_SetObjectUserFlags;
	GetObjectClientFlags = impl_GetObjectClientFlags;
	SetObjectClientFlags = impl_SetObjectClientFlags;

	GetAttachments = impl_GetAttachments;

	// Animation
	GetModelLooping = impl_GetModelLooping;
	SetModelLooping = impl_SetModelLooping;
	GetModelAnimation = impl_GetModelAnimation;
	SetModelAnimation = impl_SetModelAnimation;
	GetAnimIndex = impl_GetAnimIndex;
	GetModelPlaybackState = impl_GetModelPlaybackState;
	ResetModelAnimation = impl_ResetModelAnimation;

	// Polygrid
	SetupPolyGrid = impl_SetupPolyGrid;
	FitPolyGrid = impl_FitPolyGrid;
	SetPolyGridTexture = impl_SetPolyGridTexture;
	GetPolyGridTextureInfo = impl_GetPolyGridTextureInfo;
	SetPolyGridTextureInfo = impl_SetPolyGridTextureInfo;
	GetPolyGridInfo = impl_GetPolyGridInfo;

	// Camera functionality
	GetCameraRect = impl_GetCameraRect;
	SetCameraRect = impl_SetCameraRect;
	SetCameraFOV = impl_SetCameraFOV;
	GetCameraFOV = impl_GetCameraFOV;
}


