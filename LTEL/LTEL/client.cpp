
#include "client.h"
#include "shared.h"

#include <Godot.hpp>
#include <Object.hpp>
#include <Node.hpp>
#include <Basis.hpp>
#include <ResourceLoader.hpp>
#include <Script.hpp>
#include <ClassDB.hpp>
#include <Reference.hpp>
#include <PackedScene.hpp>
#include <Texture.hpp>
#include <Control.hpp>

#include "common.h"
#include "game_object.h"

#include "common_physics.h"
#include "client_physics.h"

LTELClient* g_pLTELClient = nullptr;

extern std::vector<godot::StreamPeerBuffer*> g_pStreamInUse;

LTELClient::LTELClient(godot::Node* pGodotLink, HINSTANCE pCRes)
{
	g_pLTELClient = this;
	m_pLTELServer = nullptr;

	m_pClientInfo = nullptr;

	m_nGameMode = GAMEMODE_NONE;
	m_bIsConnected = false;
	m_bIsInWorld = false;
	m_bAllowInput = true;
	m_sGameDataDir = "";
	m_vFOV = godot::Vector2();
	m_vRelativeMouse = godot::Vector2();
	m_fFrametime = 0.1f;
	m_pCRes = pCRes;
	m_pGodotLink = pGodotLink;
	InitFunctionPointers();

	// Additional function implementations
	InitAudioImpl();
	InitInputImpl();
	InitRenderImpl();
	InitObjectImpl();
	InitStringImpl();

	m_pCommonLT = new LTELCommon();
	m_pPhysicsLT = new LTELClientPhysics(this);

}

LTELClient::~LTELClient()
{
	delete m_pCommonLT;
	delete m_pPhysicsLT;
}


//
// Called when the client creates a server (either by starting singleplayer or multiplayer)
//
bool LTELClient::StartServerDLL(StartGameRequest* pRequest)
{
	typedef int f_GetServerShellVersion();
	typedef void f_GetServerShellFunctions(CreateServerShellFn* pCreate, DeleteServerShellFn* pDelete);
	typedef ClassDef** f_ObjectDLLSetup(int* nDefs, ServerDE* pServer, int* version);


	// CWD is the project folder...
	HINSTANCE hObjectLTO = LoadLibraryA("./bin/Object.lto");

	if (!hObjectLTO)
	{
		godot::Godot::print("Could not locate Object.lto!");
		return false;
	}

	HINSTANCE hSRes = LoadLibraryA("./bin/SRes.dll");

	if (!hSRes)
	{
		godot::Godot::print("Could not locate SRes.dll!");
		return false;
	}

	// Initial test, print out the version!
	f_GetServerShellVersion* pGetVersion = (f_GetServerShellVersion*)GetProcAddress(hObjectLTO, "GetServerShellVersion");

	if (!pGetVersion)
	{
		godot::Godot::print("Could not obtain Proc GetClientShellVersion!");
		return false;
	}

	// Print the version!
	int nVersion = pGetVersion();
	godot::Godot::print("Object.lto version: {0}", nVersion);

	if (nVersion != 3)
	{
		godot::Godot::print("Object interface version is not 3!");
		return false;
	}

	// Okay now the real fun begins x2!
	// we'll need to kick off our server impl in object.lto

	f_GetServerShellFunctions* pServerShellInitFunc = (f_GetServerShellFunctions*)GetProcAddress(hObjectLTO, "GetServerShellFunctions");

	if (!pServerShellInitFunc)
	{
		godot::Godot::print("Could not obtain Proc GetServerShellFunctions!");
		return false;
	}

	//
	int pnCreate = 0;
	int pnDelete = 0;

	pServerShellInitFunc((CreateServerShellFn*)&pnCreate, (DeleteServerShellFn*)&pnDelete);

	if (!pnCreate)
	{
		godot::Godot::print("Could not obtain CreateServerShellFn!");

		return false;
	}

	m_pLTELServer = new LTELServer(m_pGodotLink, hSRes);

	// Add a client, because we need one!
	m_pClientInfo = new ClientInfo(true, "Shogoer", this);
	m_pLTELServer->m_pClientList.push_back(m_pClientInfo);

	// We'll want to run CreateServerShellFn, to get the game's ObjectLTO instance
	CreateServerShellFn pCreate = (CreateServerShellFn)pnCreate;
	auto pServerShell = (CServerShellDE*)pCreate(m_pLTELServer);

	m_pLTELServer->m_pServerShell = pServerShell;

	// Ship over some important settings...
	m_pLTELServer->SetGameInfo(pRequest->m_pGameInfo, pRequest->m_GameInfoLen);

	// Need to run ObjectDLLSetup to get Class definitions!!
	f_ObjectDLLSetup* pObjectDLLSetup = (f_ObjectDLLSetup*)GetProcAddress(hObjectLTO, "ObjectDLLSetup");

	int nClassDefCount = 0;
	int nObjectDefVersion = 0;

	auto pClassList = pObjectDLLSetup(&nClassDefCount, m_pLTELServer, &nObjectDefVersion);

	godot::Godot::print("ObjectDLLSetup version: {0}", nObjectDefVersion);
	godot::Godot::print("Found {0} class definitions!", nClassDefCount);

	if (nObjectDefVersion != 1)
	{
		godot::Godot::print("Object class definition version is not 1!");
		return false;
	}

	m_pLTELServer->m_nClassDefCount = nClassDefCount;
	m_pLTELServer->m_pClassDefList = pClassList;

	return true;
}

