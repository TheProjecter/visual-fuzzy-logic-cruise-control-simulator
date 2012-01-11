#include "Simulation.h"
#include "TinyOgre.h"
#include "Car.h"
#include "CasualAICar.h"
#include "RenderingObject.h"
#include <OgreStringConverter.h>
#include <OgreRenderWindow.h>

#include <cfloat>

const Ogre::Real Simulation::smLogicStep = 1.0 / 66.0;
const Ogre::Real Simulation::smMaxVelocity = 100;
const Ogre::Real Simulation::smAccelerationRate = 0.01;
const Ogre::Real Simulation::smDeccelerationRate = 0.005;
const Ogre::Real Simulation::smTurningRate = 2;
const Ogre::Real Simulation::smRoadSizeTimerLimitSec = 3;

Simulation::Simulation(TinyOgre * const renderer)
:   m_state(STARTUP),
    mLogicAccumulator(0), mLogicFPS(0),
    mRenderer(renderer),
    mCEGUIOgreRenderer(0),
    mCollision(0),
    mCars(),
    mRenderingObjects(),
    mPoints(0),
    mRazdalja(0), mPriblizevanje(0), mHitrost(0), mSignal(0),
	mDistance(5), mApproaching(0), mSpeed(-50), mJakost(0),
    mCruiseControlSpeed(50), mCruiseControlSpeedDelta(0),
	mCruiseControlEnabled(false), mCCTempDisabled(false),
	mExperiments(new FuzzyLogic::CExperiments), parser(0),
	mSignalX(0), mSignalY(0), mDistanceY(0), mApproachingY(0), mSpeedY(0)
{
    if (renderer == NULL)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "renderer is NULL!", "Simulation::ctor()");
    }

	//TestSamples *ss = new TestSamples();
	//TestSamples ss;

    // CEGUI init
    // TODO: move somewhere else?
    mCEGUIOgreRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
    CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
    CEGUI::Font::setDefaultResourceGroup("Fonts");
    CEGUI::Scheme::setDefaultResourceGroup("Schemes");
    CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
    CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

    // skin
    CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");

    // mouse cursor
    CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow");
    CEGUI::MouseCursor::getSingleton().setImage( CEGUI::System::getSingleton().getDefaultMouseCursor());

    // load GUI layout
    CEGUI::Window *guiRoot = CEGUI::WindowManager::getSingleton().loadWindowLayout("test1.layout");
    guiRoot->getChild("Root/Button")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&Simulation::quit, this));
    CEGUI::System::getSingleton().setGUISheet(guiRoot);

    // load track and generate a collision matrix
    const Ogre::Mesh * const trackMesh = mRenderer->loadTrack();
    mCollision = OGRE_NEW_T(Collision, Ogre::MEMCATEGORY_GENERAL);
    mCollision->generateCollisionMatrix(trackMesh, 5);

    // load players
    RenderingObject *car1RO = mRenderer->createPlayerCar();
    Car *car1 = OGRE_NEW_T(Car, Ogre::MEMCATEGORY_GENERAL)(car1RO->getSceneNode(), mCollision);
    //car1->getSceneNode()->setPosition(car1->getSceneNode()->getPosition() + Ogre::Vector3::UNIT_X * 4.0);
    mCars.push_back(car1);
    mRenderingObjects.push_back(car1RO);

	// AI car
	const Ogre::int32 aiCarNum = 13;
	for (Ogre::int32 i = 0; i < aiCarNum; ++i)
	{
		RenderingObject *car2RO = mRenderer->createAICar();
		Car *car2 = OGRE_NEW_T(CasualAICar, Ogre::MEMCATEGORY_GENERAL)(car2RO->getSceneNode(), mCollision, Ogre::Math::RangeRandom(0.2, 1.0), Ogre::Math::RangeRandom(0, 30));
		car2->getSceneNode()->setPosition(car2->getSceneNode()->getPosition() + Ogre::Vector3::UNIT_Z * 10.0 * i);
		mCars.push_back(car2);
		mRenderingObjects.push_back(car2RO);
	}

	//// AI car2
	//for (Ogre::int32 i = 0; i < aiCarNum; ++i)
	//{
	//	RenderingObject *car2RO = mRenderer->createAICar();
	//	Car *car2 = OGRE_NEW_T(CasualAICar, Ogre::MEMCATEGORY_GENERAL)(car2RO->getSceneNode(), mCollision, Ogre::Math::RangeRandom(0.2, 1.0), Ogre::Math::RangeRandom(0, 30));
	//	car2->getSceneNode()->setPosition(car2->getSceneNode()->getPosition() + Ogre::Vector3::UNIT_Z * 5.0 * i);
	//	mCars.push_back(car2);
	//	mRenderingObjects.push_back(car2RO);
	//}

    // 1
 //   RenderingObject *car2RO = mRenderer->createAICar();
 //   Car *car2 = OGRE_NEW_T(CasualAICar, Ogre::MEMCATEGORY_GENERAL)(car2RO->getSceneNode(), mCollision);
 //   mCars.push_back(car2);
 //   mRenderingObjects.push_back(car2RO);

    // 2
