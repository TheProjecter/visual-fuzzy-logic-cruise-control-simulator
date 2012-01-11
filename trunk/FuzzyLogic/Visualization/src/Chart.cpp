#include "Chart.h"

Chart::Chart(const std::string &caption, const Ogre::Real offsetX, const Ogre::Real offsetY)
:   //Ogre::ManualObject("todo"),
    mChartSceneNode(0),
    mChartIndicator(0),
    mOffset(offsetX, offsetY),
    mSize(0.54, 0.55),
    mPadding(0.08, 0.08),
    mPoints(), mLineNames(),
	mCaption(caption),
    mMax(1.0, 1.0), mMin(0.0, 0.0)
{
	rndIDString = Ogre::StringConverter::toString(Ogre::Math::RangeRandom(0, 9999999) + Ogre::Math::RangeRandom(0, 9999999));
	first = true;
}

Chart::~Chart()
{
	setVisible(false);
	mSceneMgr->destroySceneNode(mChartSceneNode);
}

void Chart::addPoints(const std::string &name, const ChartPointsVector &points)
{
    for (ChartPointsVector::const_iterator it = points.begin(); it != points.end(); ++it)
    {
        mMax.x = std::max(mMax.x, it->x);
        mMax.y = std::max(mMax.y, it->y);
        mMin.x = std::min(mMin.x, it->x);
        mMin.y = std::min(mMin.y, it->y);
    }

    mChartUnitStep = (mSize - mPadding * 2.0) / (mMax - mMin);

    mLineNames.push_back(name);
    mPoints.push_back(points);
}

void Chart::setVisible(const bool visible)
{
	mChartSceneNode->setVisible(visible);
	TextRenderer::getSingleton().setVisible(visible);
}