void LTELClient::SetCommandOn(int nCommandID)
{
	if (m_mCommands.find(nCommandID) == m_mCommands.end())
	{
		m_mCommands.insert({ nCommandID, true });
		return;
	}

	m_mCommands[nCommandID] = true;
}

void LTELClient::SetCommandOff(int nCommandID)
{
	if (m_mCommands.find(nCommandID) == m_mCommands.end())
	{
		return;
	}
	
	m_mCommands[nCommandID] = false;
}

godot::Ref<godot::ImageTexture> LTELClient::LoadPCX(std::string sPath)
{
	auto pNode = g_pLTELClient->m_pGodotLink->get_node("/root/Scene/Scripts/LoadPCX");
	godot::Ref<godot::ImageTexture> pTexture = pNode->call("load_image", sPath.c_str());
	return pTexture;
}

godot::Ref<godot::ImageTexture> LTELClient::LoadDTX(std::string sPath)
{
	auto pNode = g_pLTELClient->m_pGodotLink->get_node("/root/Scene/Scripts/LoadDTX");
	godot::Ref<godot::ImageTexture> pTexture = pNode->call("build", sPath.c_str(), godot::Array());
	return pTexture;
}

godot::Ref<godot::PackedScene> LTELClient::LoadABC(std::string sPath)
{
	auto pNode = g_pLTELClient->m_pGodotLink->get_node("/root/Scene/Scripts/LoadABC");
	godot::Ref<godot::PackedScene> pScene = pNode->call("build", sPath.c_str(), godot::Array());
	return pScene;
}

bool LTELClient::SetAlphaToTransparentColour(LTELSurface* pSurface, HDECOLOR hTransparentColor, bool bSkipIfOptimized)
{
	// If we can reliably skip optimization, then do so!
	if (bSkipIfOptimized && pSurface->bOptimized)
	{
		return false;
	}

	// We don't optimize the screen or text
	if (pSurface->bIsScreen || pSurface->bIsText)
	{
		return false;
	}

	// Null check
	if (!pSurface->pTextureRect || pSurface->pTextureRect->get_texture().is_null() || pSurface->pTextureRect->get_texture()->get_data().is_null())
	{
		// We don't know if this is a jake error, or a game error yet!
		return false;
	}

	godot::Color oColor = godot::Color();

	if (hTransparentColor)
	{
		oColor = LT2GodotColor(hTransparentColor);
	}

	oColor.a = 1.0f;

	godot::Ref<godot::ImageTexture> pTexture = pSurface->pTextureRect->get_texture();

	auto pImage = pTexture->get_data();

	pImage->lock();

	// Loop through the image, check the pixel and set it to transparent!
	for (int x = 0; x < pImage->get_width(); x++)
	{
		for (int y = 0; y < pImage->get_height(); y++)
		{
			auto pPixel = pImage->get_pixel(x, y);
			pPixel.a = 1.0f;

			if (pPixel == oColor)
			{
				// Just swap the alpha, and put it back!
				pPixel.a = 0.0f;

				// Debug
				//pPixel = godot::Color();

				pImage->set_pixel(x, y, pPixel);
			}

		}
	}

	pImage->unlock();

	// Now re-set the image!
	pTexture->set_data(pImage);

	// Cool, let's not do this again
	pSurface->bOptimized = true;

	return true;
}

