#include "server.h"
#include "shared.h"
#include "LT1/AppHeaders/cpp_engineobjects_de.h"

#include <File.hpp>
#include <BoneAttachment.hpp>
#include <RayCast.hpp>
#include <SpatialMaterial.hpp>
#include <SpriteBase3D.hpp>

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
	return g_pLTELServer->CreateObjectListFromVector(g_pLTELServer->m_pObjectList);
}

void simpl_RelinquishList(ObjectList* pList)
{
	auto pObjectLink = pList->m_pFirstLink;
	while (pObjectLink)
	{
		// Get a copy of the pointer so we can delete it after we iterate
		auto pCurrentLink = pObjectLink;

		// Iterate...
		pObjectLink = pObjectLink->m_pNext;

		// Delete!
		pCurrentLink->m_hObject = nullptr;
		delete pCurrentLink;
	}

	// Clean up the rest
	pList->m_pFirstLink = nullptr;
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

	godot::Godot::print("Adding world model");
	{
		auto pClass = g_pLTELServer->GetClass((char*)"WorldModel");

		if (!pClass)
		{
			godot::Godot::print("Failed to create world");
			return DE_ERROR;
		}

		ObjectCreateStruct ocs = { 0 };
		strcpy_s(ocs.m_Name, 100, "World");
		ocs.m_ObjectType = OT_WORLDMODEL;
		ocs.m_Pos = DVector(0, -100, 0);
		ocs.m_Rotation = DRotation(0, 0, 0, 1);
		ocs.m_Scale = DVector(5, 5, 5);
		ocs.m_NextUpdate = 0.1f;
		ocs.m_Flags = FLAG_VISIBLE | FLAG_SOLID;
		auto pBaseClass = g_pLTELServer->CreateObject(pClass, &ocs);

		g_pLTELServer->m_pWorldObject = (GameObject*)pBaseClass->m_hObject;

	}
	return DE_OK;
}

DRESULT simpl_RunWorld()
{
	return DE_OK;
}

HCLASS simpl_GetClass(char* pName)
{
	//godot::Godot::print("[simpl_GetClass] Class requested: {0}", pName);

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
#if 1	
		GameObject* pObject = new GameObject(pClass, pBaseClass);

		pObject->SetFromObjectCreateStruct(*pStruct);

		// Grab the instance so we can get a reference to its godot data
		GameObject* pClientObject = (GameObject*)g_pLTELServer->m_pClientList[0]->GetClient()->CreateObject(pStruct);
		pClientObject->SetBaseClass(pBaseClass);
		pClientObject->SetClassDef(pClass);

		// Give it our server id
		pClientObject->SetID(pObject->GetID());

		pObject->SetCamera(pClientObject->GetCamera());
		pObject->SetNode(pClientObject->GetNode());
		pObject->SetExtraData(pClientObject->GetExtraData());

		pBaseClass->m_hObject = (HOBJECT)pObject;
#else
		GameObject* pObject = (GameObject*)g_pLTELServer->m_pClientList[0]->GetClient()->CreateObject(pStruct);
		pObject->SetBaseClass(pBaseClass);
		pObject->SetClassDef(pClass);
#endif

		g_pLTELServer->m_pCurrentObject = pObject;

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
		pBaseClass->_EngineMsgFn(pBaseClass, MID_INITIALUPDATE, pStruct, INITIALUPDATE_NORMAL);

		return pBaseClass;
	}

	// Cleanup!
	free(pBaseClass);
	//delete pObject;
	g_pLTELServer->m_pCurrentObject = nullptr;

	return nullptr;
}

void simpl_RemoveObject(HOBJECT hObject)
{
	if (!hObject)
	{
		return;
	}

	GameObject* pObj = (GameObject*)hObject;

	auto pTest = g_pLTELServer->m_pObjectList;

	pObj->QueueForDeletion();
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
	std::vector<GameObject*> pList;

	for (auto pObj : g_pLTELServer->m_pObjectList)
	{
		if (pObj->GetName() == pName)
		{
			pList.push_back(pObj);
		}
	}

	return g_pLTELServer->CreateObjectListFromVector(pList);
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
	strcpy_s(pSkinName, skinBufLen, pObj->GetSkinname().c_str());
	return TRUE;
}

