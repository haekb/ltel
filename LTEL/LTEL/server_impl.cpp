#include "server.h"
#include "shared.h"

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

void* simpl_GetClientUserData(HCLIENT hClient)
{
	// We don't have a client user yet...
	return nullptr;
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
	g_pLTELServer->StartWorld(pszWorldFileName);

	g_pLTELServer->m_pServerShell->OnAddClient((HCLIENT)g_pLTELServer->m_pClientList[0]);
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

	return pClient->GetFlags();
}

DBOOL simpl_GetClientRefName(HCLIENTREF hClient, char* pName, int maxLen)
{
	int nIndex = (int)hClient - 1;
	ClientInfo* pClient = *(g_pLTELServer->m_pClientList.begin() + nIndex);

	return simpl_GetClientName((HCLIENT)pClient, pName, maxLen);
}

HOBJECT simpl_GetClientRefObject(HCLIENTREF hClient)
{
	return nullptr;
}

LPBASECLASS simpl_HandleToObject(HOBJECT hObject)
{
	if (!hObject)
	{
		return nullptr;
	}

	return nullptr;
}

void LTELServer::InitFunctionPointers()
{
	// Object functionality
	CreateObjectList = simpl_CreateObjectList;
	GetNextObject = simpl_GetNextObject;
	GetNextInactiveObject = simpl_GetNextInactiveObject;
	RelinquishList = simpl_RelinquishList;
	HandleToObject = simpl_HandleToObject;

	// System/IO functionality
	BPrint = simpl_BPrint;
	CPrint = simpl_CPrint;
	
	// String functionality
	RunGameConString = simpl_RunGameConString;
	CreateString = simpl_CreateString;
	GetStringData = simpl_GetStringData;
	FreeString = simpl_FreeString;

	// Network functionality
	
	// Client functionality
	GetClientName = simpl_GetClientName;
	GetNextClientRef = simpl_GetNextClientRef;
	GetClientRefInfoFlags = simpl_GetClientRefInfoFlags;
	GetClientRefName = simpl_GetClientRefName;
	GetClientRefObject = simpl_GetClientRefObject;

	// Game State functionality
	GetGameInfo = simpl_GetGameInfo;
	GetClientUserData = simpl_GetClientUserData;
	LoadWorld = simpl_LoadWorld;
	RunWorld = simpl_RunWorld;
	GetClass = simpl_GetClass;


}

