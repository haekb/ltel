#include "server.h"
#include "shared.h"
#include "LT1/AppHeaders/cpp_engineobjects_de.h"

extern LTELServer* g_pLTELServer;

void simpl_RunGameConString(char* pString)
{
	godot::Godot::print("[impl_RunGameConString] {0}", pString);
}

HSTRING simpl_CreateString(char* pString)
{
	return (HSTRING)shared_CreateString(pString);
}

char* simpl_GetStringData(HSTRING hString)
{
	return shared_GetStringData(hString);
}


DRESULT simpl_GetGameInfo(void** ppData, DDWORD* pLen)
{
	if (!g_pLTELServer->m_pGameInfo)
	{
		*ppData = nullptr;
		*pLen = 0;

		return DE_ERROR;
	}

	*pLen = sizeof(g_pLTELServer->m_pGameInfo);

	memcpy(*ppData, g_pLTELServer->m_pGameInfo, *pLen);
	return DE_OK;
}

ObjectList* simpl_CreateObjectList()
{
	return new ObjectList();
}

void simpl_RelinquishList(ObjectList* pList)
{
	delete pList;
}

void simpl_BPrint(char* pMsg, ...)
{
	va_list list;
	char szMessage[256] = "";


	va_start(list, pMsg);

	vsnprintf(szMessage, 256, pMsg, list);

	va_end(list);

	godot::Godot::print("[CONSOLE PRINT]: {0}", szMessage);
}

void simpl_CPrint(char* pMsg, ...)
{
	va_list list;
	char szMessage[256] = "";


	va_start(list, pMsg);

	vsnprintf(szMessage, 256, pMsg, list);

	va_end(list);

	godot::Godot::print("[CONSOLE PRINT]: {0}", szMessage);
}

DRESULT simpl_LoadWorld(char* pszWorldFileName, DDWORD flags)
{
	g_pLTELServer->m_pServerShell->OnAddClient((HCLIENT)g_pLTELServer->m_pClientList[0]);

	g_pLTELServer->StartWorld(pszWorldFileName);

	godot::Godot::print("Adding client!");

	return DE_OK;
}

DRESULT simpl_RunWorld()
{
	return DE_OK;
}

HCLASS simpl_GetClass(char* pName)
{
	godot::Godot::print("[simpl_GetClass] Class requested: {0}", pName);

	// TODO: Create a nice hash table so this is a fast lookup
	auto pClassDefinitions = g_pLTELServer->m_pClassDefList;
	ClassDef* pCurrentClassDef = *(pClassDefinitions);
	while (pCurrentClassDef)
	{
		if (_stricmp(pName, pCurrentClassDef->m_ClassName) == 0)
		{
			break;
		}

		pCurrentClassDef = *(++pClassDefinitions);
	}

	return (HCLASS)pCurrentClassDef;
}

HOBJECT simpl_GetNextObject(HOBJECT hObj)
{
	return nullptr;
}

HOBJECT simpl_GetNextInactiveObject(HOBJECT hObj)
{
	return nullptr;
}

void simpl_FreeString(HSTRING hString)
{
	return shared_FreeString(hString);
}

DBOOL simpl_GetClientName(HCLIENT hClient, char* pName, int maxLen)
{
	ClientInfo* pClient = (ClientInfo*)hClient;

	strcpy_s(pName, maxLen, pClient->GetName().c_str());

	return DTRUE;
}

HCLIENTREF simpl_GetNextClientRef(HCLIENTREF hRef)
{
	std::vector<ClientInfo*>::iterator pClientIt;
	int nIndex = (int)hRef;

	// Here for reference:
	// We start at 1, so we'd need to decrement by 1 to access this current ref, 
	// but since we want the next client we want to increment by one instead, keeping the value passed it!
	//nIndex -= 1;
	//nIndex += 1;

	// Check if we're empty, or if our current index would bring us over the limit
	if (g_pLTELServer->m_pClientList.size() == 0 || nIndex > g_pLTELServer->m_pClientList.size())
	{
		return nullptr;
	}

	if (hRef == nullptr)
	{
		return (HCLIENTREF)1;
	}

	return (HCLIENTREF)nIndex + 1;
}

DDWORD simpl_GetClientRefInfoFlags(HCLIENTREF hClient)
{
	int nIndex = (int)hClient - 1;
	ClientInfo* pClient = *(g_pLTELServer->m_pClientList.begin() + nIndex);

	if (!pClient)
	{
		return 0;
	}

	return pClient->GetFlags();
}

DBOOL simpl_GetClientRefName(HCLIENTREF hClient, char* pName, int maxLen)
{
	int nIndex = (int)hClient - 1;
	ClientInfo* pClient = *(g_pLTELServer->m_pClientList.begin() + nIndex);

	if (!pClient)
	{
		return false;
	}

	return simpl_GetClientName((HCLIENT)pClient, pName, maxLen);
}

