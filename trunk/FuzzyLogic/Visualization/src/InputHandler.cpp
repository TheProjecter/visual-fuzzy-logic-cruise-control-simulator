#include "InputHandler.h"
#include "TinyOgre.h"
#include <OgreStringConverter.h>
#include <OgreException.h>

InputHandler::InputHandler(Simulation * const sim, TinyOgre * const renderer, const unsigned long hWnd)
:   mSim(sim), mRenderer(renderer),
    mIsTurnLeftPressed(false), mIsTurnRightPressed(false)
{
    if (sim == NULL)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "sim is NULL!", "InputHandler::ctor()");
    }
    if (renderer == NULL)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "renderer is NULL!", "InputHandler::ctor()");
    }

//	m_hWnd = hWnd;

    OIS::ParamList pl;
    pl.insert(OIS::ParamList::value_type("WINDOW", Ogre::StringConverter::toString(hWnd)));

    m_ois = OIS::InputManager::createInputSystem( pl );
    mMouse = static_cast<OIS::Mouse*>(m_ois->createInputObject( OIS::OISMouse, true ));
    mKeyboard = static_cast<OIS::Keyboard*>(m_ois->createInputObject( OIS::OISKeyboard, true));
    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);
}

InputHandler::~InputHandler()
{
    if (mMouse)
        m_ois->destroyInputObject(mMouse);

    if (mKeyboard)
        m_ois->destroyInputObject(mKeyboard);

    OIS::InputManager::destroyInputSystem(m_ois);
}

void InputHandler::capture()
{
    mMouse->capture();
    mKeyboard->capture();
}

void  InputHandler::setWindowExtents(const int width, const int height)
{
    //Set Mouse Region.. if window resizes, we should alter this to reflect as well
    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}


// MouseListener
bool InputHandler::mouseMoved(const OIS::MouseEvent &evt)
{
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.injectMouseMove(evt.state.X.rel, evt.state.Y.rel);

    // Scroll wheel.
    if (evt.state.Z.rel)
        sys.injectMouseWheelChange(evt.state.Z.rel / 120.0f);

    return true;
}

bool InputHandler::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID btn)
{
    CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(btn));
    return true;
}

bool InputHandler::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID btn)
{
    CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(btn));
    return true;
}


// KeyListener
bool InputHandler::keyPressed(const OIS::KeyEvent &evt)
{
    switch (evt.key)
    {
    case OIS::KC_W:
        mSim->setAcceleration(true);
        break;

    case OIS::KC_S:
        mSim->setBrake(true);
        break;

    case OIS::KC_A:
        mIsTurnLeftPressed = true;
        mSim->setTurningDirection(-1);
        break;

    case OIS::KC_D:
        mIsTurnRightPressed = true;
        mSim->setTurningDirection(1);
        break;

    case OIS::KC_C:
		mSim->setCCToggle();
        break;

    case OIS::KC_O:
        mSim->setCCVelocity(-1);
        break;

    case OIS::KC_P:
        mSim->setCCVelocity(1);
        break;

    case OIS::KC_SYSRQ:
        // take screenshot
        mRenderer->getRoot()->getAutoCreatedWindow()->writeContentsToFile("ScreenShot.png");
        break;

    default:
        break;
    }

    return true;
}

bool InputHandler::keyReleased(const OIS::KeyEvent &evt)
{
    switch (evt.key)
    {
    case OIS::KC_W:
        mSim->setAcceleration(false);
        break;

    case OIS::KC_S:
        mSim->setBrake(false);
        break;

    case OIS::KC_A:
        mIsTurnLeftPressed = false;
        if(mIsTurnRightPressed)
            mSim->setTurningDirection(1);
        else
            mSim->setTurningDirection(0);
        break;

    case OIS::KC_D:
        mIsTurnRightPressed = false;
        if(mIsTurnLeftPressed)
            mSim->setTurningDirection(-1);
        else
            mSim->setTurningDirection(0);
        break;

    case OIS::KC_O:
        mSim->setCCVelocity(0);
        break;

    case OIS::KC_P:
        mSim->setCCVelocity(0);
        break;

	case OIS::KC_L:
		mSim->reloadRules();
        break;

    case OIS::KC_ESCAPE:
        mSim->requestStateChange(SHUTDOWN);
        break;

    default:
        break;
    }

    return true;
}

CEGUI::MouseButton InputHandler::convertButton(OIS::MouseButtonID buttonID)
{
    switch (buttonID)
    {
    case OIS::MB_Left:
        return CEGUI::LeftButton;

    case OIS::MB_Right:
        return CEGUI::RightButton;

    case OIS::MB_Middle:
        return CEGUI::MiddleButton;

    default:
        return CEGUI::LeftButton;
    }
}
