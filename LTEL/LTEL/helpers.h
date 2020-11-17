#pragma once
#include "client.h"
#include <string>
#include <Spatial.hpp>
#include <Camera.hpp>
#include <MeshInstance.hpp>
#include <Quat.hpp>
#include <Texture.hpp>
#include <ImageTexture.hpp>

// Defs
#define GDCAST(casting_to, object) godot::Object::cast_to<casting_to>(object);

struct LTELString {
	LTELString(std::string sIncoming = "") {
		sData = sIncoming;
	}

	std::string sData;
};

struct LTELObject {
	LTELObject() {
		pData.pNode = nullptr;
		pExtraData = nullptr;
		nObjectType = OT_NORMAL;
		nObjectFlags = 0;
		nUserFlags = 0;
		pServerObject = nullptr;
	};

	// OT_*
	int nObjectType;

	int nObjectFlags;
	int nUserFlags;

	void* pExtraData;
	void* pServerObject;

	// Camera:
	union {
		godot::Spatial* pNode; // Normies only
		godot::Camera* pCamera;
		godot::MeshInstance* pPolyGrid;

	} pData;
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

// helpers
bool replace(std::string& str, const std::string& from, const std::string& to);

godot::Color LT2GodotColor(HDECOLOR hColor);

godot::Quat LT2GodotQuat(DRotation* pDRotation);

LTELObject* HObject2LTELObject(HOBJECT hObj);