//    RenderingObject *car3RO = mRenderer->createAICar();
//    Car *car3  = OGRE_NEW_T(TwitchyAICar, Ogre::MEMCATEGORY_GENERAL)(car3RO->getSceneNode(), mCollision);
//    mCars.push_back(car3);
//    mRenderingObjects.push_back(car3RO);

    // set camera
    Ogre::Camera *cam = mRenderer->getCamera();
    cam->setNearClipDistance(1);
    cam->setPosition( Ogre::Vector3::UNIT_Z * 0.1 );
    cam->moveRelative( Ogre::Vector3::UNIT_Y * 100 );
    cam->moveRelative( Ogre::Vector3::UNIT_Z * 50 );
    cam->setAutoTracking(true, car1RO->getSceneNode());//, Ogre::Vector3::UNIT_X * 1000);

    OgreAssert(mCars.size() == mRenderingObjects.size(), "ROs and Cars not equal sizes!");

	//
	// Saso fuzzy logic
	//
	parser = new CFuzzyLanguageParser(mExperiments);
	
	reloadRules(); // load rules and init charts
}

Simulation::~Simulation()
{
	// GOGO team memleak!	
	delete mRazdalja;
	delete mPriblizevanje;
	delete mHitrost;
	delete mSignal;

	delete mExperiments;
	mExperiments = 0;
	delete parser;

    for (std::vector<Car*>::iterator i = mCars.begin(); i != mCars.end(); ++i)
    {
        OGRE_DELETE_T(*i, Car, Ogre::MEMCATEGORY_GENERAL);
    }

    // i should probably delete Car objects first, since they have a reference to RO's scene node
    for (std::vector<RenderingObject*>::iterator i = mRenderingObjects.begin(); i != mRenderingObjects.end(); ++i)
    {
        OGRE_DELETE_T(*i, RenderingObject, Ogre::MEMCATEGORY_GENERAL);
    }

    OGRE_DELETE_T(mCollision, Collision, Ogre::MEMCATEGORY_GENERAL);
}

SimulationState Simulation::getCurrentState() const
{
    return m_state;
}

// for the sake of clarity, I am not using actual thread synchronization
// objects to serialize access to this resource. You would want to protect
// this block with a mutex or critical section, etc.
bool Simulation::lockState()
{
    if (m_locked == false)
    {
        m_locked = true;
        return true;
    }
    else
        return false;
}

bool Simulation::unlockState()
{
    if (m_locked == true)
    {
        m_locked = false;
        return true;
    }
    else
        return false;
}

bool Simulation::requestStateChange(SimulationState newState)
{
    if (m_state == STARTUP)
    {
        m_locked = false;
        m_state = newState;

        return true;
    }

    // this state cannot be changed once initiated
    if (m_state == SHUTDOWN)
    {
        return false;
    }

    if ((m_state == GUI || m_state == SIMULATION || m_state == LOADING || m_state == CANCEL_LOADING) &&
            (newState != STARTUP) && (newState != m_state))
    {
        m_state = newState;
        return true;
    }
    else
        return false;
}