DDWORD simpl_GetPointContainers(DVector* pPoint, HLOCALOBJ* pList, DDWORD maxListSize)
{
	*pPoint = DVector(0, 0, 0);
	*pList = nullptr;
	return 0;
}

DRESULT simpl_SaveObjects(char* pszSaveFileName, ObjectList* pList, DDWORD dwParam, DDWORD flags)
{
	auto pObjectLink = pList->m_pFirstLink;

	auto pFile = godot::File::_new();
	pFile->open(pszSaveFileName, godot::File::WRITE);

	// Store a "version"
	pFile->store_32(1);

	while (pObjectLink)
	{
		GameObject* pGameObject = (GameObject*)pObjectLink->m_hObject;

		auto pMessage = shared_StartHMessageWrite();

		pGameObject->GetClassDef()->m_EngineMessageFn(pGameObject->GetBaseClass(), MID_SAVEOBJECT, pMessage, dwParam);

		godot::StreamPeerBuffer* pStream = (godot::StreamPeerBuffer * )pMessage;

		// Store a reference to the class name the size of the data, and the data.
		pFile->store_pascal_string(pGameObject->GetClassDef()->m_ClassName);
		pFile->store_32(pStream->get_data_array().size());
		pFile->store_buffer(pStream->get_data_array());

#if 0
		// Loading example:
		pFile->seek(0);

		godot::StreamPeerBuffer* pLoadStream = godot::StreamPeerBuffer::_new();

		auto szClass = pFile->get_pascal_string();
		auto nStreamSize = pFile->get_32();
		pLoadStream->set_data_array(pFile->get_buffer(nStreamSize));
		pLoadStream->free();
#endif

		shared_EndHMessageWrite(pMessage);

		// Iterate...
		pObjectLink = pObjectLink->m_pNext;
	}

	pFile->store_pascal_string("Fin");

	pFile->close();

	return DE_OK;
}

void simpl_PingObjects(HOBJECT hObj)
{

}

DBOOL simpl_IsCommandOn(HCLIENT hClient, int command)
{
	if (!hClient)
	{
		return false;
	}

	ClientInfo* pClient = (ClientInfo*)hClient;

	if (!pClient->GetClient())
	{
		return false;
	}

	return pClient->GetClient()->IsCommandOn(command);
}

DDWORD simpl_GetModelAnimation(HLOCALOBJ hObj)
{
	return shared_GetModelAnimation(hObj);
}

DDWORD simpl_GetModelPlaybackState(HLOCALOBJ hObj)
{
	return shared_GetModelPlaybackState(hObj);
}

void simpl_SetModelLooping(HLOCALOBJ hObj, DBOOL bLoop)
{
	shared_SetModelLooping(hObj, bLoop);
}

DDWORD simpl_GetObjectUserFlags(HOBJECT hObj)
{
	if (!hObj)
	{
		return 0;
	}

	GameObject* pObj = (GameObject*)hObj;

	return pObj->GetUserFlags();
}

DRESULT simpl_SetObjectUserFlags(HLOCALOBJ hObj, DDWORD flags)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;

	pObj->SetUserFlags(flags);

	return DE_OK;
}

DRESULT simpl_SetClientViewPos(HCLIENT hClient, DVector* pPos)
{
	// For sound controller?
	return DE_OK;
}

DRESULT simpl_PlaySound(PlaySoundInfo* pPlaySoundInfo)
{
	return DE_OK;
}

void simpl_SetBlockingPriority(HOBJECT hObj, DBYTE pri)
{

}

HCLASS simpl_GetObjectClass(HOBJECT hObject)
{
	return (HCLASS)((GameObject*)hObject)->GetClassDef();
}

DBOOL simpl_IsKindOf(HCLASS hClass, HCLASS hTest)
{
	auto pClassDef = (ClassDef*)hClass;
	auto pTestClassDef = (ClassDef*)hTest;

	auto pCurrentClassDef = pClassDef;
	while (pCurrentClassDef)
	{
		// TODO: Can we just do pointer test?
		if (_stricmp(pCurrentClassDef->m_ClassName, pTestClassDef->m_ClassName) == 0)
		{
			return DTRUE;
		}

		pCurrentClassDef = pCurrentClassDef->m_ParentClass;
	}

	return DFALSE;
}