bool LTELClient::BlitSurfaceToSurface(LTELSurface* pDest, LTELSurface* pSrc, DRect* pDestRect, DRect* pSrcRect, bool bScale)
{
	bool bCanBlit = false;

	if (!pDest || !pSrc)
	{
		return DE_ERROR;
	}

	godot::Control* pControl = GDCAST(godot::Control, g_pLTELClient->m_pGodotLink->get_node(CANVAS_NODE));

	if (!pControl)
	{
		return DE_ERROR;
	}

	auto vPos = godot::Vector2(pDestRect->left, pDestRect->top);
	auto vSize = godot::Vector2(pDestRect->right - pDestRect->left, pDestRect->bottom - pDestRect->top);

	godot::Node* pNode = GDCAST(godot::Node, pControl);

	// Screen!
	if (!pDest->bIsScreen)
	{
		//return DE_OK;
		if (pDest->bIsText)
		{
			pNode = GDCAST(godot::Node, pDest->pLabel);
		}
		else
		{
			pNode = GDCAST(godot::Node, pDest->pTextureRect);
			bCanBlit = true;
		}
	}

	if (pSrc->bIsText && pSrc->pLabel)
	{
		pSrc->pLabel->set_position(vPos);
		if (bScale)
		{
			pSrc->pLabel->set_size(vSize);
		}
		if (!pSrc->pLabel->get_parent())
		{
			pNode->add_child(pSrc->pLabel);
		}
	}
	else if (pDest->pTextureRect && pSrc->pTextureRect)
	{
		if (bCanBlit)
		{
			godot::Ref<godot::ImageTexture> pDestTexture = pDest->pTextureRect->get_texture();
			auto pSrcTexture = pSrc->pTextureRect->get_texture();

			if (!pDestTexture.is_null() && !pSrcTexture.is_null())
			{
				//x,y,w,h
				godot::Rect2 rSrcRect = godot::Rect2();

				if (pSrcRect)
				{
					float nWidth = pSrcRect->right - pSrcRect->left;
					float nHeight = pSrcRect->bottom - pSrcRect->top;
					float x = pSrcRect->left;
					float y = pSrcRect->top;

					rSrcRect.set_size(godot::Vector2(nWidth, nHeight));
					rSrcRect.set_position(godot::Vector2(x, y));
				}
				else
				{
					rSrcRect.set_size(godot::Vector2(pSrcTexture->get_data()->get_width(), pSrcTexture->get_data()->get_height()));
					rSrcRect.set_position(godot::Vector2(0, 0));
				}



				godot::Vector2 vDestRect = godot::Vector2(pDestRect->left, pDestRect->top);
				auto pImage = pDestTexture->get_data();

				if (pSrc->bOptimized)
				{
					pImage->blend_rect(pSrcTexture->get_data(), rSrcRect, vDestRect);
				}
				else
				{
					pImage->blit_rect(pSrcTexture->get_data(), rSrcRect, vDestRect);
				}

				pDestTexture->set_data(pImage);
			}
			else
			{
				bCanBlit = false;
			}
		}

		// Not else, because above path can fallback here!
		if (!bCanBlit)
		{
			pSrc->pTextureRect->set_position(vPos);
			pSrc->pTextureRect->set_size(godot::Vector2(pSrc->pTextureRect->get_texture()->get_data()->get_width(), pSrc->pTextureRect->get_texture()->get_height()));

			if (pSrc->pTextureRect->get_parent())
			{
				pSrc->pTextureRect->get_parent()->remove_child(pSrc->pTextureRect);
			}

			//if (!pSrc->pTextureRect->get_parent())
			{
				pNode->add_child(pSrc->pTextureRect);
			}
		}
	}
	else
	{
		godot::Godot::print("Possible memory leak!");
		return DE_ERROR;
	}

	// Don't need this yet
	return DE_OK;
}

