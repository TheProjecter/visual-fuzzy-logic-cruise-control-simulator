#include "CasualAICar.h"
#include "Collision.h"
#include "Simulation.h"

CasualAICar::CasualAICar(Ogre::SceneNode * const node, const Collision * const collision, const Ogre::Real maxVelocity, const Ogre::Real sight)
:   Car(node, collision),
	mMaxVelocity(maxVelocity),
	mSight(sight)
{
}

CasualAICar::~CasualAICar()
{
}

void CasualAICar::update(const Ogre::Real timeSinceLastFrame)
{
    const Ogre::Vector3 curPos = Car::mSceneNode->getPosition();
    const Ogre::Quaternion curOrient = Car::mSceneNode->getOrientation();
    const Ogre::Real curVelocity = Car::mVelocity; // NOTE: this does NOT represent the actual next frame (i'd need to multiply it with timeSinceLastFrame)

    const Ogre::Vector3 dest = curOrient * Ogre::Vector3::UNIT_Z;
    const Ogre::Vector3 nextPos = curPos + (dest * (curVelocity + mSight));

    // think
    const CollisionType colType = Car::mCollision->collides(nextPos);
    if (colType == OFFROAD || colType == ROADSIDE)
    {
        //Car::mVelocity = 0.0;
        Car::setAcceleration(false);
        Car::setTurningDirection(-1);
    }
    else
    {
        Car::setAcceleration(true);
        Car::setTurningDirection(0);
    }

    // if going too fast, take it easy...
    if (Car::mVelocity > Simulation::smMaxVelocity * mMaxVelocity)
    {
        Car::setAcceleration(false);
    }

    // do
    Car::update(timeSinceLastFrame);
}
