#include "VElevationTile.h"
#include <stdlib.h>
#include "gdal_priv.h"

const double PI = 3.1415926535898;
const int pixels = 256;

VElevationTile::VElevationTile() {
}


void VElevationTile::init(const char* path, int level) {
    // Try load tif and twf
    tifFile = path;
    tifFile.append(".tif");
    tfwFile = path;
    tfwFile.append(".tfw");
    std::ifstream meta;
    meta.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    double ignore;
    rowCount = 0;
    colCount = 0;
    this->level = level;
    this->isLoadedData = false;

    try {
        //meta.open(tifFile.c_str());
        //meta >> stepX >> ignore >> ignore >> stepY >> baseX >> baseY;
        //meta.close();
    }
    catch (std::ifstream::failure e) {
        std::cout << "Open meta file failed: " << tfwFile << std::endl;
    }
}

int  VElevationTile::getVerticesArraySize() const {
    return (this->rowCount + 1) * (this->colCount + 1) * 5;
}

int  VElevationTile::getIndicesArraySize() const {
    return (this->rowCount) * (this->colCount) * 2 * 3;
}

double VElevationTile::getMeterPerPixel(int nLevel) {

    // Calculate its level, width height in real meters in correspond level.
    // For example, level 15 each pixel will have 4.7 meters in real world.
    if (nLevel < 0 || nLevel > 23) {
        std::cout << "Invalid level value (less than zero or greater than 23: " << nLevel << std::endl;
        return -1.0;
    }

    return 0.018661384 * pow(2.0, double(23 - nLevel));
}

void VElevationTile::calculateBaseByIndex(int rowTile, int columnTile) {
    //This utility will can convert row/column tiles into lon, lat.
    double lon_deg = (double)rowTile / pow(2.0, (double)this->level) * 360.0 - 180.0;
    double lat_rad = atan(sinh(PI * (1.0 - 2.0 * (double)columnTile / pow(2.0, (double)this->level))));
    double lat_deg = lat_rad * 180.0 / PI;
    baseX = lon_deg;
    baseY = lat_deg;
}

float VElevationTile::getElevation(int nRow, int nCol) {

    // -1 means a negative value
    if (nRow * nCol >= elevations.size()) {
        std::cout << " nRow " << nRow << " and nCol " << nCol << " are out of bound" << std::endl;
        return -1.0;
    }

    return this->elevations[nRow * colCount + nCol];
}

int  VElevationTile::getRowCount() const {
    return rowCount;
}

int  VElevationTile::getColCount() const {
    return colCount;
}

void VElevationTile::readElevationsByFloat() {
    GDALDataset* poDataset;

    GDALAllRegister();

    poDataset = (GDALDataset*)GDALOpen(tifFile.c_str(), GA_ReadOnly);
    if (poDataset == NULL) {
        std::cout << "Open tif for creationg data set failed!" << tifFile << std::endl;;
    }

    elevations.clear();
    float* pafScanblock1 = NULL;
    pafScanblock1 = (float*)CPLMalloc(sizeof(float) * (1) * (1));

    if (poDataset != NULL) {
        // Use GDAL to extract elevations, always in band 1.
        GDALRasterBand* band = poDataset->GetRasterBand(1);

        rowCount = poDataset->GetRasterXSize();
        colCount = poDataset->GetRasterYSize();

        double trans[6];
        CPLErr err = poDataset->GetGeoTransform(trans);

        for (int nRow = 0; nRow < rowCount; nRow++) {
            for (int nCol = 0; nCol < colCount; nCol++) {
                const GDALDataType rasterDataType = GDALDataType(band->GetRasterDataType());
                band->RasterIO(GF_Read, nRow, nCol, 1, 1, pafScanblock1, 1, 1, rasterDataType, 0, 0);

                float elevation = *pafScanblock1;
                elevations.push_back(elevation);
            }
        }
    }

    this->isLoadedData = true;
    CPLFree(pafScanblock1);
    GDALClose(poDataset);
}

void VElevationTile::getVertices(float* coords, unsigned int arrayOffset, int simplifyStep) {
    if (simplifyStep < 1) {
        std::cout << " simplify step must greater than 1 " << std::endl;
    }
    const double pixelPerMeter = VElevationTile::getMeterPerPixel(this->level);

    // Generate an 2 d array vertices, map each elevations in Z
    for (int row = 0; row < rowCount + simplifyStep; row += simplifyStep) {
        for (int col = 0; col < colCount + simplifyStep; col += simplifyStep) {
            const int index = (row * (colCount + 1) / simplifyStep + col) * 5 + arrayOffset;
            coords[index] = row * pixelPerMeter * simplifyStep * scaleX + offsetX;
            coords[index + 1] = col * pixelPerMeter * simplifyStep * scaleY + offsetY;
            coords[index + 3] = (double)row / (double)rowCount;
            coords[index + 4] = 1 - (double)col / (double)colCount;

            int nRow = row;
            int nCol = col;
            if (row == rowCount) nRow = row - 1;
            if (col == colCount) nCol = col - 1;
            coords[index + 2] = getElevation(nRow, nCol) * scaleZ + offsetZ;
        }
    }
}

void VElevationTile::getIndices(unsigned int* indices, unsigned int arrayOffset, int simplifyStep) {
    if (simplifyStep < 1) {
        std::cout << " simplify step must greater than 1 " << std::endl;
    }

    int index = arrayOffset;
    // Mapping 2 triangles into a 2d array mesh.

    for (int row = 0; row < rowCount; row += simplifyStep) {
        int row1 = (row) * (colCount + 1) / simplifyStep;
        int row2 = (row + 1) * (colCount + 1) / simplifyStep;

        for (int col = 0; col < colCount; col += simplifyStep) {
            indices[index++] = row1 + col / simplifyStep;
            indices[index++] = row1 + col / simplifyStep + 1;
            indices[index++] = row2 + col / simplifyStep;

            indices[index++] = row1 + col / simplifyStep + 1;
            indices[index++] = row2 + col / simplifyStep;
            indices[index++] = row2 + col / simplifyStep + 1;
        }
    }
}

void VElevationTile::setMeshTranslate(float sX, float sY, float sZ, float oX, float oY, float oZ) {
    scaleX = sX;
    scaleY = sY;
    scaleZ = sZ;
    offsetX = oX;
    offsetY = oY;
    offsetZ = oZ;
}

bool VElevationTile::isValid() const {
    return this->isLoadedData;
}