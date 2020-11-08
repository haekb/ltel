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

void LTELServer::InitFunctionPointers()
{
	
	// String functionality
	RunGameConString = simpl_RunGameConString;
	CreateString = simpl_CreateString;

	// Network functionality
	
	// Game State functionality
	GetGameInfo = simpl_GetGameInfo;

}

