#include "Car.h"
#include "Simulation.h"
#include <OgreLogManager.h>
#include <OgreStringConverter.h>

Car::Car(Ogre::SceneNode * const node, const Collision * const collision)
:   mSceneNode(node),
    mCollision(collision),
    mActionState(NORMAL),
    mAccelerate(false), mBrake(false),
    mVelocity(0), mTurningDirection(0),
    mRoadSideTimer(0)
{
    if (node == NULL)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "node is NULL!", "Car::ctor()");
    }
    if (collision == NULL)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "collision is NULL!", "Car::ctor()");
    }
}

Car::~Car()
{
}

void Car::setAcceleration(const bool accel)
{
    mAccelerate = accel;
}

void Car::setBrake(const bool brake)
{
	mBrake = brake;
}

void Car::setTurningDirection(const Ogre::int32 turnDir)
{
    mTurningDirection = turnDir;
}

void Car::update(const Ogre::Real timeSinceLastFrame)
{
    OgreAssert(mActionState != EXPLODE, "action state is EXPLODE!");

    const Ogre::Vector3 prevPlayerCarPos = mSceneNode->getPosition();

    // collision checking
    // TODO: intersects instead?
    const CollisionType colType = mCollision->collides(prevPlayerCarPos);
    switch (colType)
    {
    case ROAD:
        // carry on
        mActionState = NORMAL;
        mRoadSideTimer = 0.0;
        break;

    case OFFROAD:
        // then explode immediately
        mActionState = EXPLODE;
        mRoadSideTimer += Simulation::smRoadSizeTimerLimitSec + timeSinceLastFrame;
        break;

    case ROADSIDE:
        // handled below
        break;

    default:
        OgreAssert(false, "unknown collision type");
        break;
    }

    // if on roadside, add some friction, lower the max. velocity a bit and set the velocity simmering threshold to both accel and deccel
    const bool isOnRoadSide = colType == ROADSIDE;
    const Ogre::Real friction = (isOnRoadSide ? 0.15: 0.0);
    const Ogre::Real maxVelocity = Simulation::smMaxVelocity * (isOnRoadSide ? 0.75 : 1.0);
    const Ogre::Real velocitySimmerThreshold = Simulation::smMaxVelocity * 0.6;

	if (mAccelerate && mBrake)
	{
		// maintain (acceleration and brakes are on)
	}
    else if (mAccelerate)
    {
        // NOTE: +1.0 is so the minimum input will be above 0.0 and the extra +0.01 is for quicker rise to max. velocity
        mVelocity += Ogre::Math::Log(Simulation::smAccelerationRate * (Simulation::smMaxVelocity - mVelocity) + 1.01 - friction);
    }
    else if (mBrake)
    {
		mVelocity -= Ogre::Math::Log(Simulation::smDeccelerationRate * (mVelocity) + 1.01 + friction + 1.5);
    }
	else 
	{
		//mVelocity -= Ogre::Math::Log(Simulation::smDeccelerationRate * (mVelocity) + 1.01);
	}

    // driving speed limits
    if (mVelocity > maxVelocity)
    {
        // if really close to maxVelocity, just clamp to it
        if (Ogre::Math::RealEqual(mVelocity, maxVelocity, 0.5))
        {
            mVelocity = maxVelocity;
        }
        else
        {
            // else, lerp down to maxVelocity
            mVelocity -= 0.2;
        }
    }
    else if (mVelocity < 0.0)
    {
        mVelocity = 0.0;
    }

    // if driving on roadside... (must apply to both: acceleration and decceleration!)
    if (isOnRoadSide)
    {
        // we're driving too fast, hence we should simmer!
        if (mVelocity > velocitySimmerThreshold)
        {
            mActionState = SIMMER;

            // timer limit reached, explode
            mRoadSideTimer += timeSinceLastFrame;
            if (mRoadSideTimer >= Simulation::smRoadSizeTimerLimitSec)
            {
                mActionState = EXPLODE;
            }
        }
        else
        {
            // driving slow enough
            mActionState = NORMAL;
            mRoadSideTimer = 0.0;
        }
    }

    // turning
//    OgreAssert(mTurningDirection >= -1 && mTurningDirection <= 1, "Invalid mTurningDirection values!");
//    const Ogre::Real turningDirReal = mTurningDirection;
//    const Ogre::Radian yawTurn = Ogre::Radian(-turningDirReal * Simulation::smTurningRateDegrees);
//    mSceneNode->yaw( yawTurn * timeSinceLastFrame );
    if (mTurningDirection < 0) // turning left
    {
        mSceneNode->yaw( Ogre::Radian(Simulation::smTurningRate) * timeSinceLastFrame );
    }
    else if(mTurningDirection > 0) // turning right
    {
        mSceneNode->yaw( -Ogre::Radian(Simulation::smTurningRate) * timeSinceLastFrame );
    }
    else // not turning
    {
    }

    const Ogre::Vector3 dest = mSceneNode->getOrientation() * Ogre::Vector3::UNIT_Z;
    mSceneNode->translate( dest * mVelocity * timeSinceLastFrame );

    // get update position
    const Ogre::Vector3 playerCarPos = mSceneNode->getPosition();

    // handle current action state?
    switch (mActionState)
    {
    case NORMAL:
        break;

    case SIMMER:
        break;

    case EXPLODE:
        Car::mSceneNode->setPosition(prevPlayerCarPos);
        Car::mVelocity = 0.0;
        break;

    default:
        OgreAssert(false, "unknown action type");
        break;
    }
}

void Car::setVelocityByCC(const Ogre::Real delta)
{
	mVelocity += Ogre::Math::Clamp(delta, (Ogre::Real)-3.5, (Ogre::Real)3.5);
	mVelocity = Ogre::Math::Clamp(mVelocity, (Ogre::Real)0, Simulation::smMaxVelocity);
}