Ogre::SceneNode * Chart::attachTo(Ogre::SceneManager &sceneMgr)
{
    mSceneMgr = &sceneMgr;

    if (mLineNames.size() < 3)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE, "I need exactly three line names. Yeah, it\'s hardcoded ;)", "Chart::attachTo()");
    }

    Ogre::LogManager::getSingletonPtr()->logMessage("mpoints: " + Ogre::StringConverter::toString(mPoints.size()));
    Ogre::LogManager::getSingletonPtr()->logMessage("msize: " + Ogre::StringConverter::toString(mSize));
    Ogre::LogManager::getSingletonPtr()->logMessage("mchartunitstep: " + Ogre::StringConverter::toString(mChartUnitStep));
    Ogre::LogManager::getSingletonPtr()->logMessage("mmax: " + Ogre::StringConverter::toString(mMax));
    Ogre::LogManager::getSingletonPtr()->logMessage("mmin: " + Ogre::StringConverter::toString(mMin));

    // chart quad bg
    Ogre::ManualObject *chartQuad = sceneMgr.createManualObject();
    chartQuad->setUseIdentityProjection(true);
    chartQuad->setUseIdentityView(true);
    chartQuad->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    chartQuad->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
    chartQuad->begin("Chart/Background", Ogre::RenderOperation::OT_TRIANGLE_LIST);
    chartQuad->position(mOffset.x, mOffset.y, 0.5);
    chartQuad->position(mOffset.x + mSize.x, mOffset.y, 0.5);
    chartQuad->position(mOffset.x + mSize.x, mOffset.y + mSize.y, 0.5);
    chartQuad->position(mOffset.x, mOffset.y + mSize.y, 0.5);
    chartQuad->position(mOffset.x, mOffset.y, 0.5);
    chartQuad->position(mOffset.x + mSize.x, mOffset.y + mSize.y, 0.5);
    chartQuad->end();

    // chart y axis
    Ogre::ManualObject *chartAxisY = sceneMgr.createManualObject();
    chartAxisY->setUseIdentityProjection(true);
    chartAxisY->setUseIdentityView(true);
    chartAxisY->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    chartAxisY->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
    chartAxisY->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
    chartAxisY->position(mOffset.x + mPadding.x, mOffset.y + mSize.y - 0.02, 0.0);
	chartAxisY->colour( 0.7, 0.7, 0.7 );
    chartAxisY->position(mOffset.x + mPadding.x, mOffset.y + 0.02, 0.0);
	chartAxisY->colour( 0.7, 0.7, 0.7 );
    chartAxisY->end();

    // chart x axis
    Ogre::ManualObject *chartAxisX = sceneMgr.createManualObject();
    chartAxisX->setUseIdentityProjection(true);
    chartAxisX->setUseIdentityView(true);
    chartAxisX->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    chartAxisX->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
    chartAxisX->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
    chartAxisX->position(mOffset.x + 0.02, mOffset.y + mPadding.y, 0.0);
	chartAxisX->colour( 0.7, 0.7, 0.7 );
    chartAxisX->position(mOffset.x + mSize.x - 0.02, mOffset.y + mPadding.y, 0.0);
	chartAxisX->colour( 0.7, 0.7, 0.7 );
    chartAxisX->end();

    // lines
    const Ogre::ColourValue colours[] = { Ogre::ColourValue(1.0, 0.2, 0.2),
                                          Ogre::ColourValue(0.0, 0.9, 0.0),
                                          Ogre::ColourValue(0.2, 0.2, 1.0) };

    Ogre::ManualObject *lines = sceneMgr.createManualObject();
    lines->setUseIdentityProjection(true);
    lines->setUseIdentityView(true);
    lines->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    lines->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
    lines->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
    for (std::vector<ChartPointsVector>::const_iterator it = mPoints.begin(); it != mPoints.end(); ++it)
    {
        for (ChartPointsVector::const_iterator jt = it->begin(); jt != it->end(); ++jt)
        {
            lines->position(chartToScreen(*jt));
            lines->colour(colours[it - mPoints.begin()]);
        }
    }
    lines->end();

	// caption text
	const Ogre::Real rndID = Ogre::Math::RangeRandom(0, 9999999999) + Ogre::Math::RangeRandom(0, 9999999999);
	TextRenderer::getSingleton().addTextBox("txt0" + Ogre::StringConverter::toString(rndID+ Ogre::Math::RangeRandom(0, 999999)), mCaption, mOffset.x + mSize.x * 0.40, mOffset.y + mSize.y, 100, 20, Ogre::ColourValue::Black);

    // text above the lines	
    TextRenderer::getSingleton().addTextBox("txt1" + Ogre::StringConverter::toString(rndID+ Ogre::Math::RangeRandom(0, 999999)), mLineNames[0], mOffset.x + mPadding.x + 0.0025, mOffset.y + mSize.y - mPadding.y + 0.04, 100, 20, colours[0]);
    TextRenderer::getSingleton().addTextBox("txt2" + Ogre::StringConverter::toString(rndID+ Ogre::Math::RangeRandom(0, 999999)), mLineNames[1], mOffset.x + mPadding.x + 0.0025 + (mSize.x - mPadding.x * 2.0) * 0.35, mOffset.y + mSize.y - mPadding.y + 0.04, 100, 20, colours[1]);
    TextRenderer::getSingleton().addTextBox("txt3" + Ogre::StringConverter::toString(rndID+ Ogre::Math::RangeRandom(0, 999999)), mLineNames[2], mOffset.x + mPadding.x + 0.0025 + (mSize.x - mPadding.x * 2.0) * 0.75, mOffset.y + mSize.y - mPadding.y + 0.04, 100, 20, colours[2]);

    // text on y axis (1)
    TextRenderer::getSingleton().addTextBox("txt4" + Ogre::StringConverter::toString(rndID+ Ogre::Math::RangeRandom(0, 999999)), "1", mOffset.x + mPadding.x - 0.02, mOffset.y + mSize.y - mPadding.y, 20, 20, Ogre::ColourValue::Black);

    // text on x axis
	Ogre::Real rndID2 = Ogre::Math::RangeRandom(0, 9999999999) + Ogre::Math::RangeRandom(0, 9999999999);
    for (std::vector<ChartPointsVector>::const_iterator it = mPoints.begin(); it != mPoints.end(); ++it)
    {
		Ogre::Real rndID3 = Ogre::Math::RangeRandom(0, 9999999999) + Ogre::Math::RangeRandom(0, 9999999999);
        for (ChartPointsVector::const_iterator jt = it->begin(); jt != it->end(); ++jt)
        {
            const Ogre::Real rndID4 = Ogre::Math::RangeRandom(0, 9999999999) + Ogre::Math::RangeRandom(0, 9999999999);
            const Ogre::Vector3 chartVec = chartToScreen(*jt);
			const std::string xString = Ogre::StringConverter::toString(jt->x).substr(0, std::min(Ogre::StringConverter::toString(jt->x).size(), (std::string::size_type)3));

            TextRenderer::getSingleton().addTextBox("txt100" + Ogre::StringConverter::toString(rndID2) + Ogre::StringConverter::toString(rndID3) + Ogre::StringConverter::toString(rndID4), xString, chartVec.x - 0.01 * xString.size(), mOffset.y + mPadding.y - 0.0025, 20, 20, Ogre::ColourValue::Black);
        }
    }

    mChartSceneNode = sceneMgr.getRootSceneNode()->createChildSceneNode();
    mChartSceneNode->attachObject(chartQuad);
    mChartSceneNode->attachObject(chartAxisY);
    mChartSceneNode->attachObject(chartAxisX);
    mChartSceneNode->attachObject(lines);

	setVisible(false);

    return mChartSceneNode;
}

