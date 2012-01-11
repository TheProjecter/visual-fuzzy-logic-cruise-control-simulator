#include "Collision.h"
#include <OgreMesh.h>
#include <OgreSubMesh.h>
#include <OgreLogManager.h>
#include <OgreStringConverter.h>

#include <cfloat>

double round(double r) {
    return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

Collision::Collision()
:   mResolution(0),
    mAABB(Ogre::AxisAlignedBox::BOX_NULL),
    mMatrixWidth(0), mMatrixHeight(0),
    mCollisionMatrix(NULL), mIsMatrixGenerated(false)
{

}

Collision::~Collision()
{
    destroyCollisionMatrix();
}

CollisionType Collision::collides(const Ogre::Vector3 &pos) const
{
    const Ogre::Vector3 minVec = mAABB.getMinimum();
    const Ogre::Vector3 maxVec = mAABB.getMaximum();

    // check if 'vector' is withing the bounding rectangle
    if (pos.x > minVec.x && pos.x < maxVec.x &&
        pos.z > minVec.z && pos.z < maxVec.z)
    {
        return lookIntoCollisionMatrix(pos.x, pos.z);
    }
    else
    {
        //OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "vector is OOB!", "Collision::collides()");

		// NOTE: this will make the car explode immediately
        //return OFFROAD;

		return ROADSIDE;
    }
}

bool Collision::isOutOfBounds(const Ogre::int32 x, const Ogre::int32 y) const
{
    return !(x >= 0 && x < mMatrixWidth && y >= 0 && y < mMatrixHeight);
}

CollisionType Collision::lookIntoCollisionMatrix(const Ogre::Real x, const Ogre::Real z) const
{
    const Ogre::int32 xIndex = ((x - mAABB.getMinimum().x) / mResolution);
    const Ogre::int32 zIndex = ((z - mAABB.getMinimum().z) / mResolution);
//    Ogre::LogManager::getSingletonPtr()->logMessage("x = " + Ogre::StringConverter::toString(xIndex));
//    Ogre::LogManager::getSingletonPtr()->logMessage("z = " + Ogre::StringConverter::toString(zIndex));
//    Ogre::LogManager::getSingletonPtr()->logMessage("cz = " + Ogre::StringConverter::toString(Ogre::Real(z)));
//    Ogre::LogManager::getSingletonPtr()->logMessage("cx = " + Ogre::StringConverter::toString(Ogre::Math::Floor((100.1/10.0)+0.5)));
//    Ogre::LogManager::getSingletonPtr()->logMessage("cz = " + Ogre::StringConverter::toString(Ogre::Math::Floor((0.0/10.0)+0.5)));
//    Ogre::LogManager::getSingletonPtr()->logMessage("cy = " + Ogre::StringConverter::toString(Ogre::Math::Floor((0.1/10.0)+0.5)));
    if (isOutOfBounds(xIndex, zIndex))
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "x or/and z is out bounds!", "Collision::lookIntoCollisionMatrix()");
    }

    return (mCollisionMatrix[zIndex][xIndex] ? ROAD : ROADSIDE);
}

void Collision::generateCollisionMatrix(const Ogre::Mesh * const mesh, const Ogre::Real resolution)
{
    if (mesh == NULL)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "mesh is NULL!", "Collision::generateCollisionMatrix()");
    }
    if (resolution <= 0)
    {
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "resolution is not a positive real!", "Collision::generateCollisionMatrix()");
    }

    if (mIsMatrixGenerated)
    {
        destroyCollisionMatrix();
    }
    OgreAssert(mIsMatrixGenerated == false && mCollisionMatrix == NULL, "C. matrix not generated!");

    // get mesh vertex data
    size_t vertex_count;
    Ogre::Vector3 *vertices;
    Ogre::uint8* tex;
    size_t index_count;
    unsigned long *indices;