HOBJECT simpl_GetClientRefObject(HCLIENTREF hClient)
{
	int nIndex = (int)hClient - 1;
	ClientInfo* pClient = *(g_pLTELServer->m_pClientList.begin() + nIndex);

	if (!pClient)
	{
		return nullptr;
	}

	return (HOBJECT)pClient->GetObj();
}

LPBASECLASS simpl_HandleToObject(HOBJECT hObject)
{
	if (!hObject)
	{
		return nullptr;
	}

	GameObject* pObj = (GameObject*)hObject;

	return pObj->GetBaseClass();
}

LPBASECLASS simpl_CreateObject(HCLASS hClass, struct ObjectCreateStruct_t* pStruct)
{
	ClassDef* pClass = (ClassDef*)hClass;

	if (!pClass)
	{
		return nullptr;
	}

	LPBASECLASS pBaseClass = (BaseClass*)malloc(pClass->m_ClassObjectSize);

	if (!pBaseClass)
	{
		return nullptr;
	}

	memset(pBaseClass, 0, pClass->m_ClassObjectSize);
	pBaseClass->m_pFirstAggregate = nullptr;
	pBaseClass->m_hObject = nullptr;

	pClass->m_ConstructFn(pBaseClass);

	// Bind the function for later use
	pClass->m_EngineMessageFn = pBaseClass->_EngineMsgFn;
	pClass->m_ObjectMessageFn = pBaseClass->_ObjectMsgFn;

	auto nResult = pBaseClass->_EngineMsgFn(pBaseClass, MID_PRECREATE, pStruct, PRECREATE_NORMAL);

	if (nResult == 1)
	{ 

		GameObject* pObject = new GameObject(pClass, pBaseClass);
		pBaseClass->m_hObject = (HOBJECT)pObject;

		pObject->SetFromObjectCreateStruct(*pStruct);

		g_pLTELServer->m_pCurrentObject = pObject;

		g_pLTELServer->m_pClientList[0]->GetClient()->CreateObject(pStruct);


		// We can safely add this to our object list!
		g_pLTELServer->m_pObjectList.push_back(pObject);

#if 0
		//
		// This code is a trap to catch heap corruption
		//
		std::vector <void*> pVoid;

		while (true)
		{
			void* pData = malloc(1024);
			pVoid.push_back(pData);
			//

			//break;
			Sleep(10);
		}

		// Actually not needed but nice to dream
		for (auto pData : pVoid)
		{
			free(pData);
			pData = nullptr;
		}

		pVoid.clear();
#endif

		// FIXME: We need to run initial update, as shown by this disassemble:
		// maybeOnEngineMessage((double)fStack0000002c,pBaseClass,1,pOCS);
		//auto nResult = pBaseClass->EngineMessageFn(MID_INITIALUPDATE, pStruct, INITIALUPDATE_NORMAL);
		//pBaseClass->_EngineMsgFn(pBaseClass, MID_INITIALUPDATE, pStruct, INITIALUPDATE_NORMAL);

		return pBaseClass;
	}

	// Cleanup!
	free(pBaseClass);
	//delete pObject;
	g_pLTELServer->m_pCurrentObject = nullptr;

	return nullptr;
}

// These are used to get the property values from the world file.
// The property names are case sensitive.  If the property doesn't exist,
// it will return DE_NOTFOUND.
DRESULT simpl_GetPropString(char* pPropName, char* pRet, int maxLen)
{
	return DE_NOTFOUND;
}
DRESULT simpl_GetPropVector(char* pPropName, DVector* pRet)
{
	return DE_NOTFOUND;
}
DRESULT simpl_GetPropColor(char* pPropName, DVector* pRet)
{
	return DE_NOTFOUND;
}
DRESULT simpl_GetPropReal(char* pPropName, float* pRet)
{
	return DE_NOTFOUND;
}
DRESULT simpl_GetPropFlags(char* pPropName, DDWORD* pRet)
{
	return DE_NOTFOUND;
}
DRESULT simpl_GetPropBool(char* pPropName, DBOOL* pRet)
{
	return DE_NOTFOUND;
}
DRESULT simpl_GetPropLongInt(char* pPropName, long* pRet)
{
	return DE_NOTFOUND;
}
DRESULT simpl_GetPropRotation(char* pPropName, DRotation* pRet)
{
	return DE_NOTFOUND;
}
DRESULT simpl_GetPropRotationEuler(char* pPropName, DVector* pAngles)
{
	return DE_NOTFOUND;
}