//
// STUB FUNCTIONS
// If you wish to implement functions please place it in their own sectioned off .cpp file!
//

DRESULT LTELClient::GetPointStatus(DVector* pPoint)
{
	return DE_ERROR;
}

DRESULT LTELClient::GetPointShade(DVector* pPoint, DVector* pColor)
{
	return DE_ERROR;
}

DRESULT LTELClient::OpenFile(char* pFilename, DStream** pStream)
{
	return DE_ERROR;
}

DRESULT LTELClient::GetSConValueFloat(char* pName, float& val)
{
	return DE_ERROR;
}

DRESULT LTELClient::GetSConValueString(char* pName, char* valBuf, DDWORD bufLen)
{
	return DE_ERROR;
}

float LTELClient::GetServerConVarValueFloat(char* pName)
{
	return 0.0f;
}

char* LTELClient::GetServerConVarValueString(char* pName)
{
	return nullptr;
}

DRESULT LTELClient::SetupEuler(DRotation* pRotation, float pitch, float yaw, float roll)
{
	return m_pCommonLT->SetupEuler(*pRotation, pitch, yaw, roll);
}

DRESULT LTELClient::GetRotationVectors(DRotation* pRotation, DVector* pUp, DVector* pRight, DVector* pForward)
{
	return m_pCommonLT->GetRotationVectors(*pRotation, *pUp, *pRight, *pForward);
}

HMESSAGEWRITE LTELClient::StartMessage(DBYTE messageID)
{
	auto pStream = (godot::StreamPeerBuffer*)StartHMessageWrite();
	pStream->put_8(messageID);

	return (HMESSAGEWRITE)pStream;
}

DRESULT LTELClient::EndMessage(HMESSAGEWRITE hMessage)
{
	return EndMessage2(hMessage, MESSAGE_GUARANTEED);
}

DRESULT LTELClient::EndMessage2(HMESSAGEWRITE hMessage, DDWORD flags)
{
	if (!m_pLTELServer)
	{
		// Clean up...
		auto pStream = (godot::StreamPeerBuffer*)hMessage;
		pStream->free();

		return DE_SERVERERROR;
	}


	m_pLTELServer->ReceiveMessageFromClient(m_pClientInfo, (godot::StreamPeerBuffer*)hMessage, flags);

	shared_CleanupStream((godot::StreamPeerBuffer*)hMessage);

	// Clean up...
	auto pStream = (godot::StreamPeerBuffer*)hMessage;
	pStream->free();

	return DE_OK;
}

DRESULT LTELClient::SendToServer(LMessage& msg, DBYTE msgID, DDWORD flags)
{
	return DE_ERROR;
}

DRESULT LTELClient::ProcessAttachments(HOBJECT hObj)
{
	return DE_ERROR;
}

DEParticle* LTELClient::AddParticle(HLOCALOBJ hObj, DVector* pPos, DVector* pVelocity, DVector* pColor, float lifeTime)
{
	return nullptr;
}

DRESULT LTELClient::GetSpriteControl(HLOCALOBJ hObj, SpriteControl*& pControl)
{
	return DE_ERROR;
}

DRESULT LTELClient::StartQuery(char* pInfo)
{
	return DE_ERROR;
}

DRESULT LTELClient::UpdateQuery()
{
	return DE_ERROR;
}

DRESULT LTELClient::GetQueryResults(NetSession*& pListHead)
{
	return DE_ERROR;
}

DRESULT LTELClient::EndQuery()
{
	return DE_ERROR;
}

HMESSAGEWRITE LTELClient::StartHMessageWrite()
{
	return shared_StartHMessageWrite();
}

DRESULT LTELClient::WriteToMessageFloat(HMESSAGEWRITE hMessage, float val)
{
	return shared_WriteToMessageFloat(hMessage, val);
}

DRESULT LTELClient::WriteToMessageByte(HMESSAGEWRITE hMessage, DBYTE val)
{
	return shared_WriteToMessageByte(hMessage, val);
}

DRESULT LTELClient::WriteToMessageWord(HMESSAGEWRITE hMessage, D_WORD val)
{
	return shared_WriteToMessageWord(hMessage, val);
}