//    const Ogre::Vector3 position = Ogre::Vector3::ZERO;
//    const Ogre::Quaternion orient = Ogre::Quaternion::IDENTITY;
//    const Ogre::Vector3 scale = Ogre::Vector3::UNIT_SCALE;

    OgreAssert(mesh != NULL, "mesh null");
    getMeshInformation(mesh, vertex_count, vertices, tex, index_count, indices);
    OgreAssert(vertices != NULL && indices != NULL, "vertices or/and indices null");

    for (int i =0; i < vertex_count ; ++i )
    {
        Ogre::LogManager::getSingleton().logMessage("vertex: " + Ogre::StringConverter::toString(vertices[i]));
    }

    // calculate AABB
    // NOTE: Mesh.getBounds() is not very precise!
    Ogre::Real  minX = FLT_MAX, maxX = FLT_MIN,
                minY = FLT_MAX, maxY = FLT_MIN,
                minZ = FLT_MAX, maxZ = FLT_MIN;
    for (size_t i = 0; i < vertex_count; ++i)
    {
        if (vertices[i].x < minX)
        {
            minX = (vertices[i].x);
        }
        if (vertices[i].x > maxX)
        {
            maxX = (vertices[i].x);
        }

        if (vertices[i].y < minY)
        {
            minY = (vertices[i].y);
        }
        if (vertices[i].y > maxY)
        {
            maxY = (vertices[i].y);
        }

        if (vertices[i].z < minZ)
        {
            minZ = (vertices[i].z);
        }
        if (vertices[i].z > maxZ)
        {
            maxZ = (vertices[i].z);
        }
    }

    mAABB = Ogre::AxisAlignedBox(minX, minY, minZ, maxX, maxY, maxZ);

    OgreAssert(minX < FLT_MAX && maxX > FLT_MIN &&
               minY < FLT_MAX && maxY > FLT_MIN &&
               minZ < FLT_MAX && maxZ > FLT_MIN,
               "AABB was not properly set! Limits are FLT_MIN and FLT_MAX");
//    Ogre::LogManager::getSingletonPtr()->logMessage("min: " + Ogre::StringConverter::toString(mesh->getBounds().getMinimum()));
//    Ogre::LogManager::getSingletonPtr()->logMessage("max: " + Ogre::StringConverter::toString(mesh->getBounds().getMaximum()));
    Ogre::LogManager::getSingletonPtr()->logMessage("min: " + Ogre::StringConverter::toString(mAABB.getMinimum()));
    Ogre::LogManager::getSingletonPtr()->logMessage("max: " + Ogre::StringConverter::toString(mAABB.getMaximum()));

    // calculate the width and height and thus the number of cells
    mResolution = resolution;
    mMatrixWidth = Ogre::Math::Ceil((maxX - minX) / mResolution);
    mMatrixHeight = Ogre::Math::Ceil((maxZ - minZ) / mResolution);
    Ogre::LogManager::getSingletonPtr()->logMessage("w: " + Ogre::StringConverter::toString(mMatrixWidth));
    Ogre::LogManager::getSingletonPtr()->logMessage("h: " + Ogre::StringConverter::toString(mMatrixHeight));

    initCollisionMatrix(vertices, tex, index_count, indices);

    delete[] vertices;
    delete[] indices;
    delete[] tex;

    mIsMatrixGenerated = true;
}

