#include "client_info.h"
#include "LT1/AppHeaders/server_de.h"

ClientInfo::ClientInfo(bool bIsHost, std::string sName, ClientDE* pClient)
{
	m_bIsHost = bIsHost;
	m_sName = sName;
	m_pClient = pClient;
	m_nFlags = CIF_LOCAL;
	m_pObject = nullptr;
	m_pUserData = nullptr;
}

ClientInfo::~ClientInfo()
{
}
