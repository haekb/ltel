#pragma once
#include "LT1/AppHeaders/basedefs_de.h"
#include "LT1/AppHeaders/basetypes_de.h"

#include "LT1/AppHeaders/physics_lt.h"

class LTELCommonPhysics :
    public PhysicsLT
{
public:
    LTELCommonPhysics();
    virtual ~LTELCommonPhysics();

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
};

