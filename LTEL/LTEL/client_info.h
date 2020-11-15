#pragma once
#include "LT1/AppHeaders/client_de.h"

#include <string>

class ClientInfo
{
public:
    ClientInfo(bool bIsHost, std::string sName, ClientDE* pClient);
    ~ClientInfo();

    //void SetIsHost(bool bVal);
    //void SetName(char* szName);
    //void SetClient(ClientDE* pClient);

    // Setters
    void SetObject(LPBASECLASS pObject) { m_pObject = pObject; }
    void SetUserData(void* pData) { m_pUserData = pData; }
    
    // Getters
    bool IsHost() { return m_bIsHost; }
    std::string GetName() { return m_sName; }
    uint32_t GetFlags() { return m_nFlags; }
    ClientDE* GetClient() { return m_pClient; }
    ClientShellDE* GetClientShell() { return m_pClient->GetClientShell(); }
    LPBASECLASS GetObject() { return m_pObject; }
    void* GetUserData() { return m_pUserData; }

protected:
    bool m_bIsHost;
    std::string m_sName;
    ClientDE* m_pClient;
    uint32_t m_nFlags;
    LPBASECLASS m_pObject;
    void* m_pUserData;
};