DRESULT LTELClient::WriteToMessageDWord(HMESSAGEWRITE hMessage, DDWORD val)
{
	return shared_WriteToMessageDWord(hMessage, val);
}

DRESULT LTELClient::WriteToMessageString(HMESSAGEWRITE hMessage, char* pStr)
{
	return shared_WriteToMessageString(hMessage, pStr);
}

DRESULT LTELClient::WriteToMessageVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageVector(hMessage, pVal);
}

DRESULT LTELClient::WriteToMessageCompVector(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageCompVector(hMessage, pVal);
}

DRESULT LTELClient::WriteToMessageCompPosition(HMESSAGEWRITE hMessage, DVector* pVal)
{
	return shared_WriteToMessageCompPosition(hMessage, pVal);
}

DRESULT LTELClient::WriteToMessageRotation(HMESSAGEWRITE hMessage, DRotation* pVal)
{
	return shared_WriteToMessageRotation(hMessage, pVal);
}

DRESULT LTELClient::WriteToMessageHString(HMESSAGEWRITE hMessage, HSTRING hString)
{
	return shared_WriteToMessageHString(hMessage, hString);
}

DRESULT LTELClient::WriteToMessageHMessageWrite(HMESSAGEWRITE hMessage, HMESSAGEWRITE hDataMessage)
{
	return shared_WriteToMessageHMessageWrite(hMessage, hDataMessage);
}

DRESULT LTELClient::WriteToMessageHMessageRead(HMESSAGEWRITE hMessage, HMESSAGEREAD hDataMessage)
{
	return shared_WriteToMessageHMessageRead(hMessage, hDataMessage);
}

// Not used in Shogo!
DRESULT LTELClient::WriteToMessageFormattedHString(HMESSAGEWRITE hMessage, int messageCode, ...)
{
	return DE_ERROR;
}

DRESULT LTELClient::WriteToMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObj)
{
	return shared_WriteToMessageObject(hMessage, hObj);
}

DRESULT LTELClient::WriteToLoadSaveMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObject)
{
	return shared_WriteToLoadSaveMessageObject(hMessage, hObject);
}

float LTELClient::ReadFromMessageFloat(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageFloat(hMessage);
}

DBYTE LTELClient::ReadFromMessageByte(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageByte(hMessage);
}

D_WORD LTELClient::ReadFromMessageWord(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageWord(hMessage);
}

DDWORD LTELClient::ReadFromMessageDWord(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageDWord(hMessage);
}

char* LTELClient::ReadFromMessageString(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageString(hMessage);
}

void LTELClient::ReadFromMessageVector(HMESSAGEREAD hMessage, DVector* pVal)
{
	return shared_ReadFromMessageVector(hMessage, pVal);
}

void LTELClient::ReadFromMessageCompVector(HMESSAGEREAD hMessage, DVector* pVal)
{
	return shared_ReadFromMessageCompVector(hMessage, pVal);
}

void LTELClient::ReadFromMessageCompPosition(HMESSAGEREAD hMessage, DVector* pVal)
{
	return shared_ReadFromMessageCompPosition(hMessage, pVal);
}

void LTELClient::ReadFromMessageRotation(HMESSAGEREAD hMessage, DRotation* pVal)
{
	return shared_ReadFromMessageRotation(hMessage, pVal);
}

HOBJECT LTELClient::ReadFromMessageObject(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageObject(hMessage);
}

HSTRING LTELClient::ReadFromMessageHString(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageHString(hMessage);
}

DRESULT LTELClient::ReadFromLoadSaveMessageObject(HMESSAGEREAD hMessage, HOBJECT* hObject)
{
	return shared_ReadFromLoadSaveMessageObject(hMessage, hObject);
}

HMESSAGEREAD LTELClient::ReadFromMessageHMessageRead(HMESSAGEREAD hMessage)
{
	return shared_ReadFromMessageHMessageRead(hMessage);
}

void LTELClient::EndHMessageRead(HMESSAGEREAD hMessage)
{
	shared_EndHMessageRead(hMessage);
}

void LTELClient::EndHMessageWrite(HMESSAGEWRITE hMessage)
{
	shared_EndHMessageWrite(hMessage);
}

void LTELClient::ResetRead(HMESSAGEREAD hRead)
{
	shared_ResetRead(hRead);
}
