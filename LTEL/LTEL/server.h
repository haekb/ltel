#pragma once
#include "LT1/AppHeaders/cpp_servershell_de.h"
#include "LT1/AppHeaders/client_de.h"
#include "client_info.h"
#include "game_object.h"

// Godot Stuff
#include <Node.hpp>
#include <StreamPeer.hpp>
#include <StreamPeerBuffer.hpp>
// End

#include <Windows.h>
#include <string>
#include <vector>

class LTELServer :
    public ServerDE
{
public:
    LTELServer(godot::Node* pGodotLink, HINSTANCE pSRes);
    virtual ~LTELServer();

    HINSTANCE m_pSRes;
    godot::Node* m_pGodotLink;
    float m_fFrametime;
    float m_fTime;

    std::string m_sGameDataDir;
    void* m_pGameInfo;

    CServerShellDE* m_pServerShell;

    uint32_t m_nFlags;

    bool m_bInWorld;

    
    int m_nClassDefCount;
    ClassDef** m_pClassDefList;

    std::vector<ClientInfo*> m_pClientList;
    std::vector<GameObject*> m_pObjectList;

    // Stupid stateful
    GameObject* m_pCurrentObject;

    // Required Impl
    void InitFunctionPointers();

    void SetGameInfo(void* pData, int pLen);

    void StartWorld(std::string sWorldName);

    bool ReceiveMessageFromClient(ClientInfo* pClientInfo, godot::StreamPeerBuffer* pStream, DDWORD flags);

    void Update(DFLOAT timeElapsed);

    

    //
    // ServerDE
    //
    
    // OBSOLETE.  Use PhysicsLT functions.
    virtual DRESULT GetGlobalForce(DVector* pVec);
    virtual DRESULT SetGlobalForce(DVector* pVec);


    // OBSOLETE messaging functions.  Use CommonLT::CreateMessage, LMessage::Release, 
    // and ServerDE::SendToX.

    virtual HMESSAGEWRITE	StartSpecialEffectMessage(LPBASECLASS pObject) ;
    virtual HMESSAGEWRITE	StartInstantSpecialEffectMessage(DVector* pPos) ;
    virtual HMESSAGEWRITE	StartMessageToObject(LPBASECLASS pSender, HOBJECT hSendTo, DDWORD messageID) ;
    virtual DRESULT			StartMessageToServer(LPBASECLASS pSender, DDWORD messageID, HMESSAGEWRITE* hWrite) ;
    virtual HMESSAGEWRITE	StartMessage(HCLIENT hSendTo, DBYTE messageID) ;
    virtual DRESULT			EndMessage2(HMESSAGEWRITE hMessage, DDWORD flags) ;
    virtual DRESULT			EndMessage(HMESSAGEWRITE hMessage) ;	// Just calls EndMessage2 with MESSAGE_GUARANTEED.

    // NEW message functions.  The main difference between these and the functions above is
    // that these don't free the message for you so you can send it multiple times.  

    // Use this to setup a special effect message.  If your object has
    // a special effect message, the client shell's SpecialEffectNotify() 
    // will be called.  An object can only have ONE special effect message.
    // If the object already has a special effect message, then it
    // clears out the current one.
    virtual DRESULT	SetObjectSFXMessage(HOBJECT hObject, LMessage& msg) ;

    // Send a message to an object.  hSender can be NULL.
    virtual DRESULT SendToObject(LMessage& msg, DDWORD msgID, HOBJECT hSender, HOBJECT hSendTo, DDWORD flags) ;

    // Send the message to the server shell.  hSender can be NULL.
    virtual DRESULT SendToServer(LMessage& msg, DDWORD msgID, HOBJECT hSender, DDWORD flags) ;

    // Send the message to the client.  If hSendTo is NULL, it sends to them all.
    virtual DRESULT SendToClient(LMessage& msg, DBYTE msgID, HCLIENT hSendTo, DDWORD flags) ;

    // Sends the sfx message to all the clients who can see pos.
    virtual DRESULT	SendSFXMessage(LMessage& msg, DVector& pos, DDWORD flags) ;

    // Get a client's ping time.
    virtual DRESULT GetClientPing(HCLIENT hClient, float& ping) ;

    // OBSOLETE: use CommonLT::SetupEuler.
    virtual DRESULT	SetupEuler(DRotation* pRotation, float pitch, float yaw, float roll) ;

    // OBSOLETE: use the CommonLT ones.
    virtual float	GetObjectMass(HOBJECT hObj) ;
    virtual void	SetObjectMass(HOBJECT hObj, float mass) ;

    // OBSOLETE: Use PhysicsLT.
    virtual float	GetForceIgnoreLimit(HOBJECT hObj, float& limit) ;
    virtual void	SetForceIgnoreLimit(HOBJECT hObj, float limit) ;

    // OBSOLETE.  Use PhysicsLT::GetStandingOn.
    virtual DRESULT	GetStandingOn(HOBJECT hObj, CollisionInfo* pInfo) ;

    // OBSOLETE.  Use PhysicsLT.
    virtual void GetObjectDims(HOBJECT hObj, DVector* pNewDims) ;
    virtual DRESULT	SetObjectDims(HOBJECT hObj, DVector* pNewDims) ;
    virtual DRESULT	SetObjectDims2(HOBJECT hObj, DVector* pNewDims) ;

    // OBSOLETE: use CommonLT functions.
    virtual DRESULT	GetVelocity(HOBJECT hObj, DVector* pVel) ;
    virtual DRESULT	SetVelocity(HOBJECT hObj, DVector* pVel) ;
    virtual DRESULT	GetAcceleration(HOBJECT hObj, DVector* pAccel) ;
    virtual DRESULT	SetAcceleration(HOBJECT hObj, DVector* pAccel) ;

    // OBSOLETE: use CommonLT.
    virtual DRESULT	GetModelAnimUserDims(HOBJECT hObj, DVector* pDims, HMODELANIM hAnim) ;

    // OBSOLETE.  Use PhysicsLT.
    virtual HOBJECT GetWorldObject();

    // Get the bounding box for the current world.
    virtual DRESULT	GetWorldBox(DVector& min, DVector& max);

    // OBSOLETE.  Use CommonLT::GetRotationVectors.
    virtual DRESULT GetRotationVectors(DRotation* pRotation,
        DVector* pUp, DVector* pRight, DVector* pForward);

    // Set an object's friction coefficient.
    virtual DRESULT	SetFrictionCoefficient(HOBJECT hObj, float coeff);

    // OBSOLETE.  Use PhysicsLT::MoveObject.
    virtual DRESULT MoveObject(HOBJECT hObj, DVector* pNewPos);

    // Get/set an object's net flags.  Net flags are a combination of NETFLAG_ defines.
    virtual DRESULT GetNetFlags(HOBJECT hObj, DDWORD& flags);
    virtual DRESULT SetNetFlags(HOBJECT hObj, DDWORD flags);

    // OBSOLETE: use CommonLT.
    virtual DRESULT GetPolyTextureFlags(HPOLY hPoly, DDWORD* pFlags);

    //
    // CSBase
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
};