void Chart::indicator(const Ogre::Real chartX, const Ogre::Real chartY)
{
	const std::string chartXString = Ogre::StringConverter::toString(chartX).substr(0, std::min(Ogre::StringConverter::toString(chartX).size(), (std::string::size_type)4));
	const std::string chartYString = Ogre::StringConverter::toString(chartY).substr(0, std::min(Ogre::StringConverter::toString(chartY).size(), (std::string::size_type)3));
	const Ogre::Vector3 x = chartToScreen( Ogre::Vector2(chartX, mMin.y) );
	const Ogre::Vector3 y = chartToScreen( Ogre::Vector2(mMin.x, chartY) );
	const Ogre::ColourValue magenta(1,0,1);

	// indicator
    Ogre::ManualObject *chartIndicator = mSceneMgr->createManualObject();
    chartIndicator->setUseIdentityProjection(true);
    chartIndicator->setUseIdentityView(true);
    chartIndicator->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    chartIndicator->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
    chartIndicator->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
    chartIndicator->position( x );
	chartIndicator->colour( magenta );
    chartIndicator->position( chartToScreen( Ogre::Vector2(chartX, chartY) ) );
	chartIndicator->colour( magenta );
    chartIndicator->position( y );
	chartIndicator->colour( magenta );
    chartIndicator->end();

	// text of values on the sides of the axes
	if(first)
	{
		TextRenderer::getSingleton().addTextBox("txt6" + rndIDString, chartXString, x.x - 0.01 * chartXString.size(), x.y - 0.035, 20, 20, magenta);
		TextRenderer::getSingleton().addTextBox("txt7" + rndIDString, chartYString, y.x - 0.035 - 0.01 * chartYString.size(), y.y - 0.0025, 20, 20, magenta);
		first = false;
	} 
	else
	{
		TextRenderer::getSingleton().removeTextBox("txt6" + rndIDString);
		TextRenderer::getSingleton().removeTextBox("txt7" + rndIDString);
		TextRenderer::getSingleton().addTextBox("txt6" + rndIDString, chartXString, x.x - 0.01 * chartXString.size(), x.y - 0.035, 20, 20, magenta);
		TextRenderer::getSingleton().addTextBox("txt7" + rndIDString, chartYString, y.x - 0.035 - 0.01 * chartYString.size(), y.y - 0.0025, 20, 20, magenta);
	}

    if (mChartIndicator)
    {
        mChartSceneNode->detachObject(mChartIndicator);
        mSceneMgr->destroyManualObject(mChartIndicator);
    }
    mChartIndicator = chartIndicator;
    mChartSceneNode->attachObject(mChartIndicator);

    mChartSceneNode->needUpdate();
}

Ogre::Vector3 Chart::chartToScreen(const Ogre::Vector2 &coords) const
{
    if (coords.x < mMin.x || coords.y < mMin.y || coords.x > mMax.x || coords.y > mMax.y)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "data point " + Ogre::StringConverter::toString(coords) + " are out of chart's bounds!", "Chart::chartToScreen()");
    }

//        const Ogre::Real screenWidth = Ogre::Root::getSingletonPtr()->getAutoCreatedWindow()->getWidth();
//        const Ogre::Real screenHeight = Ogre::Root::getSingletonPtr()->getAutoCreatedWindow()->getHeight();

    // move to chart's offset (0,0)
    // NOTE: 0.0025 places the points right by the side of the main axes, instead of ON them.
    Ogre::Vector2 screenCoords(mOffset + mPadding.x + 0.0025);

    // move to the data point on chart
    screenCoords += ((coords - mMin) * mChartUnitStep);
//        Ogre::LogManager::getSingletonPtr()->logMessage("screenCoords: " + Ogre::StringConverter::toString(screenCoords));

    return Ogre::Vector3(screenCoords.x, screenCoords.y, 0.0);
}