void simpl_ScaleObject(HOBJECT hObj, DVector* pNewScale)
{
	shared_SetObjectScale(hObj, pNewScale);
}

HMODELANIM simpl_GetAnimIndex(HOBJECT hObj, char* pAnimName)
{
	return shared_GetAnimIndex(hObj, pAnimName);
}

DRESULT simpl_CreateInterObjectLink(HOBJECT hOwner, HOBJECT hLinked)
{
	return DE_OK;
}

DRESULT simpl_CreateAttachment(HOBJECT hParent, HOBJECT hChild, char* pNodeName,
	DVector* pOffset, DRotation* pRotationOffset, HATTACHMENT* pAttachment)
{
	if (!hParent || !hChild)
	{
		return DE_INVALIDPARAMS;
	}

	GameObject* pParent = (GameObject*)hParent;
	GameObject* pChild = (GameObject*)hChild;

	LTELAttachment* pLTELAttachment = nullptr;

	// No node name!
	if (pNodeName && !pParent->IsType(OT_MODEL))
	{
		return DE_INVALIDPARAMS;
	}
	else if (pNodeName)
	{
		LTELModel* pExtraData = (LTELModel*)pParent->GetExtraData();

		if (!pExtraData->pSkeleton)
		{
			return DE_NODENOTFOUND;
		}
		else if (!pExtraData->pSkeleton->find_bone(pNodeName))
		{
			return DE_NODENOTFOUND;
		}

		godot::BoneAttachment* pBoneAttachment = godot::BoneAttachment::_new();
		pExtraData->pSkeleton->add_child(pBoneAttachment);

		pBoneAttachment->set_bone_name(pNodeName);

		if (pChild->GetNode()->get_parent())
		{
			pChild->GetNode()->get_parent()->remove_child(pChild->GetNode());
		}
		
		pBoneAttachment->add_child(pChild->GetNode());

		pExtraData->pSkeleton->add_child(pBoneAttachment);

		pLTELAttachment = new LTELAttachment();
		pLTELAttachment->pBoneAttachment = pBoneAttachment;


		*pAttachment = (HATTACHMENT)pLTELAttachment;
	}
	else
	{
		godot::Spatial* pContainer = godot::Spatial::_new();
		pParent->GetNode()->add_child(pContainer);

		if (pChild->GetNode()->get_parent())
		{
			pChild->GetNode()->get_parent()->remove_child(pChild->GetNode());
		}

		pContainer->add_child(pChild->GetNode());

		pLTELAttachment = new LTELAttachment();
		pLTELAttachment->pSpatialContainer = pContainer;

		*pAttachment = (HATTACHMENT)pContainer;
	}

	pLTELAttachment->pObj = pChild;
	pLTELAttachment->pParent = pParent;

	pChild->SetPosition(*pOffset);
	pChild->SetRotation(*pRotationOffset);

	// Finally add a reference
	pParent->AddAttachment(pLTELAttachment);

	return DE_OK;
}

DRESULT simpl_FindAttachment(HOBJECT hParent, HOBJECT hChild, HATTACHMENT* hAttachment)
{
	if (!hParent || !hChild)
	{
		return DE_INVALIDPARAMS;
	}

	GameObject* pParent = (GameObject*)hParent;
	GameObject* pChild = (GameObject*)hChild;

	auto pAttachments = pParent->GetAttachments();

	for (auto pAttachment : pAttachments)
	{
		if (pAttachment->pObj == pChild)
		{
			*hAttachment = (HATTACHMENT)pAttachment;

			return DE_OK;
		}
	}
	
	hAttachment = nullptr;

	return DE_ERROR;
}