bool Simulation::playerCarCollision()
{
    OgreAssert(mCars.size() > 0, "mCars size is 0!");
    const Car * const playerCar = mCars.at(0);
    for (std::vector<Car*>::iterator i = ++mCars.begin(); i != mCars.end(); ++i)
    {
        const Car * const car = *i;

        // collision check
        if (playerCar->getSceneNode()->getPosition().positionEquals(car->getSceneNode()->getPosition(), 2))
        {
            return true;
        }
    }

    return false;
}

bool Simulation::frameLogicUpdate(const Ogre::Real timeSinceLastFrame)
{
	bool playerCollision = false; // for cruise control
    Car &playerCar = *(mCars.at(0));

    mLogicAccumulator += timeSinceLastFrame;
    while (mLogicAccumulator > smLogicStep)
    {
        // update cars
        for (std::vector<Car*>::iterator i = mCars.begin(); i != mCars.end(); ++i)
        {
            Car &car = **i;
            if (car.getActionState() != EXPLODE)
            {
                car.update(smLogicStep);
            }
        }

        // add some points to the player every logic frame!
        // TODO: maybe i should move this and mPoints to Car::update()?
        if (playerCar.getActionState() != EXPLODE)
        {
            mPoints += playerCar.getVelocity() * 0.025;

            if (playerCarCollision())
            {
				playerCollision = true;

                Ogre::LogManager::getSingletonPtr()->logMessage( "collision!" );
            }
        }

        mLogicAccumulator -= smLogicStep;
        ++mLogicFPS;
    }

//    const Ogre::Real alpha = accumulator / dt;
//    State state = currentState*alpha + previousState * ( 1.0 - alpha );

			if (mCruiseControlEnabled)
		{
			Car const *aheadCar = 0;
			if (playerCollision == false)
			{
				// get the car in front of us
				const Ogre::Vector3 playerPos = playerCar.getSceneNode()->getPosition();
				const Ogre::Quaternion playerOrient = playerCar.getSceneNode()->getOrientation();
				const Ogre::Vector3 playerDest = playerOrient * Ogre::Vector3::UNIT_Z;

				Ogre::Real aheadMinDistance = FLT_MAX;
				for (std::vector<Car*>::iterator i = ++mCars.begin(); i != mCars.end(); ++i)
				{
					Car const * const car = *i;

					const Ogre::Vector3 carPos = car->getSceneNode()->getPosition();
					const Ogre::Quaternion carOrient = car->getSceneNode()->getOrientation();
					const Ogre::Vector3 carDest = carOrient * Ogre::Vector3::UNIT_Z;

					const Ogre::Real dist = playerPos.distance(carPos);
					const Ogre::Vector3 nextPlayerPos = playerPos + (playerDest * dist);
		
					// if the car is in FRONT of us, consider it..
					if (nextPlayerPos.distance(carPos) < 3.0)
					{
						if (aheadMinDistance > dist)
						{
							aheadMinDistance = dist;
							aheadCar = car;
						}
					}
				}
			}

			// change cruise control velocity
			mCruiseControlSpeed += mCruiseControlSpeedDelta;
			mCruiseControlSpeed = Ogre::Math::Clamp(mCruiseControlSpeed, (Ogre::Real)0.0, Simulation::smMaxVelocity);

			// calculate fuzzy logic input values
			mSpeed = playerCar.getVelocity() - mCruiseControlSpeed;
			if (aheadCar)
			{
				const Ogre::Real dist = playerCar.getSceneNode()->getPosition().distance(aheadCar->getSceneNode()->getPosition());
				mDistance = dist / (playerCar.getVelocity() / 10.0 * 3.0);
				mApproaching = playerCar.getVelocity() - aheadCar->getVelocity();
			}
			else if (playerCollision)
			{
				//mDistance = 0.0;
				//mApproaching = 50.0;
				mDistance = (mExperiments->GetB_ByName("Distance", "Average") + mExperiments->GetC_ByName("Distance", "Average")) / 2.0;
				mApproaching = mExperiments->GetD_ByName("Approaching", "Fast") - 0.0001;
			}
			else
			{
				//mDistance = 5.0;
				//mApproaching = 0.0;
				mDistance = mExperiments->GetD_ByName("Distance", "High") - 0.0001;
				mApproaching = mExperiments->GetA_ByName("Approaching", "Slow") + 0.0001;
			}

			//mDistance = Ogre::Math::Clamp(mDistance, (Ogre::Real)0.0, (Ogre::Real)5.0);
			//mApproaching = Ogre::Math::Clamp(mApproaching, (Ogre::Real)0.0, (Ogre::Real)50.0);
			//mSpeed = Ogre::Math::Clamp(mSpeed, (Ogre::Real)-50.0, (Ogre::Real)50.0);
			mDistance = Ogre::Math::Clamp(mDistance, (Ogre::Real)mExperiments->GetA_ByName("Distance", "Low") + (Ogre::Real)0.0001, (Ogre::Real)mExperiments->GetD_ByName("Distance", "High") - (Ogre::Real)0.0001);
			mApproaching = Ogre::Math::Clamp(mApproaching, (Ogre::Real)mExperiments->GetA_ByName("Approaching", "Slow") + (Ogre::Real)0.0001, (Ogre::Real)mExperiments->GetD_ByName("Approaching", "Fast") - (Ogre::Real)0.0001);
			mSpeed = Ogre::Math::Clamp(mSpeed, (Ogre::Real)mExperiments->GetA_ByName("Speed", "Slow") + (Ogre::Real)0.0001, (Ogre::Real)mExperiments->GetD_ByName("Speed", "Fast") - (Ogre::Real)0.0001);

			// Saso fuzzy logic
			hash_map <std::string, FuzzyLogic::CFuzzyInput*> h_fuzzyInputs;
			FuzzyLogic::CFuzzyInput inputDistance("Distance", mDistance);
			FuzzyLogic::CFuzzyInput inputApproaching("Approaching", mApproaching);
			FuzzyLogic::CFuzzyInput inputSpeed("Speed", mSpeed);
			h_fuzzyInputs.insert(CFuzzyInput_Pairs(inputDistance.GetVariableName(), &inputDistance));
			h_fuzzyInputs.insert(CFuzzyInput_Pairs(inputApproaching.GetVariableName(), &inputApproaching));
			h_fuzzyInputs.insert(CFuzzyInput_Pairs(inputSpeed.GetVariableName(), &inputSpeed));

			mSignalX = mExperiments->CalculateFuzzyRules(h_fuzzyInputs);
			mSignalY = std::max(mExperiments->GetLinguisticVariableValue("Signal","Brake"), std::max(mExperiments->GetLinguisticVariableValue("Signal","Maintain"), mExperiments->GetLinguisticVariableValue("Signal","Accelerate")));
			mDistanceY = std::max(mExperiments->GetLinguisticVariableValue("Distance","Low"), std::max(mExperiments->GetLinguisticVariableValue("Distance","Average"), mExperiments->GetLinguisticVariableValue("Distance","High")));
			mApproachingY = std::max(mExperiments->GetLinguisticVariableValue("Approaching","Slow"), std::max(mExperiments->GetLinguisticVariableValue("Approaching","Average"), mExperiments->GetLinguisticVariableValue("Approaching","Fast")));
			mSpeedY = std::max(mExperiments->GetLinguisticVariableValue("Speed","Slow"), std::max(mExperiments->GetLinguisticVariableValue("Speed","Acceptable"), mExperiments->GetLinguisticVariableValue("Speed","Fast")));

			// fuzzy logic-based decision -- cruise control
			Ogre::Real velocityDelta =  mSignalX / (mExperiments->GetD_ByName("Signal", "Accelerate") - mExperiments->GetA_ByName("Signal", "Brake"));
			velocityDelta *= 5.0;
			playerCar.setVelocityByCC(velocityDelta);

			//Ogre::LogManager::getSingletonPtr()->logMessage( "" );
	  //      Ogre::LogManager::getSingletonPtr()->logMessage( "veckratnik:" + Ogre::StringConverter::toString(mDistance) );
			//Ogre::LogManager::getSingletonPtr()->logMessage( "blizanje:" + Ogre::StringConverter::toString(mApproaching) );
			//Ogre::LogManager::getSingletonPtr()->logMessage( "razlika:" + Ogre::StringConverter::toString(mSpeed) );			
			//Ogre::LogManager::getSingletonPtr()->logMessage( "signal:" + Ogre::StringConverter::toString((Ogre::Real)mSignalX) );	
			//Ogre::LogManager::getSingletonPtr()->logMessage( "signalZaviraj:" + Ogre::StringConverter::toString((Ogre::Real)mExperiments->GetLinguisticVariableValue("Signal", "Brake")) );	
			//Ogre::LogManager::getSingletonPtr()->logMessage( "signalVzdrzuj:" + Ogre::StringConverter::toString((Ogre::Real)mExperiments->GetLinguisticVariableValue("Signal", "Maintain")) );	
			//Ogre::LogManager::getSingletonPtr()->logMessage( "signalPospesuj:" + Ogre::StringConverter::toString((Ogre::Real)mExperiments->GetLinguisticVariableValue("Signal", "Accelerate")) );	
			//Ogre::LogManager::getSingletonPtr()->logMessage( "dolzina:" + Ogre::StringConverter::toString( (Ogre::Real)(mExperiments->GetD_ByName("Signal", "Accelerate") - mExperiments->GetA_ByName("Signal", "Brake")) ));
			//Ogre::LogManager::getSingletonPtr()->logMessage( "delta:" + Ogre::StringConverter::toString( (Ogre::Real)velocityDelta ));	

			mExperiments->ResetTokenValues();
		} 

    return true;
}


