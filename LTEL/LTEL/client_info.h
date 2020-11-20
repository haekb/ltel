#pragma once
#include "LT1/AppHeaders/client_de.h"

#include "game_object.h"
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
    void SetObj(GameObject* pObject) { m_pObject = pObject; }
    void SetUserData(void* pData) { m_pUserData = pData; }
    void SetFlags(uint32_t nFlags) { m_nFlags = nFlags; }
    
    // Getters
    bool IsHost() { return m_bIsHost; }
    std::string GetName() { return m_sName; }
    uint32_t GetFlags() { return m_nFlags; }
    ClientDE* GetClient() { return m_pClient; }
    ClientShellDE* GetClientShell() { return m_pClient->GetClientShell(); }
    GameObject* GetObj() { return m_pObject; }
    void* GetUserData() { return m_pUserData; }

protected:
    bool m_bIsHost;
    std::string m_sName;
    ClientDE* m_pClient;
    uint32_t m_nFlags;
    //LPBASECLASS m_pObject;
    void* m_pUserData;
    GameObject* m_pObject;
};