#ifndef CHART_H_
#define CHART_H_

#include <vector>

#include <OgreRoot.h>
#include <OgreManualObject.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <OgreLogManager.h>
#include <OgreStringConverter.h>

#include "TextRenderer.h"

typedef std::vector<Ogre::Vector2> ChartPointsVector;

class Chart
{
public:
     /** \brief ctor
     *
     * \param offsetX [-1,1] chart's offset on screen
     * \param offsetY [-1,1] chart's offset on screen
     */
	Chart(const std::string &caption, const Ogre::Real offsetX, const Ogre::Real offsetY);
	~Chart();

    /** \brief plots specified data points
     *
     * \param points a vector of chart's data points (X,Y)
     */
    void addPoints(const std::string &name, const ChartPointsVector &points);

	 /** \brief hides or shows the chart
     *
     * \param visible make the chart visible?
     */
	void setVisible(const bool visible);

    /** \brief attaches itself to the specified scene manager
     *
     * \param sceneMgr
     * \return scene node
     *
        TODO: do it right
     */
    Ogre::SceneNode * attachTo(Ogre::SceneManager &sceneMgr);

    /** \brief indicates the point on the graph by two perpendicular lines 
     *
     * \param chartX input
     * \param chartY value
     */
    void indicator(const Ogre::Real chartX, const Ogre::Real chartY);

protected:
    /**< convert coordinates from chart to screen coordinates*/
    Ogre::Vector3 chartToScreen(const Ogre::Vector2 &coords) const; 

protected:
    Ogre::SceneManager *mSceneMgr;
    Ogre::SceneNode *mChartSceneNode;
    Ogre::ManualObject *mChartIndicator;
    const Ogre::Vector2 mOffset, mSize, mPadding;
    std::vector<ChartPointsVector> mPoints;
    std::vector<std::string> mLineNames;
	const std::string mCaption;
    Ogre::Vector2 mMax, mMin, mChartUnitStep;

	Ogre::String rndIDString;
	bool first;
};

#endif // CHART_H_
