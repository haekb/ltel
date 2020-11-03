#pragma once
#include "client.h"
#include <string>
#include <Spatial.hpp>
#include <Camera.hpp>
#include <MeshInstance.hpp>
#include <Quat.hpp>

// Defs
struct LTELObject {
	LTELObject() {
		pData.pNode = nullptr;
		pExtraData = nullptr;
		nObjectType = OT_NORMAL;
		nObjectFlags = 0;
		nUserFlags = 0;
	};

	// OT_*
	int nObjectType;

	int nObjectFlags;
	int nUserFlags;

	void* pExtraData;

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

		// Quick init, this should maybe not be here...
		if (nWidth * nHeight > 0)
		{
			int pgColorSize = nWidth * nHeight * sizeof(PGColor);
			int pDataSize = nWidth * nHeight * sizeof(char*);

			pColorTable = (PGColor*)malloc(pgColorSize);
			pData = (char*)malloc(pDataSize);

			memset((void*)pColorTable, 0, pgColorSize);
			memset((void*)pData, 0, pDataSize);
		}
	}
	char* pData; // RGB?
	int nWidth;
	int nHeight;
	PGColor* pColorTable;
};

// helpers
bool replace(std::string& str, const std::string& from, const std::string& to);

godot::Quat LT2GodotQuat(DRotation* pDRotation);

LTELObject* HObject2LTELObject(HOBJECT hObj);