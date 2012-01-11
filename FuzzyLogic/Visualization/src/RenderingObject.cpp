#include "RenderingObject.h"
#include <OgreLogManager.h>

RenderingObject::RenderingObject(Ogre::SceneNode * const node, Ogre::ParticleSystem * const particleSystem, Ogre::SceneNode * const explosionNode)
:   mSceneNode(node),
    mParticleSystem(particleSystem),
    mExplosionSceneNode(explosionNode)
{
    if (node == NULL)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "node is NULL!", "RenderingObj::ctor()");
    }
    if (particleSystem == NULL)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "particleSystem is NULL!", "RenderingObj::ctor()");
    }
    if (explosionNode == NULL)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "explosionNode is NULL!", "RenderingObj::ctor()");
    }
}

RenderingObject::~RenderingObject()
{

}

void RenderingObject::visualizeActionState(const ActionState actionState)
{
    // handle current action state with something visual
    switch (actionState)
    {
    case NORMAL:
        mParticleSystem->setEmitting(false);
        mExplosionSceneNode->setVisible(false);
        break;

    case SIMMER:
//        Ogre::LogManager::getSingletonPtr()->logMessage("Simmering..");
        mParticleSystem->setEmitting(true);
        mExplosionSceneNode->setVisible(false);
        break;

    case EXPLODE:
//        Ogre::LogManager::getSingletonPtr()->logMessage("EXPLODEY!");
        mParticleSystem->setEmitting(false);
        mExplosionSceneNode->setVisible(true);
        mSceneNode->setVisible(false, false);
        break;

    default:
        OgreAssert(false, "unknown action type");
        break;
    }
}
