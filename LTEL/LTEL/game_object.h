#pragma once
#include "LT1/AppHeaders/basedefs_de.h"
#include "LT1/AppHeaders/cpp_servershell_de.h"
#include "LT1/AppHeaders/cpp_engineobjects_de.h"

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

	void SetFlags(int nFlag) { m_nFlags = nFlag; }
	uint32_t GetFlags() { return m_nFlags; }

	void SetPosition(DVector vPos) { m_vPos = vPos; }
	DVector GetPosition() { return m_vPos; }

	void SetNextUpdate(float fVal) { m_fNextUpdate = fVal; }
	float GetNextUpdate() { return m_fNextUpdate; }


	//std::unordered_map<std::string, GenericProp*> m_mProps;

protected:
	int m_nState;
	uint16_t m_nObjectType;
	uint32_t m_nFlags;
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
};

