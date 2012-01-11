#include "Game.h"
#include <OgreWindowEventUtilities.h>
#include <OgreTimer.h>

Game::Game()
{
    renderer = OGRE_NEW_T(TinyOgre, Ogre::MEMCATEGORY_GENERAL);
    renderer->setup();
    // NOTE: TinyOgre.getRoot() must be called after TinyOgre.setup()!

    // set up the input handlers
    unsigned long hWnd;
    renderer->getRoot()->getAutoCreatedWindow()->getCustomAttribute("WINDOW", &hWnd);

    sim = OGRE_NEW_T(Simulation, Ogre::MEMCATEGORY_GENERAL)(renderer);
    handler = OGRE_NEW_T(InputHandler, Ogre::MEMCATEGORY_GENERAL)(sim, renderer, hWnd);

    renderer->getRoot()->addFrameListener(sim);
}

Game::~Game()
{
    renderer->getRoot()->removeFrameListener(sim);

    OGRE_DELETE_T(handler, InputHandler, Ogre::MEMCATEGORY_GENERAL);
    OGRE_DELETE_T(sim, Simulation, Ogre::MEMCATEGORY_GENERAL);
    OGRE_DELETE_T(renderer, TinyOgre, Ogre::MEMCATEGORY_GENERAL);
}

void Game::loop()
{
    Ogre::Root * const root = renderer->getRoot();
//    Ogre::Timer *timer = new Ogre::Timer();
//    timer->reset();
//    unsigned long time = timer->getMilliseconds();

    sim->requestStateChange(SIMULATION);
    while (sim->getCurrentState() != SHUTDOWN)
    {
        handler->capture();
        Ogre::WindowEventUtilities::messagePump();

        // do logic update
//        const unsigned long now = timer->getMilliseconds();
//        Ogre::Real dt = static_cast<Ogre::Real>(now - time) / 1000.0;
//        time = now;
//
//        // note: max frame time to avoid spiral of death
//        if ( dt > 0.25 )
//            dt = 0.25;

//        if (sim->frameLogicUpdate(dt) == false)
//        {
//            sim->requestStateChange(SHUTDOWN);
//        }

        if (root->renderOneFrame() == false)
        {
            sim->requestStateChange(SHUTDOWN);
        }

        if (root->getAutoCreatedWindow()->isActive() == false)
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            WaitMessage();
#else
            sleep(1);
#endif

        }
        else
        {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            Sleep(1);
#else
//            sleep(1);
#endif
        }
    }

//    delete timer;
}