bool Simulation::frameRenderUpdate(const Ogre::Real timeSinceLastFrame)
{
	if (mCruiseControlEnabled)
	{
		OgreAssert(mExperiments, "j2s NULL!");
		mRazdalja->indicator(mDistance, mDistanceY);
		mPriblizevanje->indicator(mApproaching, mApproachingY);
		mHitrost->indicator(mSpeed, mSpeedY);
		mSignal->indicator(mSignalX, mSignalY);
	}

    // update cars
//    for (std::vector<RenderingObject*>::iterator i = mRenderingObjects.begin(); i != mRenderingObjects.end(); ++i)
    OgreAssert(mCars.size() == mRenderingObjects.size(), "ROs and Cars not equal sizes!");
    for (Ogre::int32 i = 0; i < mCars.size(); ++i)
    {
        mRenderingObjects.at(i)->visualizeActionState( mCars.at(i)->getActionState() );
    }

    // adjust camera position
    OgreAssert(mCars.size() > 0, "mCars size is 0!");
    Car * const playerCar = mCars.at(0);

    // Static camera
    // mRenderer->getCamera()->setPosition(playerCarPos.x, mRenderer->getCamera()->getPosition().y, playerCarPos.z + 0.1);
	// not-so-static camera
    const Ogre::Vector3 cameraPos = (Ogre::Vector3::UNIT_Y * (40.0 - playerCar->getVelocity() * 0.3)) + (Ogre::Vector3::UNIT_Z * (-40.0 + playerCar->getVelocity() * 0.01));
    const Ogre::Vector3 playerCarPos = playerCar->getSceneNode()->getPosition();
    const Ogre::Quaternion playerCarQ = playerCar->getSceneNode()->getOrientation();
    mRenderer->getCamera()->setPosition(playerCarPos);
    mRenderer->getCamera()->move(playerCarQ * cameraPos);

    // update GUI
    std::stringstream pointsSS; // NOTE: Ogre doesn't provide this in its StringConverter class!
    pointsSS << static_cast<Ogre::uint64>(mPoints);
    const Ogre::String velocityString = Ogre::StringConverter::toString(static_cast<Ogre::int32>( playerCar->getVelocity() ));
	const Ogre::String ccVelocityString = mCruiseControlEnabled ? Ogre::StringConverter::toString(static_cast<Ogre::int32>( mCruiseControlSpeed )) + " km/h" : "disabled";

    CEGUI::WindowManager::getSingleton().getWindow("Root/CarVelocity")->setText( velocityString + " km/h" );
    CEGUI::WindowManager::getSingleton().getWindow("Root/CCVelocity")->setText( ccVelocityString );
    CEGUI::WindowManager::getSingleton().getWindow("Root/Points")->setText( pointsSS.str() );
    CEGUI::WindowManager::getSingleton().getWindow("Root/RenderFPS")->setText( Ogre::StringConverter::toString(static_cast<int>(mRenderer->getRoot()->getAutoCreatedWindow()->getAverageFPS())) + " FPS" );

    static Ogre::Real accumulatorFPS = 1.01f; // debug logic FPS
    accumulatorFPS += timeSinceLastFrame;
    if (accumulatorFPS > 1.0)
    {
        CEGUI::WindowManager::getSingleton().getWindow("Root/LogicFPS")->setText(Ogre::StringConverter::toString(mLogicFPS) + " updates");
        accumulatorFPS = 0.0;
        mLogicFPS = 0;
    }

    return true;
}