void Collision::initCollisionMatrix(const Ogre::Vector3 * const vertices, const Ogre::uint8 * const tex, const size_t index_count, const unsigned long * const indices)
{
    // allocate matrix
    mCollisionMatrix = new bool*[mMatrixHeight];
    for (Ogre::int32 i = 0; i < mMatrixHeight; ++i)
    {
        mCollisionMatrix[i] = new bool[mMatrixWidth];
        for (Ogre::int32 j = 0; j < mMatrixWidth; ++j)
        {
            mCollisionMatrix[i][j] = false;
        }
    }

    // rasterize triangles (defined in vertices & indices)
    const Ogre::Vector3 offset = mAABB.getMinimum();
    for (Ogre::int32 iIndex = 0; iIndex < index_count; iIndex += 3)
    {
        Ogre::Vector3 v1 = (vertices[indices[iIndex+0]] - offset) / mResolution;
        Ogre::Vector3 v2 = (vertices[indices[iIndex+1]] - offset) / mResolution;
        Ogre::Vector3 v3 = (vertices[indices[iIndex+2]] - offset) / mResolution;

//        OgreAssert( tex[indices[iIndex+0]] == tex[indices[iIndex+3]] && tex[indices[iIndex+3]] == tex[indices[iIndex+2]], "nope" );
        const bool value = tex[indices[iIndex+1]];

        // 28.4 fixed-point coordinates
		const int Y1 = round(16.0f * v1.z);
        const int Y2 = round(16.0f * v2.z);
        const int Y3 = round(16.0f * v3.z);

        const int X1 = round(16.0f * v1.x);
        const int X2 = round(16.0f * v2.x);
        const int X3 = round(16.0f * v3.x);

        // Deltas
        const int DX12 = X1 - X2;
        const int DX23 = X2 - X3;
        const int DX31 = X3 - X1;

        const int DY12 = Y1 - Y2;
        const int DY23 = Y2 - Y3;
        const int DY31 = Y3 - Y1;

        // Fixed-point deltas
        const int FDX12 = DX12 << 4;
        const int FDX23 = DX23 << 4;
        const int FDX31 = DX31 << 4;

        const int FDY12 = DY12 << 4;
        const int FDY23 = DY23 << 4;
        const int FDY31 = DY31 << 4;

        // Bounding rectangle
        int minx = (std::min(X1, std::min(X2, X3)) + 0xF) >> 4;
        int maxx = (std::max(X1, std::max(X2, X3)) + 0xF) >> 4;
        int miny = (std::min(Y1, std::min(Y2, Y3)) + 0xF) >> 4;
        int maxy = (std::max(Y1, std::max(Y2, Y3)) + 0xF) >> 4;

        // Block size, standard 8x8 (must be power of two)
        // NOTE: if mesh is incorrectly rasterized, 'q' is probably too small
        const int q = 1 << 10;

        // Start in corner of 8x8 block
        minx &= ~(q - 1);
        miny &= ~(q - 1);

        // Half-edge constants
        int C1 = DY12 * X1 - DX12 * Y1;
        int C2 = DY23 * X2 - DX23 * Y2;
        int C3 = DY31 * X3 - DX31 * Y3;

        // Correct for fill convention
        if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
        if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
        if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

        // Loop through blocks
        for(int y = miny; y < maxy; y += q)
        {
            for(int x = minx; x < maxx; x += q)
            {
                // Corners of block
                int x0 = x << 4;
                int x1 = (x + q - 1) << 4;
                int y0 = y << 4;
                int y1 = (y + q - 1) << 4;

                // Evaluate half-space functions
                bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
                bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
                bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
                bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
                int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

                bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
                bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
                bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
                bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
                int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

                bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
                bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
                bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
                bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
                int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

                // Skip block when outside an edge
                if(a == 0x0 || b == 0x0 || c == 0x0) continue;

                // Accept whole block when totally covered
                if(a == 0xF && b == 0xF && c == 0xF)
                {
                    for(int iy = 0; iy < q; iy++)
                    {
                        for(int ix = x; ix < x + q; ix++)
                        {
                            mCollisionMatrix[iy][ix] = value;
                        }
                    }
                }
                else // Partially covered block
                {
                    int CY1 = C1 + DX12 * y0 - DY12 * x0;
                    int CY2 = C2 + DX23 * y0 - DY23 * x0;
                    int CY3 = C3 + DX31 * y0 - DY31 * x0;

                    for(int iy = y; iy < y + q; iy++)
                    {
                        int CX1 = CY1;
                        int CX2 = CY2;
                        int CX3 = CY3;

                        for(int ix = x; ix < x + q; ix++)
                        {
                            if(CX1 > 0 && CX2 > 0 && CX3 > 0)
                            {
                                mCollisionMatrix[iy][ix] = value;
                            }

                            CX1 -= FDY12;
                            CX2 -= FDY23;
                            CX3 -= FDY31;
                        }

                        CY1 += FDX12;
                        CY2 += FDX23;
                        CY3 += FDX31;
                    }
                }
            }
        }
    }

    // modify for roadside
    // find the first road bit
    int yStart = -1, xStart = -1;
    for (int y = 0; y < mMatrixHeight; ++y)
    {
        for (int x = 0; x < mMatrixWidth; ++x)
        {
            if (mCollisionMatrix[y][x])
            {
                yStart = y;
                xStart = x;
                break;
            }
        }

        if (yStart >= 0)
        {
            break;
        }
    }

    // follow the outer edge and turn it into roadside bits
//    Ogre::int32 y = yStart, x = xStart;
//    do
//    {
//        // find next edge bit
//        if (!isOutOfBounds(x+1,y) && mCollisionMatrix[y][x+1])
//        {
//            ++x;
//        }
//        else if (!isOutOfBounds(x+1,y+1) && mCollisionMatrix[y+1][x+1])
//        {
//            ++y;
//            ++x;
//        }
//        else if (!isOutOfBounds(x,y+1) && mCollisionMatrix[y+1][x])
//        {
//            ++y;
//        }
//        else if (!isOutOfBounds(x-1,y+1) && mCollisionMatrix[y+1][x-1])
//        {
//            ++y;
//            --x;
//        }
//        else if (!isOutOfBounds(x-1,y) && mCollisionMatrix[y][x-1])
//        {
//            --x;
//        }
//        else if (!isOutOfBounds(x-1,y-1) && mCollisionMatrix[y-1][x-1])
//        {
//            --y;
//            --x;
//        }
//        else if (!isOutOfBounds(x,y-1) && mCollisionMatrix[y-1][x])
//        {
//            --y;
//        }
//        else if (!isOutOfBounds(x+1,y-1) && mCollisionMatrix[y-1][x+1])
//        {
//            --y;
//            ++x;
//        }
//        else
//        {
//            Ogre::LogManager::getSingletonPtr()->logMessage("WARNING: " + Ogre::StringConverter::toString(y) + "," + Ogre::StringConverter::toString(x) );
//        }
//
//        // turn into roadside bit
//        mCollisionMatrix[y][x] = false;
//    } while (!(y == yStart && x == xStart));


    for (Ogre::int32 i = 0; i < mMatrixHeight; ++i)
    {
        std::string s = "";
        for (Ogre::int32 j = 0; j < mMatrixWidth; ++j)
        {
            s += (mCollisionMatrix[i][j] ? "1":"0");
        }
        Ogre::LogManager::getSingletonPtr()->logMessage(s);
    }
}

