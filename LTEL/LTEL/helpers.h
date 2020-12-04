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
	}

	// Animation
	bool bLoop;
	int nCurrentAnimIndex;

	godot::AnimationPlayer* pAnimationPlayer;
	std::vector<std::string> vAnimationList;

	// Skeleton
	godot::Skeleton* pSkeleton;
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