bool Simulation::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    // for ALT+F4 and "x" (close) windows events
    if (mRenderer->getRoot()->getAutoCreatedWindow()->isClosed())
    {
        return false;
    }

    // do logic update
    if (frameLogicUpdate(evt.timeSinceLastFrame) == false)
    {
        return false;
    }

    // do rendering update
    if (frameRenderUpdate(evt.timeSinceLastFrame) == false)
    {
        return false;
    }

    return true;
}

void Simulation::setAcceleration(const bool accel)
{
    OgreAssert(mCars.size() > 0, "mCars size is 0!");
    Car * const playerCar = mCars.at(0);
    if (playerCar->getActionState() != EXPLODE)
    {
        playerCar->setAcceleration(accel);
    }

	// temporarily disable CC on acceleration
	if (mCruiseControlEnabled && accel)
	{
		mCCTempDisabled = true;
		setCCToggle();
	}
	else if (mCCTempDisabled)
	{
		mCCTempDisabled = false;
		setCCToggle();
	}

}

void Simulation::setBrake(const bool brake)
{
    OgreAssert(mCars.size() > 0, "mCars size is 0!");
    Car &playerCar = *(mCars.at(0));
    if (playerCar.getActionState() != EXPLODE)
    {
        playerCar.setBrake(brake);

		// disable CC on brake
		if (mCruiseControlEnabled)
		{
			setCCToggle();
		}
    }
}

