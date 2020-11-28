#pragma once

#include "LT1/AppHeaders/cpp_client_de.h"
#include "LT1/AppHeaders/cpp_clientshell_de.h"
#include <Node.hpp>
#include <Windows.h>
#include <string>

#include <Ref.hpp>
#include <ImageTexture.hpp>
#include <StreamPeer.hpp>
#include <StreamPeerBuffer.hpp>
#include <PackedScene.hpp>

#include "helpers.h"
#include "server.h"

// Some windows function names clash with ClientDE
#undef CreateFont
#undef PlaySound

class LTELClient :
    public ClientDE
{

public:
	LTELClient(godot::Node* pGodotLink, HINSTANCE pCRes);
    virtual ~LTELClient();

	bool StartServerDLL(StartGameRequest* pRequest);

    // Required Impl
	void InitFunctionPointers();
	void InitRenderImpl();
	void InitObjectImpl();
	void InitStringImpl();

	
    void SetCommandOn(int nCommandID);

	godot::Ref<godot::ImageTexture> LoadPCX(std::string sPath);
    godot::Ref<godot::ImageTexture> LoadDTX(std::string sPath);
    godot::Ref<godot::PackedScene> LoadABC(std::string sPath);

    bool BlitSurfaceToSurface(LTELSurface* pDest, LTELSurface* pSrc, DRect* pDestRect, DRect* pSrcRect, bool bScale);

    // Commands which are on, this is cleared every frame
    std::unordered_map<int, bool> m_mCommands;

	std::string m_sGameDataDir;
	godot::Vector2 m_vFOV;
	HINSTANCE m_pCRes;
	godot::Node* m_pGodotLink;
	float m_fFrametime;

	LTELServer* m_pLTELServer;

	int m_nGameMode;
	bool m_bIsConnected;
    bool m_bIsInWorld;

    ClientInfo* m_pClientInfo;

    CClientShellDE* m_pClientShell;

    godot::Vector2 m_vRelativeMouse;

    

	//
	// CSBASE
	//

	// Start a data message for writing.  A data message is used to write an HMESSAGEWRITE into another
	// already open HMESSAGEWRITE.  When the data message is complete, first call
	// WriteToMessageHMessageWrite to write it into the other HMESSAGE, then call EndMessageHMessageWrite to free it.
	virtual HMESSAGEWRITE	StartHMessageWrite();


    virtual DRESULT			WriteToMessageFloat(HMESSAGEWRITE hMessage, float val);
    virtual DRESULT			WriteToMessageByte(HMESSAGEWRITE hMessage, DBYTE val);
    virtual DRESULT			WriteToMessageWord(HMESSAGEWRITE hMessage, D_WORD val);
    virtual DRESULT			WriteToMessageDWord(HMESSAGEWRITE hMessage, DDWORD val);
    virtual DRESULT			WriteToMessageString(HMESSAGEWRITE hMessage, char* pStr);
    virtual DRESULT			WriteToMessageVector(HMESSAGEWRITE hMessage, DVector* pVal); // 12 bytes
    virtual DRESULT			WriteToMessageCompVector(HMESSAGEWRITE hMessage, DVector* pVal); // 9 bytes
    virtual DRESULT			WriteToMessageCompPosition(HMESSAGEWRITE hMessage, DVector* pVal); // 7 bytes
    virtual DRESULT			WriteToMessageRotation(HMESSAGEWRITE hMessage, DRotation* pVal);
    virtual DRESULT			WriteToMessageHString(HMESSAGEWRITE hMessage, HSTRING hString);

    // Writes a HMESSAGEWRITE into an already opened HMESSAGEWRITE.
    // Inputs:
    //		hMessage -		HMESSAGEWRITE written to.
    //		hDataMessage -	HMESSAGEWRITE written from.
    virtual DRESULT			WriteToMessageHMessageWrite(HMESSAGEWRITE hMessage, HMESSAGEWRITE hDataMessage);

    // Writes a HMESSAGEREAD into an already opened HMESSAGEWRITE.
    // Inputs:
    //		hMessage -		HMESSAGEWRITE written to.
    //		hDataMessage -	HMESSAGEREAD written from.
    virtual DRESULT			WriteToMessageHMessageRead(HMESSAGEWRITE hMessage, HMESSAGEREAD hDataMessage);

    // Helper so you don't have to FormatString and FreeString..
    virtual DRESULT			WriteToMessageFormattedHString(HMESSAGEWRITE hMessage, int messageCode, ...);

    // Note: you can't send object references to the client yet, so the client can't
    // even read object references.
    // You can't write object references in a HMESSAGEWRITE passed in MID_SAVEOBJECT.
    virtual DRESULT			WriteToMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObj);

    // Use this only while saving objects (inside MID_SAVEOBJECT).
    virtual DRESULT			WriteToLoadSaveMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObject);


    // When your OnMessage function is called, use the handle you're given
    // to read the message data with these functions.
    virtual float			ReadFromMessageFloat(HMESSAGEREAD hMessage);
    virtual DBYTE			ReadFromMessageByte(HMESSAGEREAD hMessage);
    virtual D_WORD			ReadFromMessageWord(HMESSAGEREAD hMessage);
    virtual DDWORD			ReadFromMessageDWord(HMESSAGEREAD hMessage);
    virtual char* ReadFromMessageString(HMESSAGEREAD hMessage);
    virtual void			ReadFromMessageVector(HMESSAGEREAD hMessage, DVector* pVal); // 12 bytes
    virtual void			ReadFromMessageCompVector(HMESSAGEREAD hMessage, DVector* pVal); // 9 bytes
    virtual void			ReadFromMessageCompPosition(HMESSAGEREAD hMessage, DVector* pVal); // 7 bytes
    virtual void			ReadFromMessageRotation(HMESSAGEREAD hMessage, DRotation* pVal);
    virtual HOBJECT			ReadFromMessageObject(HMESSAGEREAD hMessage);
    virtual HSTRING			ReadFromMessageHString(HMESSAGEREAD hMessage);

    // Use this only while loading objects (inside MID_LOADOBJECT).
    virtual DRESULT			ReadFromLoadSaveMessageObject(HMESSAGEREAD hMessage, HOBJECT* hObject);

    // Reads a data message from an HMESSAGEREAD.  The returned HMESSAGEREAD can then be used in the
    // ReadFromMessageX functions.  This will create a new HMESSAGEREAD which must be
    // freed with a call to EndHMessageRead().
    // Inputs:
    //		hMessage -		HMESSAGEREAD read from.
    virtual HMESSAGEREAD	ReadFromMessageHMessageRead(HMESSAGEREAD hMessage);

    // Frees a HMESSAGEREAD created from a call of ReadFromMessageHMessageRead.
    virtual void			EndHMessageRead(HMESSAGEREAD hMessage);

    // Frees a HMESSAGEWRITE created from a call of StartHMessageWrite.
    virtual void			EndHMessageWrite(HMESSAGEWRITE hMessage);

    // Reset reading (so you can read the message again).
    // This is useful if you read out of a message and subclasses
    // will be reading out of it.  Note: the message will AUTOMATICALLY
    // reset when you hit the end, so you won't need this in most cases.
    virtual void			ResetRead(HMESSAGEREAD hRead);

	//
	// CLIENTDE
	//

    // OBSOLETE: use the CommonDE one.
    virtual DRESULT GetPointStatus(DVector* pPoint);

    // Get the shade (RGB, 0-255) at the point you specify.
    // Returns DE_NOTINWORLD if the point is outside the world.
    virtual DRESULT	GetPointShade(DVector* pPoint, DVector* pColor);

    // Open a file up.  Pass in the relative filename.
    // Free the file by calling DStream::Release().
    virtual DRESULT	OpenFile(char* pFilename, DStream** pStream);

    // The 'new' accessors for server console variables.  Returns DE_NOTFOUND
