#ifndef __InputHandler_h_
#define __InputHandler_h_

#include "Simulation.h"

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISMouse.h>
#include <OISKeyboard.h>

class TinyOgre;

class InputHandler :
		public OIS::MouseListener,
		public OIS::KeyListener
{
public:
    static CEGUI::MouseButton convertButton(const OIS::MouseButtonID buttonID);

public:
	InputHandler(Simulation * const sim, TinyOgre * const renderer, unsigned long hWnd);
	~InputHandler();

	void setWindowExtents(int width, int height) ;
	void capture();

	// MouseListener
	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID);

	// KeyListener
	bool keyPressed(const OIS::KeyEvent &evt);
	bool keyReleased(const OIS::KeyEvent &evt);

protected:
	OIS::InputManager *m_ois;
	OIS::Mouse *mMouse;
	OIS::Keyboard *mKeyboard;

    //	unsigned long m_hWnd;
    Simulation * const mSim;
    TinyOgre * const mRenderer; // TODO: mRenderer je unused!
    bool mIsTurnLeftPressed, mIsTurnRightPressed; /**< I need these to fix some car handling problems */

};

#endif // __Input_h_
