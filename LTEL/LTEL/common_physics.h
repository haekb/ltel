#pragma once
#include "LT1/AppHeaders/basedefs_de.h"
#include "LT1/AppHeaders/basetypes_de.h"
#include "LT1/AppHeaders/physics_lt.h"

#include "game_object.h"

class LTELCommonPhysics :
    public PhysicsLT
{
public:

	LTELCommonPhysics();
	~LTELCommonPhysics();

	// Get/Set friction coefficient.
	 DRESULT GetFrictionCoefficient(HOBJECT hObj, float& coeff);
	 DRESULT SetFrictionCoefficient(HOBJECT hObj, float coeff);

	// Get/set force ignore limit.
	 DRESULT GetForceIgnoreLimit(HOBJECT hObj, float& limit);
	 DRESULT SetForceIgnoreLimit(HOBJECT hObj, float limit);

	// Get/Set acceleration and velocity
	 DRESULT	GetVelocity(HOBJECT hObj, DVector* pVel);
	 DRESULT	SetVelocity(HOBJECT hObj, DVector* pVel);

	 DRESULT	GetAcceleration(HOBJECT hObj, DVector* pAccel);
	 DRESULT	SetAcceleration(HOBJECT hObj, DVector* pAccel);

	// Get/Set an object's mass (default is 30).
	 DRESULT GetObjectMass(HOBJECT hObj, float& mass);
	 DRESULT SetObjectMass(HOBJECT hObj, float mass);

	// Get the object's current dimensions.
	 DRESULT GetObjectDims(HOBJECT hObj, DVector* pNewDims);

	// Changes the object's dimensions without pushing against objects and world.
	// Flags is a combination of SETDIMS_ flags above.
	 DRESULT	SetObjectDims(HOBJECT hObj, DVector* pNewDims, DDWORD flags);

	// This function moves an object, colliding/pushing/crushing objects
	// in its way (if it's solid..)  Flags is a combination of MOVEOBJECT_ flags.
	 DRESULT MoveObject(HOBJECT hObj, DVector* pPos, DDWORD flags);

	// Find out what the object is standing on.
	 DRESULT GetStandingOn(HOBJECT hObj, CollisionInfo* pInfo);

	// Get the world object.
	 DRESULT GetWorldObject(HOBJECT* hObj);

	// Get/set global force.  This is an acceleration applied to all objects
	// when they move.  Default is (0,-2000,0) which simulates gravity.
	 DRESULT GetGlobalForce(DVector& vec);
	 DRESULT SetGlobalForce(DVector& vec);

public:
	DVector m_vGlobalForce;
};

