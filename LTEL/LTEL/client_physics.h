#pragma once
#include "common_physics.h"
class LTELClientPhysics :
    public CPhysicsLT
{
public:
	LTELClientPhysics();
	~LTELClientPhysics();


	// These will just point to common physics until I can figure out a better way to redirect
public:
	// Get/Set friction coefficient.
	virtual DRESULT GetFrictionCoefficient(HOBJECT hObj, float& coeff);
	virtual DRESULT SetFrictionCoefficient(HOBJECT hObj, float coeff);

	// Get/set force ignore limit.
	virtual DRESULT GetForceIgnoreLimit(HOBJECT hObj, float& limit);
	virtual DRESULT SetForceIgnoreLimit(HOBJECT hObj, float limit);

	// Get/Set acceleration and velocity
	virtual DRESULT	GetVelocity(HOBJECT hObj, DVector* pVel);
	virtual DRESULT	SetVelocity(HOBJECT hObj, DVector* pVel);

	virtual DRESULT	GetAcceleration(HOBJECT hObj, DVector* pAccel);
	virtual DRESULT	SetAcceleration(HOBJECT hObj, DVector* pAccel);

	// Get/Set an object's mass (default is 30).
	virtual DRESULT GetObjectMass(HOBJECT hObj, float& mass);
	virtual DRESULT SetObjectMass(HOBJECT hObj, float mass);

	// Get the object's current dimensions.
	virtual DRESULT GetObjectDims(HOBJECT hObj, DVector* pNewDims);

	// Changes the object's dimensions without pushing against objects and world.
	// Flags is a combination of SETDIMS_ flags above.
	virtual DRESULT	SetObjectDims(HOBJECT hObj, DVector* pNewDims, DDWORD flags);

	// This function moves an object, colliding/pushing/crushing objects
	// in its way (if it's solid..)  Flags is a combination of MOVEOBJECT_ flags.
	virtual DRESULT MoveObject(HOBJECT hObj, DVector* pPos, DDWORD flags);

	// Find out what the object is standing on.
	virtual DRESULT GetStandingOn(HOBJECT hObj, CollisionInfo* pInfo);

	// Get the world object.
	virtual DRESULT GetWorldObject(HOBJECT* hObj);

	// Get/set global force.  This is an acceleration applied to all objects
	// when they move.  Default is (0,-2000,0) which simulates gravity.
	virtual DRESULT GetGlobalForce(DVector& vec);
	virtual DRESULT SetGlobalForce(DVector& vec);

	// New client functions!
public:

	// Updates the object's movement using its velocity, acceleration, and the
// time delta passed in (usually the frame time).  Fills in m_Offset with the
// position delta you should apply.
	virtual DRESULT UpdateMovement(MoveInfo* pInfo);

	// Move the specified object but only test for collisions/pushing on
	// the objects specified.  It'll carry things standing on it.
	virtual DRESULT MovePushObjects(HOBJECT hToMove, DVector& newPos,
		HOBJECT* hPushObjects, DDWORD nPushObjects);

	// Rotate the specified object but only test for collisions/pushing on
	// the objects specified.  It'll carry things standing on it.
	// This only works on world models.
	virtual DRESULT RotatePushObjects(HOBJECT hToMove, DRotation& newRot,
		HOBJECT* hPushObjects, DDWORD nPushObjects);

protected:

	LTELCommonPhysics* m_pCommonPhysics;
};

