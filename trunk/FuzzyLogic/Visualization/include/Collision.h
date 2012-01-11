#ifndef __Collision_h_
#define __Collision_h_

#include <vector>

#include <OgrePrerequisites.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreAxisAlignedBox.h>

class Mesh;

typedef enum
{
    ROAD,
    ROADSIDE,
    OFFROAD
} CollisionType;

class Collision
{
public:
    Collision();
    ~Collision();

    CollisionType collides(const Ogre::Vector3 &vector) const;
    void generateCollisionMatrix(const Ogre::Mesh * const mesh, const Ogre::Real resolution = 1);

protected:
    bool isOutOfBounds(const Ogre::int32 x, const Ogre::int32 y) const;
    CollisionType lookIntoCollisionMatrix(const Ogre::Real x, const Ogre::Real z) const;
    void initCollisionMatrix(const Ogre::Vector3 * const vertices, const Ogre::uint8 * const tex, const size_t index_count, const unsigned long * const indices);
    void getMeshInformation(const Ogre::Mesh* const mesh,
                        size_t &vertex_count, Ogre::Vector3* &vertices, Ogre::uint8* &tex,
                        size_t &index_count, unsigned long* &indices,
                        const Ogre::Vector3 &position = Ogre::Vector3::ZERO,
                        const Ogre::Quaternion &orient = Ogre::Quaternion::IDENTITY,
                        const Ogre::Vector3 &scale = Ogre::Vector3::UNIT_SCALE);
    void destroyCollisionMatrix();

protected:
    Ogre::Real mResolution;
    Ogre::AxisAlignedBox mAABB; /**< minimum is world offset, maximum-minimum is bounding box */
    Ogre::uint16 mMatrixWidth, mMatrixHeight; /**< width = x, height = z */
    bool **mCollisionMatrix;
    bool mIsMatrixGenerated;

    //std::vector<Car *>
};


#endif // __Collision_h_