void Simulation::setTurningDirection(const Ogre::int32 turnDir)
{
    OgreAssert(mCars.size() > 0, "mCars size is 0!");
    Car * const playerCar = mCars.at(0);
    if (playerCar->getActionState() != EXPLODE)
    {
        playerCar->setTurningDirection(turnDir);
    }
}

void Simulation::setCCVelocity(const Ogre::int32 turnDir)
{
	mCruiseControlSpeedDelta = (Ogre::Real)turnDir;
}

void Simulation::setCCToggle()
{
	mCruiseControlEnabled = !mCruiseControlEnabled;

	mRazdalja->setVisible(mCruiseControlEnabled);
	mPriblizevanje->setVisible(mCruiseControlEnabled);
	mHitrost->setVisible(mCruiseControlEnabled);
	mSignal->setVisible(mCruiseControlEnabled);
}

void Simulation::reloadRules()
{
	delete mExperiments;
	mExperiments = new CExperiments();
	parser->fuzzyInterface = mExperiments;
	parser->Parse("rules.txt");
	
    //
    // charts
    //
	mCruiseControlEnabled = false;
	mCCTempDisabled = false;		

	delete mRazdalja;
	mRazdalja = 0;
	delete mRazdalja;
	delete mPriblizevanje;
	mPriblizevanje = 0;
	delete mHitrost;
	mHitrost = 0;
	delete mSignal;
	mSignal = 0;
	TextRenderer::getSingleton().removeAll();

	mRazdalja = new Chart("Distance", -0.99, 0.13);
	mPriblizevanje = new Chart("Approaching", -0.99, -0.43);
	mHitrost = new Chart("Speed", -0.99, -0.99);
	mSignal = new Chart("Signal", 0.45, -0.99);

    ChartPointsVector majhna, srednja, velika;
    majhna.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Distance","Low"), 0 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Distance","Low"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Distance","Low"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Distance","Low"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Distance","Low"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Distance","Low"), 0 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Distance","Average"), 0 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Distance","Average"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Distance","Average"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Distance","Average"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Distance","Average"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Distance","Average"), 0 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Distance","High"), 0 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Distance","High"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Distance","High"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Distance","High"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Distance","High"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Distance","High"), 0 ) );

    mRazdalja->addPoints("low", majhna);
    mRazdalja->addPoints("average", srednja);
    mRazdalja->addPoints("high", velika);
    mRenderer->attachChart(*mRazdalja);

    majhna.clear();
    srednja.clear();
    velika.clear();
    majhna.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Approaching","Slow"), 0 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Approaching","Slow"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Approaching","Slow"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Approaching","Slow"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Approaching","Slow"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Approaching","Slow"), 0 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Approaching","Average"), 0 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Approaching","Average"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Approaching","Average"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Approaching","Average"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Approaching","Average"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Approaching","Average"), 0 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Approaching","Fast"), 0 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Approaching","Fast"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Approaching","Fast"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Approaching","Fast"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Approaching","Fast"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Approaching","Fast"), 0 ) );
    mPriblizevanje->addPoints("slow", majhna);
    mPriblizevanje->addPoints("average", srednja);
    mPriblizevanje->addPoints("fast", velika);
    mRenderer->attachChart(*mPriblizevanje);

    majhna.clear();
    srednja.clear();
    velika.clear();
    majhna.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Speed","Slow"), 0 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Speed","Slow"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Speed","Slow"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Speed","Slow"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Speed","Slow"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Speed","Slow"), 0 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Speed","Acceptable"), 0 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Speed","Acceptable"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Speed","Acceptable"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Speed","Acceptable"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Speed","Acceptable"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Speed","Acceptable"), 0 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Speed","Fast"), 0 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Speed","Fast"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Speed","Fast"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Speed","Fast"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Speed","Fast"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Speed","Fast"), 0 ) );
    mHitrost->addPoints("slow", majhna);
    mHitrost->addPoints("acceptable", srednja);
    mHitrost->addPoints("fast", velika);
    mRenderer->attachChart(*mHitrost);

    majhna.clear();
    srednja.clear();
    velika.clear();
    majhna.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Signal","Brake"), 0 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Signal","Brake"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Signal","Brake"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Signal","Brake"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Signal","Brake"), 1 ) );
    majhna.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Signal","Brake"), 0 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Signal","Maintain"), 0 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Signal","Maintain"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Signal","Maintain"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Signal","Maintain"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Signal","Maintain"), 1 ) );
    srednja.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Signal","Maintain"), 0 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetA_ByName("Signal","Accelerate"), 0 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Signal","Accelerate"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetB_ByName("Signal","Accelerate"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Signal","Accelerate"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetC_ByName("Signal","Accelerate"), 1 ) );
    velika.push_back( Ogre::Vector2( mExperiments->GetD_ByName("Signal","Accelerate"), 0 ) );
    mSignal->addPoints("brake", majhna);
    mSignal->addPoints("maintain", srednja);
    mSignal->addPoints("accelerate", velika);
    mRenderer->attachChart(*mSignal);

//	setCCToggle();
}