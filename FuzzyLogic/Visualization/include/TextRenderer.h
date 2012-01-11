#ifndef TEXTREDNERER_H_
#define TEXTREDNERER_H_

#include <Ogre.h>
#include <OgreSingleton.h>

class TextRenderer : public Ogre::Singleton<TextRenderer>
{
private:

    Ogre::OverlayManager*    _overlayMgr;
    Ogre::Overlay*           _overlay;
    Ogre::OverlayContainer*  _panel;


public:
    TextRenderer();
    ~TextRenderer();

    void addTextBox(
        const std::string& ID,
        const std::string& text,
        Ogre::Real x, Ogre::Real y,
        Ogre::Real width, Ogre::Real height,
        const Ogre::ColourValue& color = Ogre::ColourValue(1.0, 1.0, 1.0, 1.0));

    void removeTextBox(const std::string& ID);
	void removeAll();

    void setText(const std::string& ID, const std::string& Text);
    const std::string& getText(const std::string& ID);

	void setVisible(const bool v)
	{
		if (v)
			_overlay->show();
		else
			_overlay->hide();
	}

};

#endif // TEXTREDNERER_H_
