#pragma once
#include <string>

#include <Godot.hpp>
#include <Object.hpp>
#include <Node.hpp>
#include <Spatial.hpp>
#include <Camera.hpp>
#include <MeshInstance.hpp>
#include <Quat.hpp>
#include <Texture.hpp>
#include <ImageTexture.hpp>
#include <AnimationPlayer.hpp>
#include <Skeleton.hpp>
#include <BoneAttachment.hpp>

#include <Label.hpp>
#include <TextureRect.hpp>

#include "client_info.h"
#include "game_object.h"

// Defs
#define CANVAS_NODE "/root/Scene/Canvas"
#define GDCAST(casting_to, object) godot::Object::cast_to<casting_to>(object);

// Custom message ids
#define MID_SEND_TO_OBJ 253
#define MID_SFX_MSG 254
#define MID_SFX_INSTANT_MSG 255

enum SurfaceType {
	ST_UNKNOWN = 0,	// Unknown value
	ST_AIR = 1,	// No a surface, but not the sky either
	ST_CHAINFENCE = 9,	// Chain link fence	
	ST_STONE = 10,	// Stone
	ST_STONE_HEAVY = 11,	// Heavy stone
	ST_STONE_LIGHT = 12,	// Light stone
	ST_METAL = 20,	// Metal
	ST_METAL_HEAVY = 21,	// Heavy metal
	ST_METAL_LIGHT = 22,	// Light metal
	ST_METAL_HOLLOW = 23,	// Hollow metal
	ST_METAL_HOLLOW_HEAVY = 24,	// Hollow heavy metal
	ST_METAL_HOLLOW_LIGHT = 25,	// Hollow light metal
	ST_WOOD = 30,	// Wood
	ST_DENSE_WOOD = 31,	// Dense wood
	ST_LIGHT_WOOD = 32,	// Light wood
	ST_GLASS = 40,	// Glass
	ST_ENERGY = 50,	// Energy (Force fields, etc)
	ST_BUILDING = 60,
	ST_TERRAIN = 70,	// Dirt, ice, etc.
	ST_CLOTH = 80,	// Cloth, carpet, furniture
	ST_PLASTIC = 90,	// Linoleum, teflon
	ST_PLASTIC_HEAVY = 91,	// Heavy plastic
	ST_PLASTIC_LIGHT = 92,	// Light plastic
	ST_FLESH = 100,
	ST_SKY = 110,

	ST_MECHA = 200,
	ST_LIQUID = 201
};


struct LTELSurface 
{

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

struct LTELString {
	LTELString(std::string sIncoming = "") {
		sData = sIncoming;
	}

	std::string sData;
};

// Extra Data
struct LTELPolyGrid {
	LTELPolyGrid(int width = 0, int height = 0) {
		nWidth = width;
		nHeight = height;
		pData = nullptr;
		pColorTable = nullptr;
		bLocked = false;
		pHeightmap = nullptr;

		// Quick init, this should maybe not be here...
		if (nWidth * nHeight > 0)
		{
			int pgColorSize = nWidth * nHeight * sizeof(PGColor);
			int pDataSize = nWidth * nHeight * sizeof(char*);

			pColorTable = (PGColor*)malloc(pgColorSize);
			//pData = (char*)malloc(pDataSize);

			memset((void*)pColorTable, 0, pgColorSize);
			//memset((void*)pData, 0, pDataSize);
		}
	}

	godot::Ref<godot::ImageTexture> pColormap;
	godot::Ref<godot::ImageTexture> pHeightmap;

	bool bLocked;
	char* pData; // RGB?
	int nWidth;
	int nHeight;
	PGColor* pColorTable;
};

struct LTELModel {
	LTELModel() {
		// Animation
		pAnimationPlayer = nullptr;
		bLoop = false;
		nCurrentAnimIndex = -1;

		// Skeleton
		pSkeleton = nullptr;
		pMesh = nullptr;
	}

	// Animation
	bool bLoop;
	int nCurrentAnimIndex;

	godot::AnimationPlayer* pAnimationPlayer;
	std::vector<std::string> vAnimationList;

	// Skeleton
	godot::Skeleton* pSkeleton;
	godot::MeshInstance* pMesh;
};

struct LTELAttachment {
	LTELAttachment() {
		pParent = nullptr;
		pObj = nullptr;

		pSpatialContainer = nullptr;
		pBoneAttachment = nullptr;
	}

	// Object the attachment is attached too!
	GameObject* pParent;

	// That's us!
	GameObject* pObj;

	// Only one of these will be something, so just nullcheck
	godot::Spatial* pSpatialContainer;
	godot::BoneAttachment* pBoneAttachment;

};

// helpers
bool replace(std::string& str, const std::string& from, const std::string& to);

godot::Color LT2GodotColor(HDECOLOR hColor);

godot::Vector3 LT2GodotVec3(DVector pVector);
godot::Quat LT2GodotQuat(DRotation* pDRotation);

GameObject* HObject2GameObject(HOBJECT hObj);

