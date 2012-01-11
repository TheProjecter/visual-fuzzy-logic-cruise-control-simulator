#ifndef __RenderingObject_h_
#define __RenderingObject_h_

#include <OgreSceneNode.h>
#include <OgreParticleSystem.h>
#include "ActionState.h"

/**< High-level class for individual game object's visuals (FX, animations...) */
class RenderingObject
{
public:
    RenderingObject(Ogre::SceneNode * const node, Ogre::ParticleSystem * const particleSystem, Ogre::SceneNode * const explosionNode);
    ~RenderingObject();

    void visualizeActionState(const ActionState actionState);

    Ogre::SceneNode * getSceneNode() const { return mSceneNode; }

protected:
    Ogre::SceneNode * const mSceneNode;
    Ogre::ParticleSystem * const mParticleSystem;
    Ogre::SceneNode * const mExplosionSceneNode;
};

#endif // __RenderingObject_h_
