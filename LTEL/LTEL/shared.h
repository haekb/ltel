#pragma once
#include "client.h"
#include "server.h"
#include "helpers.h"

#include <StreamPeer.hpp>
#include <StreamPeerBuffer.hpp>

// 
// Shared functions
// 
// This may need to be split into a class..
//

//
// Animation
//

DDWORD shared_GetModelAnimation(HLOCALOBJ hObj);
DDWORD shared_GetModelPlaybackState(HLOCALOBJ hObj);
HMODELANIM shared_GetAnimIndex(HOBJECT hObj, char* pAnimName);

//
// Objects
// 
DRESULT shared_SetObjectScale(HLOCALOBJ hObj, DVector* pScale);

//
// Strings
//
LTELString* shared_CreateString(char* pString);

char* shared_GetStringData(HSTRING hString);

void shared_FreeString(HSTRING hString);

int	shared_Parse(char* pCommand, char** pNewCommandPos, char* argBuffer, char** argPointers, int* nArgs);

//
// Messaging
// This is actually from CSBase, but both sides need to implement it.
//

void shared_CleanupStream(godot::StreamPeerBuffer* pStream);

bool shared_SetStreamValue(godot::StreamPeerBuffer* pStream, godot::Variant pValue);

// Start a data message for writing.  A data message is used to write an HMESSAGEWRITE into another
		// already open HMESSAGEWRITE.  When the data message is complete, first call
		// WriteToMessageHMessageWrite to write it into the other HMESSAGE, then call EndMessageHMessageWrite to free it.
HMESSAGEWRITE	shared_StartHMessageWrite();

DRESULT			shared_WriteToMessageFloat(HMESSAGEWRITE hMessage, float val);
DRESULT			shared_WriteToMessageByte(HMESSAGEWRITE hMessage, DBYTE val);
DRESULT			shared_WriteToMessageWord(HMESSAGEWRITE hMessage, D_WORD val);
DRESULT			shared_WriteToMessageDWord(HMESSAGEWRITE hMessage, DDWORD val);
DRESULT			shared_WriteToMessageString(HMESSAGEWRITE hMessage, char* pStr);
DRESULT			shared_WriteToMessageVector(HMESSAGEWRITE hMessage, DVector* pVal); // 12 bytes
DRESULT			shared_WriteToMessageCompVector(HMESSAGEWRITE hMessage, DVector* pVal); // 9 bytes
DRESULT			shared_WriteToMessageCompPosition(HMESSAGEWRITE hMessage, DVector* pVal); // 7 bytes
DRESULT			shared_WriteToMessageRotation(HMESSAGEWRITE hMessage, DRotation* pVal);
DRESULT			shared_WriteToMessageHString(HMESSAGEWRITE hMessage, HSTRING hString);

// Writes a HMESSAGEWRITE into an already opened HMESSAGEWRITE.
// Inputs:
//		hMessage -		HMESSAGEWRITE written to.
//		hDataMessage -	HMESSAGEWRITE written from.
DRESULT			shared_WriteToMessageHMessageWrite(HMESSAGEWRITE hMessage, HMESSAGEWRITE hDataMessage);

// Writes a HMESSAGEREAD into an already opened HMESSAGEWRITE.
// Inputs:
//		hMessage -		HMESSAGEWRITE written to.
//		hDataMessage -	HMESSAGEREAD written from.
DRESULT			shared_WriteToMessageHMessageRead(HMESSAGEWRITE hMessage, HMESSAGEREAD hDataMessage);

// Helper so you don't have to FormatString and FreeString..
DRESULT			shared_WriteToMessageFormattedHString(HMESSAGEWRITE hMessage, int messageCode, ...);

// Note: you can't send object references to the client yet, so the client can't
// even read object references.
// You can't write object references in a HMESSAGEWRITE passed in MID_SAVEOBJECT.
DRESULT			shared_WriteToMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObj);

// Use this only while saving objects (inside MID_SAVEOBJECT).
DRESULT			shared_WriteToLoadSaveMessageObject(HMESSAGEWRITE hMessage, HOBJECT hObject);


// When your OnMessage function is called, use the handle you're given
// to read the message data with these functions.
float			shared_ReadFromMessageFloat(HMESSAGEREAD hMessage);
DBYTE			shared_ReadFromMessageByte(HMESSAGEREAD hMessage);
D_WORD			shared_ReadFromMessageWord(HMESSAGEREAD hMessage);
DDWORD			shared_ReadFromMessageDWord(HMESSAGEREAD hMessage);
char*			shared_ReadFromMessageString(HMESSAGEREAD hMessage);
void			shared_ReadFromMessageVector(HMESSAGEREAD hMessage, DVector* pVal); // 12 bytes
void			shared_ReadFromMessageCompVector(HMESSAGEREAD hMessage, DVector* pVal); // 9 bytes
void			shared_ReadFromMessageCompPosition(HMESSAGEREAD hMessage, DVector* pVal); // 7 bytes
void			shared_ReadFromMessageRotation(HMESSAGEREAD hMessage, DRotation* pVal);
HOBJECT			shared_ReadFromMessageObject(HMESSAGEREAD hMessage);
HSTRING			shared_ReadFromMessageHString(HMESSAGEREAD hMessage);

// Use this only while loading objects (inside MID_LOADOBJECT).
DRESULT			shared_ReadFromLoadSaveMessageObject(HMESSAGEREAD hMessage, HOBJECT* hObject);

// Reads a data message from an HMESSAGEREAD.  The returned HMESSAGEREAD can then be used in the
// ReadFromMessageX functions.  This will create a new HMESSAGEREAD which must be
// freed with a call to EndHMessageRead().
// Inputs:
//		hMessage -		HMESSAGEREAD read from.
HMESSAGEREAD	shared_ReadFromMessageHMessageRead(HMESSAGEREAD hMessage);

// Frees a HMESSAGEREAD created from a call of ReadFromMessageHMessageRead.
void			shared_EndHMessageRead(HMESSAGEREAD hMessage);

// Frees a HMESSAGEWRITE created from a call of StartHMessageWrite.
void			shared_EndHMessageWrite(HMESSAGEWRITE hMessage);

// Reset reading (so you can read the message again).
// This is useful if you read out of a message and subclasses
// will be reading out of it.  Note: the message will AUTOMATICALLY
// reset when you hit the end, so you won't need this in most cases.
void			shared_ResetRead(HMESSAGEREAD hRead);

