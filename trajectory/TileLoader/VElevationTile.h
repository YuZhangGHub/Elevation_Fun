#pragma once

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

class VElevationTile
{
public:
    VElevationTile();

    // Open tif file
    void init(const char* path, int level);

    // Read/Get elevation value
    void  readElevationsByFloat();
    float getElevation(int nRow, int nCol);

    // Utils for calculate meters.
    void calculateBaseByIndex(int rowTile, int columnTile);
    static double getMeterPerPixel(int nLevel);

    // Get mesh elements, you can pass a big array then use arrayOffset to fill in data.
    // Simplify step support mesh reduction, if simplifyStep == 2, then 2 dem points a square.
    void getVertices(float* coords, unsigned int arrayOffset = 0, int simplifyStep = 1);
    void getIndices(unsigned int* indices, unsigned int arrayOffset = 0, int simplifyStep = 1);

    void computeNormals(float* vertices, int vertexCount, unsigned int* indices, int indexCount);

    // Mesh transform
    void setMeshTranslate(float sX, float sY, float sz, float oX, float oY, float oz);

    // Mesh elements' size
    int  getVerticesArraySize() const;
    int  getIndicesArraySize() const;

    // Dem pixels row and column count
    int  getRowCount() const;
    int  getColCount() const;

    bool isValid() const;

private:

    int    level;
    double baseX;
    double baseY;
    double stepX; 
    double stepY;
    std::string tifFile;
    std::string tfwFile;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleZ = 1.0f;
    float offsetX = 0;
    float offsetY = 0;
    float offsetZ = 0;

    std::vector<float> elevations;
    int rowCount;
    int colCount;
    bool isLoadedData;
};
