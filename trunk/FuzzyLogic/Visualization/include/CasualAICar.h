#ifndef __CasualAICar_h_
#define __CasualAICar_h_

#include "Car.h"
#include <OgreSceneNode.h>

class CasualAICar : public Car
{
public:
    CasualAICar(Ogre::SceneNode * const node, const Collision * const collision, const Ogre::Real maxVelocity = 0.6, const Ogre::Real sight = 0.0);
    ~CasualAICar();

    void update(const Ogre::Real timeSinceLastFrame);

protected:
	const Ogre::Real mMaxVelocity, mSight;
};

#endif // #ifndef __CasualAICar_h_