void Collision::getMeshInformation(const Ogre::Mesh* const mesh,
                        size_t &vertex_count, Ogre::Vector3* &vertices, Ogre::uint8* &tex,
                        size_t &index_count, unsigned long* &indices,
                        const Ogre::Vector3 &position,
                        const Ogre::Quaternion &orient,
                        const Ogre::Vector3 &scale)
{
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;

    vertex_count = index_count = 0;

    // Calculate how many vertices and indices we're going to need
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
            if( !added_shared )
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }
        // Add the indices
        index_count += submesh->indexData->indexCount;
    }

    // Allocate space for the vertices and indices
    vertices = new Ogre::Vector3[vertex_count];
    indices = new unsigned long[index_count];
    tex = new Ogre::uint8[vertex_count];

    added_shared = false;

    // Run through the submeshes again, adding the data into the arrays
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        Ogre::uint8 texValue = (submesh->getMaterialName() == "Road") ? 1 : 0;
        Ogre::LogManager::getSingletonPtr()->logMessage("tex: "+(submesh->getMaterialName()) );


        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }

            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //Ogre::Real* pReal;
            float* pReal;

            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);
                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
                vertices[current_offset + j] = (orient * (pt * scale)) + position;
                tex[current_offset + j] = texValue;
            }

            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }

        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        unsigned long* pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
            }
        }
        else
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = static_cast<unsigned long>(pShort[k]) +
                                          static_cast<unsigned long>(offset);
            }
        }

        ibuf->unlock();
        current_offset = next_offset;
    }
}


void Collision::destroyCollisionMatrix()
{
    if(mIsMatrixGenerated)
    {
        for (Ogre::int32 i = 0; i < mMatrixHeight; ++i)
        {
            delete[] mCollisionMatrix[i];
        }

        delete[] mCollisionMatrix;
        mCollisionMatrix = NULL;

        mIsMatrixGenerated = false;
    }
}