// Fills in the GenericProp for the different data types.  For a list
// of which property types map to which GenericProp variable, see 
// the GenericProp structure.
// Note: if the property exists, it always initializes the members in the prop first,
// so if the GenericProp variable doesn't support that property type, 
// it'll be zero (or ROT_INIT'd).
DRESULT simpl_GetPropGeneric(char* pPropName, GenericProp* pProp)
{
	memset(pProp, 0, sizeof(GenericProp));

	bool bResult = g_pLTELServer->m_pCurrentObject->GetProperty(pPropName, pProp);

	return bResult ? DE_OK : DE_ERROR;
}

// User data for HCLIENTs.
void	simpl_SetClientUserData(HCLIENT hClient, void* pData)
{
	if (!hClient)
	{
		return;
	}

	ClientInfo* pClient = (ClientInfo*)hClient;
	pClient->SetUserData(pData);
}
void* simpl_GetClientUserData(HCLIENT hClient)
{
	if (!hClient)
	{
		return nullptr;
	}

	ClientInfo* pClient = (ClientInfo*)hClient;
	return pClient->GetUserData();
}

ObjectList* simpl_FindNamedObjects(char* pName)
{
	return nullptr;
}

DRESULT simpl_TeleportObject(HOBJECT hObj, DVector* pNewPos)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObject = (GameObject*)hObj;
	pObject->Teleport(*pNewPos);

	return DE_OK;
}

void simpl_SetObjectState(HOBJECT hObj, int state)
{
	if (!hObj)
	{
		return;
	}

	GameObject* pObject = (GameObject*)hObj;
	pObject->SetState(state);
}
int	simpl_GetObjectState(HOBJECT hObj)
{
	if (!hObj)
	{
		return 0;
	}

	GameObject* pObject = (GameObject*)hObj;
	return pObject->GetState();
}

void simpl_SetObjectFlags(HLOCALOBJ hObj, DDWORD flags)
{
	if (!hObj)
	{
		return;
	}

	GameObject* pObject = (GameObject*)hObj;
	pObject->SetFlags(flags);
}

DDWORD simpl_GetObjectFlags(HLOCALOBJ hObj)
{
	if (!hObj)
	{
		return 0;
	}

	GameObject* pObject = (GameObject*)hObj;
	return pObject->GetFlags();
}

void simpl_GetObjectPos(HLOCALOBJ hObj, DVector* pPos)
{
	if (!hObj)
	{
		return;
	}

	GameObject* pObject = (GameObject*)hObj;
	*pPos = pObject->GetPosition();
}

void simpl_SetObjectPos(HLOCALOBJ hObj, DVector* pPos)
{
	if (!hObj)
	{
		return;
	}

	GameObject* pObject = (GameObject*)hObj;
	pObject->SetPosition(*pPos);
}

// A combination of the CIF_ flags above.
void simpl_SetClientInfoFlags(HCLIENT hClient, DDWORD dwClientFlags)
{
	if (!hClient)
	{
		return;
	}

	ClientInfo* pClient = (ClientInfo*)hClient;
	pClient->SetFlags(dwClientFlags);
}

DDWORD simpl_GetClientInfoFlags(HCLIENT hClient)
{
	if (!hClient)
	{
		return 0;
	}

	ClientInfo* pClient = (ClientInfo*)hClient;
	return pClient->GetFlags();
}

DDWORD simpl_GetServerFlags()
{
	return g_pLTELServer->m_nFlags;
}
DDWORD simpl_SetServerFlags(DDWORD flags)
{
	g_pLTELServer->m_nFlags = flags;
	return flags;
}

void simpl_SetNextUpdate(HOBJECT hObj, DFLOAT nextUpdate)
{
	BaseClass* pClass = (BaseClass*)hObj;

	// FIXME: This sucks! IDs maybe?
	for (auto pObj : g_pLTELServer->m_pObjectList)
	{
		if (pObj->GetBaseClass() != pClass)
		{
			continue;
		}

		pObj->SetNextUpdate(nextUpdate);
		break;
	}
}

float simpl_GetTime()
{
	return g_pLTELServer->m_fTime;
}
float simpl_GetFrameTime()
{
	return g_pLTELServer->m_fFrametime;
}


HCONVAR simpl_GetGameConVar(char* pName)
{
	//godot::Godot::print("[simpl_GetGameConVar] Wants to get command: {0}", pName);
	return nullptr;
}


float simpl_GetVarValueFloat(HCONVAR hVar)
{
	return 0.0f;
}

char* simpl_GetVarValueString(HCONVAR hVar)
{
	return nullptr;
}


DRESULT simpl_SetObjectRotation(HOBJECT hObj, DRotation* pRotation)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObject = (GameObject*)hObj;
	pObject->SetRotation(*pRotation);
	return DE_OK;
}

void simpl_SetGameConVar(char* pName, char* pVal)
{
	return;
}

