#pragma once
#include "LT1/AppHeaders/basedefs_de.h"
#include "LT1/AppHeaders/cpp_servershell_de.h"
#include "LT1/AppHeaders/cpp_engineobjects_de.h"

// Godot
#include <Node.hpp>
#include <Camera.hpp>
#include <MeshInstance.hpp>
#include <KinematicBody.hpp>
#include <CollisionShape.hpp>
#include <KinematicCollision.hpp>
// End Godot

#include <string>
#include <unordered_map>

// For uuid
#include <objbase.h>

class LTELAttachment;

class GameObject
{
public:
	GameObject(ClassDef* pClass, BaseClass* pBaseClass);
	~GameObject();

	void Update(float fDelta);

	void Teleport(DVector vNewPos);

	void SendAnimationCommandString(godot::String sCommandString);

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

	void SetClientFlags(int nFlag) { m_nClientFlags = nFlag; }
	uint32_t GetClientFlags() { return m_nClientFlags; }

	void SetPosition(DVector vPos, bool bLocalUpdate = false);
	DVector GetPosition();

	void SetScale(DVector vScale);
	DVector GetScale();

	void SetRotation(DRotation qRot);
	DRotation GetRotation();

	void SetNextUpdate(float fVal) { m_fNextUpdate = fVal; }
	float GetNextUpdate() { return m_fNextUpdate; }

	bool IsType(uint16_t nType) { return m_nObjectType == nType; }

	void SetExtraData(void* pData) { m_pExtraData = pData; }
	void* GetExtraData() { return m_pExtraData; }

	std::string GetFilename() { return m_sFilename; }
	std::string GetSkinname() { return m_sSkinname; }
	std::string GetName() { return m_sName; }

	// Data set!
	void SetNode(godot::Spatial* pNode) { m_pNode = pNode; }
	void SetCamera(godot::Camera* pCam) { m_pCamera = pCam; }
	void SetPolyGrid(godot::MeshInstance* pMesh) { m_pPolyGrid = pMesh; }

	void SetKinematicBody(godot::KinematicBody* pBody);

	// Data get!
	godot::Spatial* GetNode();
	godot::Camera* GetCamera() { return m_pCamera; }
	godot::MeshInstance* GetPolyGrid() { return m_pPolyGrid; }

	godot::KinematicBody* GetKinematicBody() { return m_pKinematicBody; }

	//std::unordered_map<std::string, GenericProp*> m_mProps;

	float GetFrictionCoeff() { return m_fFrictionCoeff; }
	float GetForceIgnoreLimit() { return m_fForceIgnoreLimit; }
	float GetMass() { return m_fMass; }
	DVector GetVelocity() { return m_vVelocity; }
	DVector GetAccel() { return m_vAccel; }
	DVector GetDims() { return m_vDims; }
	godot::Ref<godot::KinematicCollision> GetLastCollision() { return m_pLastCollision; }
	float GetPhysicsDeltaTime() { return m_fPhysicsDeltaTime; }

	void SetFrictionCoeff(float fVal) { m_fFrictionCoeff = fVal; }
	void SetForceIgnoreLimit(float fVal) { m_fForceIgnoreLimit = fVal; }
	void SetMass(float fVal) { m_fMass = fVal; }
	void SetVelocity(DVector vVal) { m_vVelocity = vVal; }
	void SetAccel(DVector vVal) { m_vAccel = vVal; }
	void SetDims(DVector vVal);
	void SetLastCollision(godot::Ref<godot::KinematicCollision> pCol) { m_pLastCollision = pCol; }
	void SetPhysicsDeltaTime(float fTime) { m_fPhysicsDeltaTime = fTime; }

	// Server stuff
	void SetClassDef(ClassDef* pClass) { m_pClassDef = pClass; }
	void SetBaseClass(BaseClass* pClass) { m_pBaseClass = pClass; }

	void QueueForDeletion() { m_bQueuedForDeletion = true; }
	bool IsQueuedForDeletion() { return m_bQueuedForDeletion; }

	std::vector<LTELAttachment*> GetAttachments() { return m_pAttachments; }
	bool AddAttachment(LTELAttachment* pObj);
	bool RemoveAttachment(LTELAttachment* pObj);

	GUID GetID() { return m_gID; }
	void SetID(GUID guid) { m_gID = guid; }

protected:
	GUID m_gID;

	bool m_bQueuedForDeletion;

	int m_nState;
	uint16_t m_nObjectType;
	uint32_t m_nFlags;
	uint32_t m_nUserFlags;
	uint32_t m_nClientFlags;
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
	float m_fPhysicsDeltaTime;

	BaseClass* m_pBaseClass;
	ClassDef* m_pClassDef;

	void* m_pExtraData;
	void* m_pServerObject;

	std::vector<LTELAttachment*> m_pAttachments;

	// Camera:
	//union {
	godot::Spatial* m_pNode; // Normies only
	godot::Camera* m_pCamera;
	godot::MeshInstance* m_pPolyGrid;
	//} m_pData;

	// Player only?
	godot::KinematicBody* m_pKinematicBody;
	godot::Ref<godot::KinematicCollision> m_pLastCollision;

	// Physics junk
	float m_fFrictionCoeff;
	float m_fForceIgnoreLimit;
	DVector m_vVelocity;
	DVector m_vAccel;
	float m_fMass;
	DVector m_vDims;

};