DRESULT simpl_RemoveAttachment(HATTACHMENT hAttachment)
{
	if (!hAttachment)
	{
		return DE_INVALIDPARAMS;
	}

	LTELAttachment* pAttachment = (LTELAttachment*)hAttachment;

	// Remove 'em from the parent list
	pAttachment->pParent->RemoveAttachment(pAttachment);

	// Remove the nodes
	if (pAttachment->pSpatialContainer)
	{
		pAttachment->pSpatialContainer->queue_free();
	}
	else
	{
		pAttachment->pBoneAttachment->queue_free();
	}

	// Finally remove the GameObject, and the attachment itself
	pAttachment->pObj->QueueForDeletion();
	delete pAttachment;

	return DE_OK;
}

DRESULT simpl_SetModelFilenames(HOBJECT hObj, char* pFilename, char* pSkinName)
{
	if (!hObj)
	{
		return DE_ERROR;
	}

	GameObject* pObj = (GameObject*)hObj;

	godot::Godot::print("Setting model filename: {0}, {1}", pFilename, pSkinName);

	return DE_OK;
}

DRESULT simpl_GetModelCommandString(HOBJECT hObj, char* pStr, DDWORD maxLen)
{
	// TODO: Expose command string
	memset(pStr, 0, maxLen);

	return DE_OK;
}

int	simpl_Parse(char* pCommand, char** pNewCommandPos, char* argBuffer, char** argPointers, int* nArgs)
{
	return shared_Parse(pCommand, pNewCommandPos, argBuffer, argPointers, nArgs);
}

void simpl_SetModelAnimation(HLOCALOBJ hObj, DDWORD iAnim)
{
	shared_SetModelAnimation(hObj, iAnim);
}

void simpl_BreakInterObjectLink(HOBJECT hOwner, HOBJECT hLinked)
{

}

DRESULT simpl_GetObjectRotation(HOBJECT hObj, DRotation* pRotation)
{
	shared_GetObjectRotation(hObj, pRotation);
	return DE_OK;
}

DBOOL simpl_IntersectSegment(IntersectQuery* pQuery, IntersectInfo* pInfo)
{
	pInfo->m_hObject = nullptr;
	pInfo->m_hPoly = INVALID_HPOLY;
	pInfo->m_Plane = DPlane(0, 0, 0, 1);
	pInfo->m_Point = DVector(0, 0, 0);
	pInfo->m_SurfaceFlags = 0;

	auto vFrom = LT2GodotVec3(pQuery->m_From);
	auto vTo = LT2GodotVec3(pQuery->m_To);

	godot::RayCast* pRaycast = godot::RayCast::_new();
	pRaycast->set_translation(vFrom);
	pRaycast->set_cast_to(vTo);
	pRaycast->set_enabled(true);
	g_pLTELServer->m_pGodotLink->add_child(pRaycast);

	// Do the actual cast
	pRaycast->force_raycast_update();

	auto pNode = pRaycast->get_collider();

	if (!pNode)
	{
		pRaycast->queue_free();
		return DFALSE;
	}

	auto vNormal = pRaycast->get_collision_normal();
	auto vPos = pRaycast->get_collision_point();

	// FIXME: This is temp, we'll need to add scripts to each node that loops back to their game object
	pInfo->m_hObject = g_pLTELServer->GetWorldObject();
	pInfo->m_hPoly = INVALID_HPOLY;
	pInfo->m_Plane = DPlane(vNormal.x, vNormal.y, vNormal.z, 1.0f);
	pInfo->m_Point = DVector(vPos.x, vPos.y, vPos.z);

	pRaycast->queue_free();

	return DTRUE;
}

void simpl_AlignRotation(DRotation* pRotation, DVector* pVector, DVector* pUp)
{
	godot::Vector3 vUp;

	if (!pUp)
	{
		vUp = godot::Vector3(0, 1, 0);
	}
	else
	{
		vUp = LT2GodotVec3(*pUp);
	}

	auto vNormal = LT2GodotVec3(*pVector);

	// No need!
	if (vNormal == vUp)
	{
		godot::Quat qRot = godot::Quat();
		qRot.set_euler(vUp);

		*pRotation = DRotation(qRot.x, qRot.y, qRot.z, qRot.w);
		return;
	}

	// Hacky!
	godot::Spatial* pTemp = godot::Spatial::_new();
	godot::Spatial* p3D = GDCAST(godot::Spatial, g_pLTELServer->m_pGodotLink->get_node("/root/Scene/3D"));
	p3D->add_child(pTemp);

	pTemp->look_at(vNormal, vUp);

	auto vRot = pTemp->get_rotation();
	godot::Quat qRot = godot::Quat();
	qRot.set_euler(vRot);

	*pRotation = DRotation(qRot.x, qRot.y, qRot.z, qRot.w);

	pTemp->queue_free();
}