DBOOL simpl_CastRay(ClientIntersectQuery* pQuery, ClientIntersectInfo* pInfo)
{
	// I'll need to implement geometry before I can cast to it!
	pInfo->m_hObject = nullptr;
	pInfo->m_hPoly = INVALID_HPOLY;
	pInfo->m_Plane = DPlane(0, 0, 0, 0);
	pInfo->m_Point = DVector(0, 0, 0);
	pInfo->m_SurfaceFlags = 0;
	return FALSE;
}

DDWORD simpl_GetObjectContainers(HOBJECT hObj, HOBJECT* pContainerList, DDWORD* pFlagList, DDWORD maxListSize)
{
	*pContainerList = nullptr;
	*pFlagList = 0;
	return 0;
}

DBOOL simpl_GetModelFilenames(HOBJECT hObj, char* pFilename, int fileBufLen, char* pSkinName, int skinBufLen)
{
	if (!hObj)
	{
		return FALSE;
	}

	GameObject* pObj = (GameObject*)hObj;

	strcpy_s(pFilename, fileBufLen, pObj->GetFilename().c_str());
	strcpy_s(pSkinName, skinBufLen, pObj->GetFilename().c_str());
	return TRUE;
}

DDWORD simpl_GetPointContainers(DVector* pPoint, HLOCALOBJ* pList, DDWORD maxListSize)
{
	*pPoint = DVector(0, 0, 0);
	*pList = nullptr;
	return 0;
}

void LTELServer::InitFunctionPointers()
{
	// Object functionality
	CreateObject = simpl_CreateObject;
	CreateObjectList = simpl_CreateObjectList;
	GetNextObject = simpl_GetNextObject;
	GetNextInactiveObject = simpl_GetNextInactiveObject;
	RelinquishList = simpl_RelinquishList;
	HandleToObject = simpl_HandleToObject;
	FindNamedObjects = simpl_FindNamedObjects;
	TeleportObject = simpl_TeleportObject;
	SetNextUpdate = simpl_SetNextUpdate;
	GetModelFilenames = simpl_GetModelFilenames;

	// Get/Sets
	GetObjectState = simpl_GetObjectState;
	SetObjectState = simpl_SetObjectState;
	GetObjectFlags = simpl_GetObjectFlags;
	SetObjectFlags = simpl_SetObjectFlags;
	GetObjectPos = simpl_GetObjectPos;
	SetObjectPos = simpl_SetObjectPos;
	GetObjectRotation = nullptr;
	SetObjectRotation = simpl_SetObjectRotation;

	GetPropGeneric = simpl_GetPropGeneric;
	GetPropString = simpl_GetPropString;
	GetPropVector = simpl_GetPropVector;
	GetPropColor = simpl_GetPropColor;
	GetPropReal = simpl_GetPropReal;
	GetPropFlags = simpl_GetPropFlags;
	GetPropBool = simpl_GetPropBool;
	GetPropLongInt = simpl_GetPropLongInt;
	GetPropRotation = simpl_GetPropRotation;
	GetPropRotationEuler = simpl_GetPropRotationEuler;

	// Container functionality
	GetObjectContainers = simpl_GetObjectContainers;
	GetPointContainers = simpl_GetPointContainers;

	// Raycasting functionality
	CastRay = simpl_CastRay;

	// System/IO functionality
	BPrint = simpl_BPrint;
	CPrint = simpl_CPrint;
	GetTime = simpl_GetTime;
	GetFrameTime = simpl_GetFrameTime;
	GetGameConVar = simpl_GetGameConVar;
	SetGameConVar = simpl_SetGameConVar;
	GetVarValueFloat = simpl_GetVarValueFloat;
	GetVarValueString = simpl_GetVarValueString;
	
	// String functionality
	RunGameConString = simpl_RunGameConString;
	CreateString = simpl_CreateString;
	GetStringData = simpl_GetStringData;
	FreeString = simpl_FreeString;

	// Network functionality
	
	// Server functionality
	GetServerFlags = simpl_GetServerFlags;
	SetServerFlags = simpl_SetServerFlags;

	// Client functionality
	GetClientName = simpl_GetClientName;
	GetNextClientRef = simpl_GetNextClientRef;
	GetClientRefInfoFlags = simpl_GetClientRefInfoFlags;
	GetClientRefName = simpl_GetClientRefName;
	GetClientRefObject = simpl_GetClientRefObject;
	GetClientUserData = simpl_GetClientUserData;
	SetClientUserData = simpl_SetClientUserData;
	GetClientInfoFlags = simpl_GetClientInfoFlags;
	SetClientInfoFlags = simpl_SetClientInfoFlags;

	// Game State functionality
	GetGameInfo = simpl_GetGameInfo;
	LoadWorld = simpl_LoadWorld;
	RunWorld = simpl_RunWorld;
	GetClass = simpl_GetClass;



}

