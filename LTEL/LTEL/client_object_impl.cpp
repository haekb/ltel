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



#define USRFLG_VISIBLE					(1<<0)
#define USRFLG_NIGHT_INFRARED			(1<<1)
#define USRFLG_IGNORE_PROJECTILES		(1<<2)

extern LTELClient* g_pLTELClient;

std::vector<LTELObject*> g_pPolygridsToUpdate;


HLOCALOBJ impl_CreateObject(ObjectCreateStruct* pStruct)
{
	LTELObject* pObject = new LTELObject();
	pObject->nObjectType = pStruct->m_ObjectType;
	pObject->nObjectFlags = pStruct->m_Flags;
	

	godot::Spatial* p3DNode = godot::Object::cast_to<godot::Spatial>(g_pLTELClient->m_pGodotLink->get_node("/root/Scene/3D"));

	switch (pStruct->m_ObjectType)
	{
	case OT_CAMERA:
		pObject->pData.pCamera = godot::Object::cast_to<godot::Camera>(g_pLTELClient->m_pGodotLink->get_node("/root/Scene/Camera"));
		break;
	case OT_NORMAL:
	{
		auto pNode = godot::Spatial::_new();

		// Add it into the world!
		p3DNode->add_child(pNode);

		pNode->set_translation(godot::Vector3(pStruct->m_Pos.x, pStruct->m_Pos.y, pStruct->m_Pos.z));
		godot::Quat vQuat = LT2GodotQuat(&pStruct->m_Rotation);
		auto vEuler = vQuat.get_euler();
		pNode->set_rotation(vEuler);
		pNode->set_scale(godot::Vector3(pStruct->m_Scale.x, pStruct->m_Scale.y, pStruct->m_Scale.z));
		pNode->set_visible(pStruct->m_Flags & FLAG_VISIBLE);

		pObject->pData.pNode = pNode;
	}
		break;
	case OT_POLYGRID:
	{
		//auto pMeshInstance = godot::MeshInstance::_new();
		//auto pMesh = godot::Mesh::_new();

		// Grab our in-scene prefab
		auto pMeshInstance = godot::Object::cast_to<godot::MeshInstance>(g_pLTELClient->m_pGodotLink->get_node("/root/Scene/Prefabs/PolyGrid"));

		// Duplicate it!
		pMeshInstance = godot::Object::cast_to<godot::MeshInstance>(pMeshInstance->duplicate());

		p3DNode->add_child(pMeshInstance);

		pMeshInstance->set_translation(godot::Vector3(pStruct->m_Pos.x, pStruct->m_Pos.y, pStruct->m_Pos.z));
		godot::Quat vQuat = LT2GodotQuat(&pStruct->m_Rotation);
		auto vEuler = vQuat.get_euler();

		// Polygrid is 90 degrees off...I think?
		vEuler.x += 90;

		pMeshInstance->set_rotation_degrees(vEuler);
		pMeshInstance->set_scale(godot::Vector3(pStruct->m_Scale.x, pStruct->m_Scale.y, pStruct->m_Scale.z));
		pMeshInstance->set_visible(pStruct->m_Flags & FLAG_VISIBLE);

		pObject->pData.pPolyGrid = pMeshInstance;
	}
		break;
	default:
		godot::Godot::print("Trying to make some other object! aaaaaaaaaaaaa");
	}


	return (HLOCALOBJ)pObject;
}

void impl_GetObjectPos(HLOCALOBJ hObj, DVector* pPos)
{
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		return;
	}

	godot::Spatial* pNode = pObj->pData.pNode;

	if (!pNode)
	{
		return;
	}

	auto vPos = pNode->get_translation();

	pPos->x = vPos.x;
	pPos->y = vPos.y;
	pPos->z = vPos.z;
}

void impl_SetObjectPos(HLOCALOBJ hObj, DVector* pPos)
{
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		return;
	}

	godot::Spatial* pNode = pObj->pData.pNode;

	if (!pNode)
	{
		return;
	}

	pNode->set_translation(godot::Vector3(pPos->x, pPos->y, pPos->z));
}

DRESULT impl_SetObjectScale(HLOCALOBJ hObj, DVector* pScale)
{
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		return DE_ERROR;
	}

	godot::Spatial* pNode = pObj->pData.pNode;

	if (!pNode)
	{
		return DE_ERROR;
	}

	pNode->set_scale(godot::Vector3(pScale->x, pScale->y, pScale->z));
	return DE_OK;
}

void impl_GetObjectRotation(HLOCALOBJ hObj, DRotation* pRotation)
{
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		return;
	}

	godot::Spatial* pNode = pObj->pData.pNode;

	if (!pNode)
	{
		return;
	}

	auto vRot = pNode->get_rotation();
	godot::Quat vQuat;
	vQuat.set_euler(vRot);

	pRotation->m_Vec.x = vQuat.x;
	pRotation->m_Vec.y = vQuat.y;
	pRotation->m_Vec.z = vQuat.z;
	pRotation->m_Spin = vQuat.w;//1.0f;
}

void impl_EulerRotateX(DRotation* pRotation, float amount)
{
	godot::Quat vQuat = godot::Quat(pRotation->m_Vec.x, pRotation->m_Vec.y, pRotation->m_Vec.z, pRotation->m_Spin);
	auto vEuler = vQuat.get_euler();

	vEuler = vEuler.rotated(godot::Vector3(1.0f, 0.0f, 0.0f), amount);

	vQuat.set_euler(vEuler);
	pRotation->m_Vec.x = vQuat.x;
	pRotation->m_Vec.y = vQuat.y;
	pRotation->m_Vec.z = vQuat.z;
	pRotation->m_Spin = vQuat.w;//1.0f;
}

