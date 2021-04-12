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
Interleaved vertices are the combined set of vertex data
Similar to the hand-made vertices array used for the cube, with position followed by normal then texture coords
When setting up properties, OpenGL needs them in one big array with a specified stride length to know where the data is
The interleaved vector is constructed to fulfill this
*/

/**
 * Render the cylinder to the screen 
 */
void Cylinder::draw() const {
    glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, indices.data());
}

/**
 * Render the cylinder as a wireframe
 * 
 * @param lineColor the color of the lines
 */
void Cylinder::drawLines(const float lineColor[4]) const {
    // set line colour
    glColor4fv(lineColor);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, lineColor);

    // draw lines with VA
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, this->vertices.data());

    glDrawElements(GL_LINES, (unsigned int)this->lineIndices.size(), GL_UNSIGNED_INT, this->lineIndices.data());

    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

/**
 * Wipe geometry data
 */
void Cylinder::clearArrays() {
    std::vector<glm::vec3>().swap(this->vertices);
    std::vector<glm::vec3>().swap(this->normals);
    std::vector<glm::vec2>().swap(this->texCoords);
    std::vector<unsigned int>().swap(this->indices);
    std::vector<unsigned int>().swap(this->lineIndices);
}

/**
 * Add a vertex
 * 
 * @param x the x coord
 * @param y the y coord
 * @param z the z coord
 */
void Cylinder::addVertex(float x, float y, float z) {
    this->vertices.push_back(glm::vec3(x, y, z));
}

/**
 * Add a normal vector
 * 
 * @param x the x coord
 * @param y the y coord
 * @param z the z coord
 */
void Cylinder::addNormal(float x, float y, float z) {
    this->normals.push_back(glm::vec3(x, y, z));
}

/**
 * Add a texture coordinate
 * 
 * @param x the x coord
 * @param y the y coord
 */
void Cylinder::addTexCoord(float x, float y) {
    this->texCoords.push_back(glm::vec2(x, y));
}

/**
 * Add a set of indices describing a triangle
 * 
 * @param i1 the first index
 * @param i2 the second index
 * @param i3 the third index
 */
void Cylinder::addIndices(unsigned int i1, unsigned int i2, unsigned int i3) {
    this->indices.push_back(i1);
    this->indices.push_back(i2);
    this->indices.push_back(i3);
}

/**
 * Generate vertex/normal/element/texture data for the cylinder
 */
void Cylinder::buildVertices() {
    const float PI = 3.1415926f;

    // clear memory of prev arrays
    clearArrays();

    float x, y, z;    // vertex position
    float nx, ny, nz; // normal
    float s, t;       // texCoord

    float edgeStep = 2*PI/edgeCount;

    float yOffset = length;
    glm::vec3 centerPoint(0.0f, yOffset, 0.0f);
    glm::vec3 centerPointNormal(0.0f, 1.0f, 0.0f);
    glm::vec2 centerPointTex(0.0f, 0.0f);
    this->texCoords.push_back(centerPointTex);
    this->vertices.push_back(centerPoint);
    this->normals.push_back(centerPointNormal);

    //2 Circular bases
    for(int a = 0; a < 2; a++) {
        for (int i = 0; i < edgeCount; i++)
        {
            z = cos(edgeStep*i)*radius;
            x = sin(edgeStep*i)*radius;
            this->addVertex(x, yOffset, z);
            this->addNormal(0.0f, yOffset > 0 ? 1.0f : -1.0f, 0.0f);
            this->addTexCoord(x, z);
            if(i > 0) {
                if(a == 0) {
                    this->addIndices(0, i, i + 1);
                }
                else {
                    this->addIndices(edgeCount + 1, edgeCount + i + 1, edgeCount + 2 + i);
                }
            }
        }

        if(a == 0) {
            this->addIndices(0, 1, edgeCount);
            yOffset = 0.0f;
            centerPoint.y = yOffset;
            centerPointNormal.y = -1.0f;
            this->vertices.push_back(centerPoint);
            this->normals.push_back(centerPointNormal);
            this->texCoords.push_back(centerPointTex);
        }
        else {
            this->addIndices(edgeCount + 1, edgeCount + 2, edgeCount * 2 + 1);
        }
    }

    int baseOffset = edgeCount*2+2;

    int max = vertices.size();
    for(int i = 1; i < max; i++) {
        if(i == edgeCount+1) {continue;}
        this->vertices.push_back(this->vertices[i]);
        this->texCoords.push_back(this->texCoords[i]);
        this->normals.push_back(glm::normalize(glm::vec3(this->vertices[i].x, 0.0f, this->vertices[i].z)));
    }

    //Rectangular side panels
    //Two triangles per panel
    for (int i = 0; i < edgeCount; i++)
    {
        int newI = i+baseOffset;
        this->addIndices(newI, newI + 1, newI + edgeCount);
        this->addIndices(newI + edgeCount, newI + edgeCount + 1, newI + 1);
    }

    this->addIndices(baseOffset, baseOffset + edgeCount - 1, baseOffset + edgeCount);
    this->addIndices(baseOffset + edgeCount, vertices.size() - 1, baseOffset + edgeCount - 1);

    //generate interleaved vertex array
    this->buildInterleavedVertices();
}

/**
 * Interleave vertex, normal, and texture data
 * into one array so OpenGL can render it properly
 */
void Cylinder::buildInterleavedVertices() {
    std::vector<float>().swap(this->interleavedVertices);

    std::size_t i;
    std::size_t count = this->vertices.size();
    for (i = 0; i < count; i ++)
    {
        this->interleavedVertices.push_back(this->vertices[i].x);
        this->interleavedVertices.push_back(this->vertices[i].y);
        this->interleavedVertices.push_back(this->vertices[i].z);

        this->interleavedVertices.push_back(this->normals[i].x);
        this->interleavedVertices.push_back(this->normals[i].y);
        this->interleavedVertices.push_back(this->normals[i].z);

        this->interleavedVertices.push_back(this->texCoords[i].x);
        this->interleavedVertices.push_back(this->texCoords[i].y);
    }
}

/**
 * @brief Construct a new Cylinder object
 * 
 * @param radius the radius of the base
 * @param length the length
 * @param edgeCount the number of edges (more = smoother but slower)
 */
Cylinder::Cylinder(float radius, float length, int edgeCount) : interleavedStride(32) {
    this->radius = radius;
    this->length = length;
    this->edgeCount = edgeCount;

    buildVertices();
}