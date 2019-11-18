#ifdef _WIN32
#include <windows.h> // include windows.h to avoid thousands of compile errors even though this class is not depending on Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <iostream>
#include <iomanip>
#include <cmath>
#include "cylinder.h"

/*
Interleaved vertices areis the combined set of vertex data
Similar to the hand-made vertices array used for the cube, with position followed by normal then texture coords
When setting up properties, OpenGL needs them in one big array with a specified stride length to know where the data is
The interleaved vector is constructed to fulfill this
*/

void Cylinder::draw() const
{
    glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, indices.data());
}

void Cylinder::drawLines(const float lineColor[4]) const
{
    // set line colour
    glColor4fv(lineColor);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, lineColor);

    // draw lines with VA
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices.data());

    glDrawElements(GL_LINES, (unsigned int)lineIndices.size(), GL_UNSIGNED_INT, lineIndices.data());

    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

void Cylinder::clearArrays() {
    std::vector<glm::vec3>().swap(vertices);
    std::vector<glm::vec3>().swap(normals);
    std::vector<glm::vec2>().swap(texCoords);
    std::vector<unsigned int>().swap(indices);
    std::vector<unsigned int>().swap(lineIndices);
}

void Cylinder::addVertex(float x, float y, float z) {
    vertices.push_back(glm::vec3(x, y, z));
}

void Cylinder::addNormal(float x, float y, float z) {
    normals.push_back(glm::vec3(x, y, z));
}

void Cylinder::addTexCoord(float x, float y) {
    texCoords.push_back(glm::vec2(x, y));
}

void Cylinder::addIndices(unsigned int i1, unsigned int i2, unsigned int i3) {
    indices.push_back(i1);
    indices.push_back(i2);
    indices.push_back(i3);
}

/*void Cylinder::buildVertices()
{
    const float PI = 3.1415926f;

    // clear memory of prev arrays
    clearArrays();

    float x, y, z;                           // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius; // normal
    float s, t;                                  // texCoord

    float edgeStep = 2*PI/edgeCount;

    float yOffset = length;
    
    //2 Circular bases
    for(int a = 0; a < 2; a++) {
        for (int i = 0; i <= edgeCount; i++)
        {
            x = cos(edgeStep*i);
            z = sin(edgeStep*i);
            addVertex(x, yOffset, z);
            addNormal(0.0f, yOffset > 0 ? 1.0f : -1.0f, 0.0f);
            addTexCoord(x, z);
        }
        yOffset = -length;
    }

    //Rectangular side panels
    //Two triangles per panel
    unsigned int k1, k2;
    for (int i = 0; i < edgeCount; ++i)
    {
        //First point on top circle
        vertices.push_back(vertices[i]);
        normals.push_back(glm::normalize(glm::vec3(vertices[i].x, 0.0f, vertices[i].z)));
        texCoords.push_back(texCoords[i]);

        //Second point on top circle
        int safeI = (i+1)%edgeCount;
        vertices.push_back(vertices[safeI]);
        normals.push_back(glm::normalize(glm::vec3(vertices[safeI].x, 0.0f, vertices[safeI].z)));
        texCoords.push_back(texCoords[safeI]);

        //First point on bottom circle twice
        int bi = i + edgeCount;
        for(int p = 0; p < 2; p++) {
            vertices.push_back(vertices[bi]);
            normals.push_back(glm::normalize(glm::vec3(vertices[bi].x, 0.0f, vertices[bi].z)));
            texCoords.push_back(texCoords[bi]);
        }

        //Second point on bottom circle
        int safeBI = (bi + 1) % (edgeCount*2);
        vertices.push_back(vertices[safeBI]);
        normals.push_back(glm::normalize(glm::vec3(vertices[safeBI].x, 0.0f, vertices[safeBI].z)));
        texCoords.push_back(texCoords[safeBI]);

        //Second point on top circle again
        vertices.push_back(vertices[safeI]);
        normals.push_back(glm::normalize(glm::vec3(vertices[safeI].x, 0.0f, vertices[safeI].z)));
        texCoords.push_back(texCoords[safeI]);
    }

    // generate interleaved vertex array as well
    buildInterleavedVertices();
}*/

void Cylinder::buildVertices()
{
    const float PI = 3.1415926f;

    // clear memory of prev arrays
    clearArrays();

    float x, y, z;                           // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius; // normal
    float s, t;                                  // texCoord

    float edgeStep = 2*PI/edgeCount;

    float yOffset = length;
    glm::vec3 centerPoint(0.0f, yOffset, 0.0f);
    glm::vec3 centerPointNormal(0.0f, 1.0f, 0.0f);
    glm::vec2 centerPointTex(0.0f, 0.0f);
    texCoords.push_back(centerPointTex);
    vertices.push_back(centerPoint);
    normals.push_back(centerPointNormal);
    
    //2 Circular bases
    for(int a = 0; a < 2; a++) {
        for (int i = 0; i < edgeCount; i++)
        {
            z = cos(edgeStep*i)*radius;
            x = sin(edgeStep*i)*radius;
            addVertex(x, yOffset, z);
            addNormal(0.0f, yOffset > 0 ? 1.0f : -1.0f, 0.0f);
            addTexCoord(x, z);
            if(i > 0) {
                if(a == 0) {
                    addIndices(0, i, i+1);
                }
                else {
                    addIndices(edgeCount+1, edgeCount+i+1, edgeCount+2+i);
                }
            }
        }

        if(a == 0) {
            addIndices(0, 1, edgeCount);
            yOffset = -length;
            centerPoint.y = yOffset;
            centerPointNormal.y = -1.0f;
            vertices.push_back(centerPoint);
            normals.push_back(centerPointNormal);
            texCoords.push_back(centerPointTex);
        }
        else {
            addIndices(edgeCount+1, edgeCount+2, edgeCount*2+1);
        }
    }

    //Rectangular side panels
    //Two triangles per panel
    for (int i = 0; i < edgeCount; ++i)
    {
        addIndices(i+1, (i+2), i+edgeCount+2);
        addIndices(i+edgeCount+2, (i+edgeCount+3), (i+2));
    }

    addIndices(1, edgeCount, edgeCount+2);
    addIndices(edgeCount+2, vertices.size()-1, edgeCount);

    //generate interleaved vertex array
    buildInterleavedVertices();
}

void Cylinder::buildInterleavedVertices()
{
    std::vector<float>().swap(interleavedVertices);

    std::size_t i;
    std::size_t count = vertices.size();
    for (i = 0; i < count; i ++)
    {
        interleavedVertices.push_back(vertices[i].x);
        interleavedVertices.push_back(vertices[i].y);
        interleavedVertices.push_back(vertices[i].z);

        interleavedVertices.push_back(normals[i].x);
        interleavedVertices.push_back(normals[i].y);
        interleavedVertices.push_back(normals[i].z);

        interleavedVertices.push_back(texCoords[i].x);
        interleavedVertices.push_back(texCoords[i].y);
    }
}

Cylinder::Cylinder(float radius, float length, int edgeCount) : interleavedStride(32)
{
    this->radius = radius;
    this->length = length;
    this->edgeCount = edgeCount;

    buildVertices();
}