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

	void SetFromObjectCreateStruct(ObjectCreateStruct pStruct);

	bool GetProperty(std::string sName, GenericProp* pProp);

	BaseClass* GetBaseClass() { return m_pBaseClass; }
	ClassDef* GetClassDef() { return m_pClassDef; }

	//std::unordered_map<std::string, GenericProp*> m_mProps;

protected:
	unsigned short m_nObjectType;
	unsigned int   m_nFlags;
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