DDWORD simpl_GetClientID(HCLIENT hClient)
{
	if (!hClient)
	{
		return -1;
	}

	ClientInfo* pClient = (ClientInfo*)hClient;

	return 0;
}

DBOOL simpl_SetObjectColor(HOBJECT hObject, float r, float g, float b, float a)
{
	if (!hObject)
	{
		return DFALSE;
	}

	GameObject* pObj = (GameObject*)hObject;

	if (!pObj->GetNode())
	{
		return DFALSE;
	}

	if (pObj->IsType(OT_MODEL))
	{
		LTELModel* pExtraData = (LTELModel*)pObj->GetExtraData();

		godot::MeshInstance* pPiece = GDCAST(godot::MeshInstance, pExtraData->pSkeleton->get_child(0));

		if (!pPiece)
		{
			return DFALSE;
		}

		// Grab the material
		godot::Ref<godot::SpatialMaterial> pMat = pPiece->get_surface_material(0);
		pMat->set_albedo(godot::Color(r, g, b, a));
		return DTRUE;
	}
	else if (pObj->IsType(OT_SPRITE))
	{
		godot::SpriteBase3D* pSprite3D = GDCAST(godot::SpriteBase3D, pObj->GetNode());
			
		// Grab the material
		godot::Ref<godot::SpatialMaterial> pMat = pSprite3D->get_material_override();
		pMat->set_albedo(godot::Color(r, g, b, a));
		return DTRUE;
	}

	return DFALSE;
}

ObjectList* simpl_FindObjectsTouchingSphere(DVector* pPosition, float radius)
{
	return nullptr;
}

void simpl_RotateAroundAxis(DRotation* pRotation, DVector* pAxis, float amount)
{
	DVector vRotatedAmount = (*pAxis) * amount;

	godot::Quat qRot = godot::Quat();
	qRot.set_euler(LT2GodotVec3(vRotatedAmount));
	// Gross no += operator
	*pRotation = DRotation(pRotation->m_Vec.x + qRot.x, pRotation->m_Vec.y + qRot.y, pRotation->m_Vec.z + qRot.z, pRotation->m_Spin + qRot.w);
}

void simpl_SetDeactivationTime(HOBJECT hObj, DFLOAT fDeactivationTime)
{
	if (!hObj)
	{
		return;
	}

	GameObject* pObj = (GameObject*)hObj;

	pObj->SetDeactivationTime(fDeactivationTime);
}

char* simpl_GetObjectName(HOBJECT hObject)
{
	if (!hObject)
	{
		return (char*)"";
	}

	GameObject* pObj = (GameObject*)hObject;

	return (char*)pObj->GetName().c_str();
}

DRESULT simpl_CacheFile(DDWORD fileType, char* pFilename)
{
	return DE_NOTFOUND;
}

DFLOAT simpl_Random(DFLOAT min, DFLOAT max)
{
	// Thanks https://stackoverflow.com/a/23868489
	return  (max - min) * ((((float)rand()) / (float)RAND_MAX)) + min;
}

DBOOL simpl_GetModelNodeTransform(HOBJECT hObj, char* pNodeName,
	DVector* pPos, DRotation* pRot)
{
	*pPos = DVector(0, 0, 0);
	*pRot = DRotation(0, 0, 0, 1);

	if (!hObj || !pNodeName)
	{
		return DFALSE;
	}

	GameObject* pObj = (GameObject*)hObj;

	if (!pObj->IsType(OT_MODEL))
	{
		return DFALSE;
	}

	LTELModel* pExtraData = (LTELModel*)pObj->GetExtraData();

	auto pSkeleton = pExtraData->pSkeleton;

	auto nBoneIndex = pSkeleton->find_bone(pNodeName);
	auto tTransform = pSkeleton->get_bone_pose(nBoneIndex);

	godot::Quat qRot = godot::Quat();
	qRot.set_euler(tTransform.basis.get_euler());

	*pPos = DVector(tTransform.origin.x, tTransform.origin.y, tTransform.origin.z);
	*pRot = DRotation(qRot.x, qRot.y, qRot.z, qRot.w);
	
	return DTRUE;
}

