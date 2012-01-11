#ifndef __Simulation_h_
#define __Simulation_h_

#include <vector>
#include <map>

#include <OgrePrerequisites.h>
#include <OgreFrameListener.h>
#include "Collision.h"
#include <CEGUI.h>
#include <RendererModules/Ogre/CEGUIOgreRenderer.h>

#include "Chart.h"

#include "stdafx.h"
#include "FuzzyReasoner.h"
#include "FuzzyRule.h"
#include "FuzzyRuleToken.h"
#include "LinguisticVariable.h"
#include "LinguisticValue.h"
#include "FuzzyInput.h"
#include <hash_map>
#include "Experiments.h"
#include "FuzzyLanguageParser.h"

typedef pair <std::string, FuzzyLogic::CFuzzyInput*> CFuzzyInput_Pairs;

class TinyOgre;
class RenderingObject;
class Car;
class CasualAICar;
class TwitchyAICar;

typedef enum
{
    STARTUP,
    GUI,
    LOADING,
    CANCEL_LOADING,
    SIMULATION,
    SHUTDOWN
} SimulationState;

class Simulation : public Ogre::FrameListener
{
public:
    static const Ogre::Real smLogicStep; // semi-fixed logic updates
    static const Ogre::Real smMaxVelocity;
    static const Ogre::Real smAccelerationRate;
    static const Ogre::Real smDeccelerationRate;
    static const Ogre::Real smTurningRate;
    static const Ogre::Real smRoadSizeTimerLimitSec; // timer until you explode, when on roadside

public:
    explicit Simulation(TinyOgre * const renderer);
    ~Simulation();

    bool requestStateChange(SimulationState state);
    bool lockState();
    bool unlockState();
    SimulationState getCurrentState() const;

    bool playerCarCollision();

    bool frameLogicUpdate(const Ogre::Real timeSinceLastFrame);
    bool frameRenderUpdate(const Ogre::Real timeSinceLastFrame);
    bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    void setAcceleration(const bool accel);
    void setBrake(const bool brake);
    void setTurningDirection(const Ogre::int32 turnDir);
    void setCCVelocity(const Ogre::int32 turnDir);
	void setCCToggle();

	void reloadRules();

    bool quit(const CEGUI::EventArgs& e)
    {
        requestStateChange(SHUTDOWN);
        return true;
    }

protected:
    SimulationState m_state;
    bool m_locked;
    TinyOgre * const mRenderer;
    CEGUI::OgreRenderer *mCEGUIOgreRenderer;

    // for logic updates
    Ogre::Real mLogicAccumulator;
    Ogre::int32 mLogicFPS;

    // game stuff
    Ogre::Real mPoints;
    Collision *mCollision;
    std::vector<Car*> mCars;
    std::vector<RenderingObject*> mRenderingObjects;

    // chart
    Chart *mRazdalja, *mPriblizevanje, *mHitrost, *mSignal;
    Ogre::Real mDistance, mApproaching, mSpeed, mJakost;
    Ogre::Real mCruiseControlSpeed, mCruiseControlSpeedDelta;
	bool mCruiseControlEnabled, mCCTempDisabled;

	// Saso fuzzy logic
	FuzzyLogic::CExperiments *mExperiments;
	CFuzzyLanguageParser *parser;
	double mSignalX, mSignalY, mDistanceY, mApproachingY, mSpeedY;
};
	
#endif // __Simulation_h_
