#ifndef __Game_h_
#define __Game_h_

#include "TinyOgre.h"
#include <OgreWindowEventUtilities.h>
#include "Simulation.h"
#include "InputHandler.h"

class Game
{
public:
    explicit Game();
    ~Game();

    void loop();

protected:
    TinyOgre* renderer;
    Simulation* sim;
    InputHandler* handler;

};

#endif // #ifndef __Game_h_
