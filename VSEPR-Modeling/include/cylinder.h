#pragma once

#include <vector>
#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtc/type_ptr.hpp"

class Cylinder {
    public:
    Cylinder(float radius = 1.0f, float length = 5.0f, int edgeCount = 30);
    void draw() const;
    void drawLines(const float lineColor[4]) const;

    unsigned int getInterleavedVertexSize() const { return (unsigned int)interleavedVertices.size() * sizeof(float); } // # of bytes
    int getInterleavedStride() const { return interleavedStride; }                                                     // should be 32 bytes
    const float *getInterleavedVertices() const { return interleavedVertices.data(); }

private:
    void buildVertices();
    void buildSquareVertices();
    void buildInterleavedVertices();
    void clearArrays();

    void addVertex(float x, float y, float z);
    void addNormal(float x, float y, float z);
    void addTexCoord(float x, float y);
    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);

    float radius;
    float length;
    int edgeCount;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> lineIndices;
    
    std::vector<float> interleavedVertices;
    float interleavedStride;
};