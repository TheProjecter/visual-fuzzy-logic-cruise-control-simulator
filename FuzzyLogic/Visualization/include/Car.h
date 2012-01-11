#ifndef __Car_h_
#define __Car_h_

#include <OgreSceneNode.h>
#include <OgreParticleSystem.h>
//#include <OgrePrerequisites.h>
#include "ActionState.h"

class Collision;

class Car
{
public:
    Car(Ogre::SceneNode * const node, const Collision * const collision);
    virtual ~Car();

    void setAcceleration(const bool accel);
	void setBrake(const bool brake);
    void setTurningDirection(const Ogre::int32 turnDir);
    virtual void update(const Ogre::Real timeSinceLastFrame);
	void setVelocityByCC(const Ogre::Real delta);

//    Ogre::Real getRoadSideTimer() const { return mRoadSideTimer; }
    ActionState getActionState() const { return mActionState; }
    Ogre::Real getVelocity() const { return mVelocity; }
    Ogre::SceneNode *getSceneNode() const { return mSceneNode; }

protected:
    Ogre::SceneNode * const mSceneNode;
    const Collision * const mCollision;

    ActionState mActionState;
    bool mAccelerate, mBrake;
    Ogre::Real mVelocity;
    Ogre::int32 mTurningDirection; /**< -1 means turn left, 0 means don't turn, 1 means turn right */
    Ogre::Real mRoadSideTimer; /**< if this timer reaches Simulation::smRoadSizeLimitSec, car explodes */
};

#endif // #ifndef __Car_h_
