#pragma once
#include "LT1/AppHeaders/basedefs_de.h"
#include "LT1/AppHeaders/cpp_servershell_de.h"
#include "LT1/AppHeaders/cpp_engineobjects_de.h"

// Godot
#include <Node.hpp>
#include <Camera.hpp>
#include <MeshInstance.hpp>
// End Godot

#include <string>
#include <unordered_map>

class GameObject
{
public:
	GameObject(ClassDef* pClass, BaseClass* pBaseClass);
	~GameObject();

	void Teleport(DVector vNewPos);

	void SetFromObjectCreateStruct(ObjectCreateStruct pStruct);

	bool GetProperty(std::string sName, GenericProp* pProp);

	BaseClass* GetBaseClass() { return m_pBaseClass; }
	ClassDef* GetClassDef() { return m_pClassDef; }

	void SetState(int nState) { m_nState = nState; }
	int GetState() { return m_nState; }

	void SetType(uint16_t nType) { m_nObjectType = nType; }
	uint16_t GetType() { return m_nObjectType; }

	void SetFlags(int nFlag);
	uint32_t GetFlags() { return m_nFlags; }

	void SetUserFlags(int nFlag) { m_nUserFlags = nFlag; }
	uint32_t GetUserFlags() { return m_nUserFlags; }

	void SetPosition(DVector vPos) { m_vPos = vPos; }
	DVector GetPosition() { return m_vPos; }

	void SetNextUpdate(float fVal) { m_fNextUpdate = fVal; }
	float GetNextUpdate() { return m_fNextUpdate; }

	bool IsType(uint16_t nType) { return m_nObjectType == nType; }

	void SetExtraData(void* pData) { m_pExtraData = pData; }
	void* GetExtraData() { return m_pExtraData; }

	// Data set!
	void SetNode(godot::Spatial* pNode) { m_pData.pNode = pNode; }
	void SetCamera(godot::Camera* pCam) { m_pData.pCamera = pCam; }
	void SetPolyGrid(godot::MeshInstance* pMesh) { m_pData.pPolyGrid = pMesh; }

	// Data get!
	godot::Spatial* GetNode();
	godot::Camera* GetCamera() { return m_pData.pCamera; }
	godot::MeshInstance* GetPolyGrid() { return m_pData.pPolyGrid; }

	//std::unordered_map<std::string, GenericProp*> m_mProps;

protected:
	int m_nState;
	uint16_t m_nObjectType;
	uint32_t m_nFlags;
	uint32_t m_nUserFlags;
	DVector m_vPos;
	DVector m_vScale;
	DRotation m_vRotation;
	short m_nContainerCode;

	void* m_pUserData;

	std::string m_sFilename;
	std::string m_sSkinname;
	std::string m_sName;

	float m_fNextUpdate;
	float m_fDeactivationTime;

	BaseClass* m_pBaseClass;
	ClassDef* m_pClassDef;

	void* m_pExtraData;
	void* m_pServerObject;

	// Camera:
	union {
		godot::Spatial* pNode; // Normies only
		godot::Camera* pCamera;
		godot::MeshInstance* pPolyGrid;

	} m_pData;
};