// if the variable isn't found.
    virtual DRESULT	GetSConValueFloat(char* pName, float& val);
    virtual DRESULT	GetSConValueString(char* pName, char* valBuf, DDWORD bufLen);

    // Gets the value of a parameter .. returns 0/NULL if you pass in NULL.
    // OBSOLETE (will be removed soon).  Use the GetSCon functions.
    virtual float	GetServerConVarValueFloat(char* pName);
    virtual char* GetServerConVarValueString(char* pName);

	// OBSOLETE: use CommonLT::SetupEuler.
	virtual DRESULT	SetupEuler(DRotation* pRotation, float pitch, float yaw, float roll);

	// OBSOLETE.  Use CommonLT::GetRotationVectors.
	virtual DRESULT GetRotationVectors(DRotation* pRotation, DVector* pUp, DVector* pRight, DVector* pForward);

	// Messaging.

	virtual HMESSAGEWRITE	StartMessage(DBYTE messageID);
	virtual DRESULT			EndMessage(HMESSAGEWRITE hMessage); // Just calls EndMessage2 with MESSAGE_GUARANTEED.
	virtual DRESULT			EndMessage2(HMESSAGEWRITE hMessage, DDWORD flags);


	// NEW message functions.  These functions don't free the message so you need to
	// call LMessage::Release after sending.

	virtual DRESULT	SendToServer(LMessage& msg, DBYTE msgID, DDWORD flags);

	// Updates the position/rotation of the attachments on the object.  Attachments are
	// always automatically updated when the object is rendered.
	virtual DRESULT	ProcessAttachments(HOBJECT hObj);

    virtual DEParticle* AddParticle(HLOCALOBJ hObj, DVector* pPos, DVector* pVelocity, DVector* pColor, float lifeTime);

    // Get the sprite control interface for a sprite.  Returns DE_INVALIDPARAMS
    // if the object is not a sprite.
    virtual DRESULT GetSpriteControl(HLOCALOBJ hObj, SpriteControl*& pControl);

	// Alternate mode of getting session lists.  These only work for services with
	// the NETSERVICE_TCPIP flag.  These functions return immediately so you can update
	// a UI in the background without having to 'freeze' the UI while it queries hosts.

	// Start querying.  pInfo contains the address list formatted just like GetSessionLists.
	virtual DRESULT	StartQuery(char* pInfo);

	// Update the query.  Call this as often as possible.
	virtual DRESULT UpdateQuery();

	// Get the current list of results from the query.  Each time you call this,
	// a new session list is allocated and you need to free it with FreeSessionList.
	virtual DRESULT GetQueryResults(NetSession*& pListHead);

	// End the current query.
	virtual DRESULT EndQuery();




};