DDWORD impl_GetObjectFlags(HLOCALOBJ hObj)
{
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		return 0;
	}

	return pObj->nObjectFlags;
}

void impl_SetObjectFlags(HLOCALOBJ hObj, DDWORD flags)
{
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		return;
	}

	pObj->nObjectFlags = flags;
}

DRESULT impl_GetObjectUserFlags(HLOCALOBJ hObj, DDWORD* pFlags)
{
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		return DE_ERROR;
	}

	*pFlags = pObj->nUserFlags;
}

DRESULT impl_SetObjectUserFlags(HLOCALOBJ hObj, DDWORD flags)
{
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		return DE_ERROR;
	}

	godot::Spatial* pNode = pObj->pData.pNode;

	if (!pNode)
	{
		return DE_ERROR;
	}

	// We only support this right now!
	pNode->set_visible(flags & USRFLG_VISIBLE);
	pObj->nUserFlags = flags;
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
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		return;
	}

	godot::Camera* pCamera = pObj->pData.pCamera;

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
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		*pX = 0.0f;
		*pY = 0.0f;
		return;
	}

	godot::Camera* pCamera = pObj->pData.pCamera;

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
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
	{
		return DE_ERROR;
	}

	auto pExtraData = new LTELPolyGrid(width, height);
	pObj->pExtraData = pExtraData;

	return DE_OK;
}

DRESULT impl_FitPolyGrid(HLOCALOBJ hObj, DVector* pMin, DVector* pMax, DVector* pPos, DVector* pScale)
{
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj || !pObj->pExtraData)
	{
		return DE_ERROR;
	}

	LTELPolyGrid* pExtraData = (LTELPolyGrid*)pObj->pExtraData;

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

	auto pObj = HObject2LTELObject(hObj);

	if (!pObj)
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
	LTELPolyGrid* pExtraData = (LTELPolyGrid*)pObj->pExtraData;

	// Albedo
	godot::Ref<godot::Texture> pTexture = pResourceLoader->load(sResourcePath.c_str());
	pMat->set_texture(godot::SpatialMaterial::TEXTURE_ALBEDO, pTexture);

	// I'm not sure if this actually is a blend texture...
	// Detail (Colour Table)
	godot::Ref<godot::ImageTexture> pDetail = godot::ImageTexture::_new();
	godot::Ref<godot::Image> pDetailImage = godot::Image::_new();

	pDetail->create(16, 16, godot::Image::FORMAT_RGB8);
	pDetailImage->create(16, 16, false, godot::Image::FORMAT_RGB8);

	pDetail->set_data(pDetailImage);
	
	// Disable this for now...
	//pMat->set_feature(godot::SpatialMaterial::FEATURE_DETAIL, true);
	pMat->set_texture(godot::SpatialMaterial::TEXTURE_DETAIL_ALBEDO, pDetail);
	pMat->set_detail_blend_mode(godot::SpatialMaterial::BLEND_MODE_MIX);
	
	pExtraData->pColormap = pDetail;

	// Depth
	godot::Ref<godot::ImageTexture> pDepth = godot::ImageTexture::_new();
	godot::Ref<godot::Image> pDepthImage = godot::Image::_new();

	pDepth->create(16, 16, godot::Image::FORMAT_RGB8);
	pDepthImage->create(16, 16, false, godot::Image::FORMAT_RGB8);

	pDepth->set_data(pDepthImage);

	pMat->set_feature(godot::SpatialMaterial::FEATURE_DEPTH_MAPPING, true);
	pMat->set_texture(godot::SpatialMaterial::TEXTURE_DEPTH, pDepth);

	pExtraData->pHeightmap = pDepth;

	// Setup the game code's image buffer sandbox
	pExtraData->pData = (char*)malloc(pDepthImage->get_data().size());
	memset(pExtraData->pData, 0, sizeof(pDepthImage->get_data().size()));

	// Make sure it's un-lit
	pMat->set_specular(0.0f);
	//pMat->set_flag(godot::SpatialMaterial::FLAG_UNSHADED, true);

	// Set the material to the mesh
	pObj->pData.pPolyGrid->set_surface_material(0, pMat);
	
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
	auto pObj = HObject2LTELObject(hObj);

	if (!pObj || !pObj->pExtraData)
	{
		return DE_ERROR;
	}

	LTELPolyGrid* pExtraData = (LTELPolyGrid*)pObj->pExtraData;

	// We want to get the image data, and copy it to our bare char* pointer. 
	// This allows the game code to modify it
	auto pImage = pExtraData->pHeightmap->get_data();
	auto pData = pImage->get_data();
	memcpy(pExtraData->pData, pData.read().ptr(), sizeof(pData.size()));

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
	auto pObj = HObject2LTELObject(hObject);

	if (!pObj)
	{
		return;
	}

	godot::Spatial* pNode = pObj->pData.pNode;

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
// Not right now!
}


// This must be last!
void LTELClient::InitObjectImpl()
{
	// Object functionality
	CreateObject = impl_CreateObject;
	GetObjectColor = impl_GetObjectColor;
	SetObjectColor = impl_SetObjectColor;
	GetObjectPos = impl_GetObjectPos;
	SetObjectPos = impl_SetObjectPos;
	SetObjectScale = impl_SetObjectScale;
	GetObjectRotation = impl_GetObjectRotation;
	EulerRotateX = impl_EulerRotateX;

	SetObjectFlags = impl_SetObjectFlags;
	GetObjectFlags = impl_GetObjectFlags;
	GetObjectUserFlags = impl_GetObjectUserFlags;
	SetObjectUserFlags = impl_SetObjectUserFlags;

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