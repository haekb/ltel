#pragma once
#include "LT1/AppHeaders/cpp_servershell_de.h"
#include <Node.hpp>
#include <Windows.h>
#include <string>


class LTELServer :
    public ServerDE
{
public:
    LTELServer(godot::Node* pGodotLink, HINSTANCE pSRes);
    virtual ~LTELServer();

    HINSTANCE m_pSRes;
    godot::Node* m_pGodotLink;
    float m_fFrametime;

    void* m_pGameInfo;

    CServerShellDE* m_pServerShell;


    // Required Impl
    void InitFunctionPointers();

    void SetGameInfo(void* pData, int pLen);

    void StartWorld(std::string sWorldName);

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
};

