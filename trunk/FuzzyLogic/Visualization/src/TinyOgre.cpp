/*
-----------------------------------------------------------------------------
Filename:    TinyOgre.cpp
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
#include "TinyOgre.h"

#include <OgreLogManager.h>
#include <OgreFontManager.h>
#include <OgreViewport.h>
#include <OgreConfigFile.h>
#include <OgreEntity.h>
#include <OgreWindowEventUtilities.h>
#include <OgreMovableObject.h>
#include <OgreManualObject.h>
#include <OgreParticleSystem.h>
#include <OgreBillboardSet.h>
#include <OgreBillboard.h>
#include <OgreAxisAlignedBox.h>

#include "Collision.h"
#include "RenderingObject.h"

//-------------------------------------------------------------------------------------
TinyOgre::TinyOgre(void)
    : mRoot(NULL),
      mCamera(NULL),
      mSceneMgr(NULL),
      mWindow(NULL),
      carCount(0),
#ifdef _DEBUG
      mResourcesCfg("resources_d.cfg"),
      mPluginsCfg("plugins_d.cfg")
#else
      mResourcesCfg("resources.cfg"),
      mPluginsCfg("plugins.cfg")
#endif
{
    // construct Ogre::Root
    mRoot = OGRE_NEW Ogre::Root(mPluginsCfg);


}
//-------------------------------------------------------------------------------------
TinyOgre::~TinyOgre(void)
{
    OGRE_DELETE mRoot;
}

void TinyOgre::setup(void)
{
//-------------------------------------------------------------------------------------
    // setup resources
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

//-------------------------------------------------------------------------------------
    // configure
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->restoreConfig() || mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "Fuzzy cruise control visualization");
    }
    else
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE, "Error when trying to initialise the rendering system", "TinyOgre::setup()");
    }

//-------------------------------------------------------------------------------------
    // choose scenemanager
//    mSceneMgr = mRoot->createSceneManager("TerrainSceneManager");
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);

//-------------------------------------------------------------------------------------
    // create camera
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");
    Ogre::SceneNode *camNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("cam");
    camNode->attachObject(mCamera);

//-------------------------------------------------------------------------------------
    // create viewports
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));


//-------------------------------------------------------------------------------------
    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

//-------------------------------------------------------------------------------------
    // Create any re    source listeners (for loading screens)
    //createResourceListener();

//-------------------------------------------------------------------------------------
    // load resources
    //Ogre::ResourceGroupManager::getSingleton().addResourceLocation("models", "FileSystem", "General");
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    // NOTE: this fixes some prolem with font (ttf) loading for TextRenderer
    Ogre::ResourceManager::ResourceMapIterator iter = Ogre::FontManager::getSingleton().getResourceIterator();
        while (iter.hasMoreElements()) { iter.getNext()->load(); }

    new TextRenderer();

//-------------------------------------------------------------------------------------
    // Create the scene
    // World geometry
//    mSceneMgr->setWorldGeometry("terrain.cfg");


    // Set ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(1, 1, 1));

    // Create a light
//    Ogre::Light* l = mSceneMgr->createLight("MainLight");
//    l->setPosition(20,80,50);


    // Create a prefab plane
    Ogre::Entity *planeEnt = mSceneMgr->createEntity("Plane", Ogre::SceneManager::PT_PLANE);
    planeEnt->setMaterialName("BumpyMetal");

    Ogre::SceneNode *planeNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    planeNode->attachObject(planeEnt);
    planeNode->setPosition(0,-20,0);
    planeNode->scale(10,10,10);
    planeNode->pitch( Ogre::Degree(90 + 180) );

    OgreAssert(mSceneMgr, "scenemgr NULL");


//    // charts
//    ChartPointsVector majhna, srednja, velika;
//    majhna.push_back( Ogre::Vector2(0,1) );
//    majhna.push_back( Ogre::Vector2(1,1) );
//    majhna.push_back( Ogre::Vector2(2,0) );
//    majhna.push_back( Ogre::Vector2(5,0) );
//    srednja.push_back( Ogre::Vector2(0,0) );
//    srednja.push_back( Ogre::Vector2(1,0) );
//    srednja.push_back( Ogre::Vector2(2,1) );
//    srednja.push_back( Ogre::Vector2(3,1) );
//    srednja.push_back( Ogre::Vector2(4,0) );
//    srednja.push_back( Ogre::Vector2(5,0) );
//    velika.push_back( Ogre::Vector2(0,0) );
//    velika.push_back( Ogre::Vector2(3,0) );
//    velika.push_back( Ogre::Vector2(4,1) );
//    velika.push_back( Ogre::Vector2(5,1) );
//
//    Chart c1(-0.99, 0.13);
//    c1.addPoints("majhna", majhna);
//    c1.addPoints("srednja", srednja);
//    c1.addPoints("velika", velika);
//    c1.attachTo(*mSceneMgr);
//
//
//    Chart c2(-0.99, -0.43);
//    majhna.clear();
//    srednja.clear();
//    velika.clear();
//    majhna.push_back( Ogre::Vector2(0,1) );
//    majhna.push_back( Ogre::Vector2(10,1) );
//    majhna.push_back( Ogre::Vector2(20,0) );
//    majhna.push_back( Ogre::Vector2(50,0) );
//    srednja.push_back( Ogre::Vector2(0,0) );
//    srednja.push_back( Ogre::Vector2(10,0) );
//    srednja.push_back( Ogre::Vector2(20,1) );
//    srednja.push_back( Ogre::Vector2(30,1) );
//    srednja.push_back( Ogre::Vector2(40,0) );
//    srednja.push_back( Ogre::Vector2(50,0) );
//    velika.push_back( Ogre::Vector2(0,0) );
//    velika.push_back( Ogre::Vector2(30,0) );
//    velika.push_back( Ogre::Vector2(40,1) );
//    velika.push_back( Ogre::Vector2(50,1) );
//    c2.addPoints("pocasno", majhna);
//    c2.addPoints("srednje", srednja);
//    c2.addPoints("hitro", velika);
//    c2.attachTo(*mSceneMgr);
//
//    Chart c3(-0.99, -0.99);
//    majhna.clear();
//    srednja.clear();
//    velika.clear();
//    majhna.push_back( Ogre::Vector2(-50,1) );
//    majhna.push_back( Ogre::Vector2(-10,1) );
//    majhna.push_back( Ogre::Vector2(0,0) );
//    majhna.push_back( Ogre::Vector2(50,0) );
//    srednja.push_back( Ogre::Vector2(-50,0) );
//    srednja.push_back( Ogre::Vector2(-10,0) );
//    srednja.push_back( Ogre::Vector2(0,1) );
//    srednja.push_back( Ogre::Vector2(10,0) );
//    srednja.push_back( Ogre::Vector2(50,0) );
//    velika.push_back( Ogre::Vector2(-50,0) );
//    velika.push_back( Ogre::Vector2(0,0) );
//    velika.push_back( Ogre::Vector2(10,1) );
//    velika.push_back( Ogre::Vector2(50,1) );
//    c3.addPoints("nizka", majhna);
//    c3.addPoints("ustrezna", srednja);
//    c3.addPoints("visoka", velika);
//    c3.attachTo(*mSceneMgr);
//
//    Chart c4(0.45, 0.13);
//    majhna.clear();
//    srednja.clear();
//    velika.clear();
//    majhna.push_back( Ogre::Vector2(-5,1) );
//    majhna.push_back( Ogre::Vector2(-2,1) );
//    majhna.push_back( Ogre::Vector2(-1,0) );
//    majhna.push_back( Ogre::Vector2(5,0) );
//    srednja.push_back( Ogre::Vector2(-5,0) );
//    srednja.push_back( Ogre::Vector2(-2,0) );
//    srednja.push_back( Ogre::Vector2(-1,1) );
//    srednja.push_back( Ogre::Vector2(1,1) );
//    srednja.push_back( Ogre::Vector2(2,0) );
//    srednja.push_back( Ogre::Vector2(5,0) );
//    velika.push_back( Ogre::Vector2(-5,0) );
//    velika.push_back( Ogre::Vector2(1,0) );
//    velika.push_back( Ogre::Vector2(2,1) );
//    velika.push_back( Ogre::Vector2(5,1) );
//    c4.addPoints("zaviraj", majhna);
//    c4.addPoints("vzdrzuj", srednja);
//    c4.addPoints("pospesuj", velika);
//    c4.attachTo(*mSceneMgr);

//    c4.amplituda(4, 0.6);
}

RenderingObject *TinyOgre::createPlayerCar()
{
    const Ogre::String postfix = Ogre::StringConverter::toString(carCount++);

    // car mesh
    Ogre::Entity *ogreHead = mSceneMgr->createEntity("ogrehead.mesh");
    Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    headNode->attachObject(ogreHead);
    headNode->scale(0.05, 0.05, 0.05);
    headNode->setPosition(1 + (4 * carCount), 3, 1);

    // set particle system
    Ogre::ParticleSystem *smokeParticleSystem = mSceneMgr->createParticleSystem("simmer" + postfix, "Examples/Smoke");
//    smokeParticleSystem->setNonVisibleUpdateTimeout(1);
    Ogre::SceneNode* particleNode = headNode->createChildSceneNode();
    particleNode->attachObject(smokeParticleSystem);

    // set explosion animation
    Ogre::BillboardSet *mySet = mSceneMgr->createBillboardSet("explosionSet" + postfix);
    mySet->setMaterialName("Examples/Explosion");
//    Ogre::Billboard *myBillboard = mySet->createBillboard(0, 0, 0);
    Ogre::Billboard *myBillboard1 = mySet->createBillboard(0, 50, 0);
//    Ogre::Billboard *myBillboard2 = mySet->createBillboard(0, 25, 25);
    Ogre::SceneNode *explosionNode = headNode->createChildSceneNode();
    explosionNode->attachObject(mySet);
    explosionNode->scale(2, 2, 2);
    explosionNode->setVisible(false);
//    explosionNode->setPosition(0, 0, 0);

    // disable the effects until needed!
    smokeParticleSystem->setEmitting(false);
    explosionNode->setVisible(false);

    RenderingObject* ret = OGRE_NEW_T(RenderingObject, Ogre::MEMCATEGORY_GENERAL)(headNode, smokeParticleSystem, explosionNode);
    return ret;
}

RenderingObject *TinyOgre::createAICar()
{
    const Ogre::String postfix = Ogre::StringConverter::toString(carCount++);

    Ogre::Entity* ogreHead = mSceneMgr->createEntity("blueogrehead.mesh");
    Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    headNode->attachObject(ogreHead);
    headNode->scale(0.05, 0.05, 0.05);
    headNode->setPosition(1 + (4 * carCount), 3, 1);

    // set particle system
    Ogre::ParticleSystem *smokeParticleSystem = mSceneMgr->createParticleSystem("AIsimmer" + postfix, "Examples/Smoke");
//    smokeParticleSystem->setNonVisibleUpdateTimeout(1);
    Ogre::SceneNode* particleNode = headNode->createChildSceneNode();
    particleNode->attachObject(smokeParticleSystem);

    // set explosion animation
    Ogre::BillboardSet *mySet = mSceneMgr->createBillboardSet("AIexplosionSet" + postfix);
    mySet->setMaterialName("Examples/Explosion");
//    Ogre::Billboard *myBillboard = mySet->createBillboard(0, 0, 0);
    Ogre::Billboard *myBillboard1 = mySet->createBillboard(0, 0, 0);
//    Ogre::Billboard *myBillboard2 = mySet->createBillboard(0, 25, 25);
    Ogre::SceneNode *explosionNode = headNode->createChildSceneNode();
    explosionNode->attachObject(mySet);
    explosionNode->scale(2, 2, 2);
    explosionNode->setVisible(false);
//    explosionNode->setPosition(0, 0, 0);

    // disable the effects until needed!
    smokeParticleSystem->setEmitting(false);
    explosionNode->setVisible(false);

    RenderingObject* ret = OGRE_NEW_T(RenderingObject, Ogre::MEMCATEGORY_GENERAL)(headNode, smokeParticleSystem, explosionNode);
    return ret;
}

const Ogre::Mesh *TinyOgre::loadTrack()
{
    // mesh
    Ogre::ManualObject *groundMeshMO = mSceneMgr->createManualObject();
    groundMeshMO->setDynamic(false);
    groundMeshMO->estimateVertexCount(10);
    groundMeshMO->estimateIndexCount(10);

    groundMeshMO->begin("Road");
    groundMeshMO->position(50, 1, 50);
    groundMeshMO->textureCoord(50.0/300.0, 50.0/300.0);
    groundMeshMO->position(250, 1, 50);
    groundMeshMO->textureCoord(250.0/300.0, 50.0/300.0);
    groundMeshMO->position(50, 1, 250);
    groundMeshMO->textureCoord(50.0/300.0, 250.0/300.0);
    groundMeshMO->position(250, 1, 250);
    groundMeshMO->textureCoord(250.0/300.0, 250.0/300.0);
    groundMeshMO->position(0, 1, 300);
    groundMeshMO->textureCoord(0.0/300.0, 300.0/300.0);
    groundMeshMO->position(300, 1, 300);
    groundMeshMO->textureCoord(300.0/300.0, 300.0/300.0);
    groundMeshMO->position(300, 1, 0);
    groundMeshMO->textureCoord(300.0/300.0, 0.0/300.0);
    groundMeshMO->position(0, 1, 0);
    groundMeshMO->textureCoord(0.0/300.0, 0.0/300.0);

    groundMeshMO->triangle(0, 6, 7);
    groundMeshMO->triangle(1, 6, 0);
    groundMeshMO->triangle(3, 6, 1);
    groundMeshMO->triangle(5, 6, 3);
    groundMeshMO->triangle(2, 5, 3);
    groundMeshMO->triangle(2, 4, 5);
    groundMeshMO->triangle(4, 2, 7);
    groundMeshMO->triangle(2, 0, 7);
    groundMeshMO->end();


    groundMeshMO->begin("RoadSide");
    groundMeshMO->position(50, 1, 50);
    groundMeshMO->textureCoord(0.0/400.0, 0.0/400.0);
    groundMeshMO->position(250, 1, 50);
    groundMeshMO->textureCoord(300.0/400.0, 100.0/400.0);
    groundMeshMO->position(50, 1, 250);
    groundMeshMO->textureCoord(100.0/400.0, 300.0/400.0);
    groundMeshMO->position(250, 1, 250);
    groundMeshMO->textureCoord(300.0/400.0, 300.0/400.0);
    groundMeshMO->position(0, 1, 300);
    groundMeshMO->textureCoord(50.0/400.0, 350.0/400.0);
    groundMeshMO->position(300, 1, 300);
    groundMeshMO->textureCoord(350.0/400.0, 350.0/400.0);
    groundMeshMO->position(300, 1, 0);
    groundMeshMO->textureCoord(350.0/400.0, 50.0/400.0);
    groundMeshMO->position(0, 1, 0);
    groundMeshMO->textureCoord(50.0/400.0, 50.0/400.0);

    // outer
    groundMeshMO->position(-5, 1, -5);
    groundMeshMO->textureCoord(0.0/400.0, 0.0/400.0);
    groundMeshMO->position(305, 1, -5);
    groundMeshMO->textureCoord(400.0/400.0, 0.0/400.0);
    groundMeshMO->position(305, 1, 305);
    groundMeshMO->textureCoord(400.0/400.0, 400.0/400.0);
    groundMeshMO->position(-5, 1, 305);
    groundMeshMO->textureCoord(0.0/400.0, 400.0/400.0);
    groundMeshMO->triangle(7, 9, 8);
    groundMeshMO->triangle(7, 6, 9);
    groundMeshMO->triangle(6, 10, 9);
    groundMeshMO->triangle(5, 10, 6);
    groundMeshMO->triangle(5, 11, 10);
    groundMeshMO->triangle(11, 5, 4);
    groundMeshMO->triangle(11, 4, 8);
    groundMeshMO->triangle(4, 7, 8);

    // inner
    groundMeshMO->position(55, 1, 55);
    groundMeshMO->textureCoord(150.0/400.0, 150.0/400.0);
    groundMeshMO->position(245, 1, 55);
    groundMeshMO->textureCoord(200.0/400.0, 150.0/400.0);
    groundMeshMO->position(55, 1, 245);
    groundMeshMO->textureCoord(150.0/400.0, 200.0/400.0);
    groundMeshMO->position(245, 1, 245);
    groundMeshMO->textureCoord(200.0/400.0, 200.0/400.0);
    groundMeshMO->triangle(12, 1, 0);
    groundMeshMO->triangle(12, 13, 1);
    groundMeshMO->triangle(13, 3, 1);
    groundMeshMO->triangle(13, 15, 3);
    groundMeshMO->triangle(14, 3, 15);
    groundMeshMO->triangle(2, 3, 14);
    groundMeshMO->triangle(2, 14, 0);
    groundMeshMO->triangle(14, 12, 0);
    groundMeshMO->end();

    Ogre::Mesh *mesh = groundMeshMO->convertToMesh("groundMesh").getPointer();

    Ogre::SceneNode *groundMeshNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
    groundMeshNode->attachObject(mSceneMgr->createEntity("groundMesh"));

    return mesh;

//groundMeshMO->position(50, 1, 50);
//    groundMeshMO->textureCoord(0.3, 1);
//    groundMeshMO->position(250, 1, 50);
//    groundMeshMO->textureCoord(0.6, 1);
//    groundMeshMO->position(50, 1, 250);
//    groundMeshMO->textureCoord(0.3, 0);
//    groundMeshMO->position(250, 1, 250);
//    groundMeshMO->textureCoord(0.6, 0);
//    groundMeshMO->position(0, 1, 300);
//    groundMeshMO->textureCoord(0, 1);
//    groundMeshMO->position(300, 1, 300);
//    groundMeshMO->textureCoord(1, 1);
//    groundMeshMO->position(300, 1, 0);
//    groundMeshMO->textureCoord(1, 0);
//    groundMeshMO->position(0, 1, 0);
//    groundMeshMO->textureCoord(0, 0);


//        groundMeshMO->position(50, 1, 50);
//    groundMeshMO->textureCoord(0.5 + (0.3 * 0.5), 1);
//    groundMeshMO->textureCoord(0.0 + 1* 0.5, (0.3));
//    groundMeshMO->position(250, 1, 50);
//    groundMeshMO->textureCoord(0.5 + (0.6 * 0.5), 1);
//    groundMeshMO->textureCoord(0.0 + 0, (0.3));
//    groundMeshMO->position(50, 1, 250);
//    groundMeshMO->textureCoord(0.5 + (0.3 * 0.5), 0);
//    groundMeshMO->textureCoord(0.0 + 1* 0.5, (0.6));
//    groundMeshMO->position(250, 1, 250);
//    groundMeshMO->textureCoord(0.5 + (0.6 * 0.5), 0);
//    groundMeshMO->textureCoord(0.0 + 0, (0.6));
//    groundMeshMO->position(0, 1, 300);
//    groundMeshMO->textureCoord(0.5 + (0 * 0.5), 1);
//    groundMeshMO->textureCoord(0.0, 1);
//    groundMeshMO->position(300, 1, 300);
//    groundMeshMO->textureCoord(0.5 + (1 * 0.5), 1);
//    groundMeshMO->textureCoord(0.0 + (1 * 0.5), 1);
//    groundMeshMO->position(300, 1, 0);
//    groundMeshMO->textureCoord(0.5 + (1 * 0.5), 0);
//    groundMeshMO->textureCoord(0.0 + (1 * 0.5), 0);
//    groundMeshMO->position(0, 1, 0);
//    groundMeshMO->textureCoord(0.5 + (0 * 0.5), 0);
//    groundMeshMO->textureCoord(0.0, 0);
}

//Ogre::SceneNode *TinyOgre::getEntity(const Ogre::String &nameString) const
//{
//    if (nameString.length() <= 0)
//    {
//        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "nameString is NULL or zero-length!", "TinyOgre::getEntity()");
//    }
//
//    OgreAssert(mSceneMgr != NULL, "mSceneMgr is NULL!");
//    return mSceneMgr->getSceneNode(nameString);
//}

Ogre::Camera *TinyOgre::getCamera() const
{
    OgreAssert(mCamera != NULL, "mCamera is NULL!");

//    Ogre::SceneNode *sceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
//    sceneNode->attachObject(mCamera);

//    return sceneNode;
    return mCamera;
}

Ogre::Root *TinyOgre::getRoot() const
{
    OgreAssert(mRoot != NULL, "mRoot is NULL!");
    return mRoot;
}

