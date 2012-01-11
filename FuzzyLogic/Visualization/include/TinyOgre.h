
/*
-----------------------------------------------------------------------------
Filename:    TinyOgre.h
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#ifndef __TinyOgre_h_
#define __TinyOgre_h_

#include <OgreString.h>
#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>

#include "Chart.h"

class RenderingObject;

class TinyOgre
{
public:
    explicit TinyOgre();
    ~TinyOgre();

    void setup();

    void attachChart(Chart &chart)
    {
        OgreAssert(mSceneMgr, "scenemgr NULL!");
        chart.attachTo(*mSceneMgr);
    }

    RenderingObject * createPlayerCar();
    RenderingObject * createAICar();
    Ogre::Mesh const * loadTrack();

    Ogre::Camera * getCamera() const;
    Ogre::Root * getRoot() const;

protected:
    Ogre::Root *mRoot;
    Ogre::Camera *mCamera;
    Ogre::SceneManager *mSceneMgr;
    Ogre::RenderWindow *mWindow;

    Ogre::uint32 carCount;

    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;
};

#endif // #ifndef __TinyOgre_h_