void LTELServer::InitFunctionPointers()
{
	// Audio functionality
	PlaySound = simpl_PlaySound;

	// Animation functionality
	GetModelAnimation = simpl_GetModelAnimation;
	GetModelPlaybackState = simpl_GetModelPlaybackState;
	SetModelLooping = simpl_SetModelLooping;
	GetAnimIndex = simpl_GetAnimIndex;
	SetModelAnimation = simpl_SetModelAnimation;

	// Object functionality
	CreateObject = simpl_CreateObject;
	RemoveObject = simpl_RemoveObject;
	CreateObjectList = simpl_CreateObjectList;
	GetNextObject = simpl_GetNextObject;
	GetNextInactiveObject = simpl_GetNextInactiveObject;
	RelinquishList = simpl_RelinquishList;
	HandleToObject = simpl_HandleToObject;
	FindNamedObjects = simpl_FindNamedObjects;
	TeleportObject = simpl_TeleportObject;
	SetNextUpdate = simpl_SetNextUpdate;
	GetModelFilenames = simpl_GetModelFilenames;
	GetObjectClass = simpl_GetObjectClass;
	IsKindOf = simpl_IsKindOf;
	ScaleObject = simpl_ScaleObject;
	CreateInterObjectLink = simpl_CreateInterObjectLink;
	BreakInterObjectLink = simpl_BreakInterObjectLink;
	SetModelFilenames = simpl_SetModelFilenames;
	GetModelCommandString = simpl_GetModelCommandString;
	SetDeactivationTime = simpl_SetDeactivationTime;
	GetObjectName = simpl_GetObjectName;
	GetModelNodeTransform = simpl_GetModelNodeTransform;

	CreateAttachment = simpl_CreateAttachment;
	FindAttachment = simpl_FindAttachment;
	RemoveAttachment = simpl_RemoveAttachment;

	AlignRotation = simpl_AlignRotation;
	RotateAroundAxis = simpl_RotateAroundAxis;

	// Physics?
	SetBlockingPriority = simpl_SetBlockingPriority;
	IntersectSegment = simpl_IntersectSegment;
	FindObjectsTouchingSphere = simpl_FindObjectsTouchingSphere;

	// Get/Sets
	GetObjectState = simpl_GetObjectState;
	SetObjectState = simpl_SetObjectState;
	GetObjectFlags = simpl_GetObjectFlags;
	SetObjectFlags = simpl_SetObjectFlags;
	GetObjectUserFlags = simpl_GetObjectUserFlags;
	SetObjectUserFlags = simpl_SetObjectUserFlags;
	GetObjectPos = simpl_GetObjectPos;
	SetObjectPos = simpl_SetObjectPos;
	GetObjectRotation = simpl_GetObjectRotation;
	SetObjectRotation = simpl_SetObjectRotation;
	SetObjectColor = simpl_SetObjectColor;

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
	Random = simpl_Random;
	
	CacheFile = simpl_CacheFile;

	// String functionality
	RunGameConString = simpl_RunGameConString;
	CreateString = simpl_CreateString;
	GetStringData = simpl_GetStringData;
	FreeString = simpl_FreeString;

	Parse = simpl_Parse;

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
	IsCommandOn = simpl_IsCommandOn;
	SetClientViewPos = simpl_SetClientViewPos;
	GetClientID = simpl_GetClientID;

	// Game State functionality
	GetGameInfo = simpl_GetGameInfo;
	LoadWorld = simpl_LoadWorld;
	RunWorld = simpl_RunWorld;
	GetClass = simpl_GetClass;
	SaveObjects = simpl_SaveObjects;
	PingObjects = simpl_PingObjects;


}

