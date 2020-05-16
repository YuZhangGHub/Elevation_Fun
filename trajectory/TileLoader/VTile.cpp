#include "VTile.h"
#include "VTileLoader.h" //For file path constants
#include "stb_image.h"

using namespace std;

VTile::VTile() {}

VTile::~VTile() {}

VTile::VTile(Vector2i tileIndices, Vector2f position, Vector2f coordinates, float width, int level):
	m_tileIndices(tileIndices), m_position(position), m_coordinates(coordinates), m_tileWidth(width), m_level(level) {

	if (tileIndices.Y == 26907 && tileIndices.X == 5564) {
		int kk = 0;
	}

	if (!VTileLoader::s_TilesFilePath.empty()) {
		m_imageFile = VTileLoader::s_TilesFilePath + "\\" + to_string(level) + "\\" + to_string(tileIndices.Y) + "\\" + to_string(tileIndices.X) + ".png";
		loadImage(); 
	}

	if (!VTileLoader::s_ElevationFilePath.empty()) {
		m_elevationFile = VTileLoader::s_TilesFilePath + "\\" + to_string(level) + "\\" + to_string(tileIndices.Y) + "\\" + to_string(tileIndices.X) + ".tfw";
		loadElevation();
	}

	if (!VTileLoader::s_GltfFilePath.empty()) {
		m_gltfFile = VTileLoader::s_TilesFilePath + "\\" + to_string(level) + "\\" + to_string(tileIndices.Y) + "\\" + to_string(tileIndices.X) + ".gltf";
		loadGltf();
	}
}

shared_ptr<char[]> VTile::getImage() {
	return m_pImage;
}

shared_ptr<char[]> VTile::getElevation() {
	return m_pElevation;
}

shared_ptr<char[]> VTile::getGltf() {
	return m_pGltf;
}

Vector2f VTile::getPosition() {
	return m_position; 
}

Vector2f VTile::getCoordinates() {
	return m_coordinates; 
}

Vector2i VTile::getIndices() {
	return m_tileIndices; 
}

string VTile::getId() {
	return m_position.id();
}

float VTile::width() {
	return m_tileWidth;
}

int VTile::level() {
	return m_level;
}

unsigned VTile::getImageSize() {
	return m_imageSize;
}

unsigned VTile::getElevationSize() { 
	return m_elevationSize;
}

unsigned VTile::getGltfSize() {
	return m_gltfSize;
}

#ifndef _WIN64
sf::Texture VTile::getTexture() { // TODO: remove after demo is done
	if (tileTexture.getSize().x < 1 && m_pImage!=nullptr) {
		sf::Image tileImage;
		tileImage.loadFromMemory(m_pImage.get(), m_imageSize);
		tileTexture.loadFromImage(tileImage);
	}

	return tileTexture;
}
#endif

unsigned char* VTile::getTexture(int& width, int& height) {
	int nrChannels = 0;
	return stbi_load(m_imageFile.c_str(), &width, &height, &nrChannels, 0);
}

void VTile::loadImage()
{
	loadFile(m_imageFile, m_pImage, m_imageSize);
}

void VTile::loadElevation()
{
	// Read elevation values by gdal£¬read directly by tif.
	std::string elevationFileName = m_elevationFile.c_str();
	size_t lastDot = elevationFileName.find_last_of('.');
	elevationFileName = elevationFileName.substr(0, lastDot);

	elevationTile.init(elevationFileName.c_str(), m_level);
	elevationTile.readElevationsByFloat();
}

void VTile::loadGltf()
{
	loadFile(m_gltfFile, m_pGltf, m_gltfSize);
}

void VTile::loadFile(string& filename, shared_ptr<char[]>& store, unsigned& size)
{
	ifstream file(filename, ios::binary | ios::ate);
	size = (unsigned) file.tellg();

	if (size > 0) {
		file.seekg(0, ios::beg);

		store = shared_ptr<char[]>(new char[size],
			[](char* ptr) { delete[]ptr; });
		//make_unique<char[]>(size);

		if (file.read(store.get(), size)) {

			//printf("File read: %s", m_imageFile.c_str());
		}
		else {
			printf("File cannot be read: %s", filename.c_str());
		}
	}
	else {
		printf("File cannot be found: %s", filename.c_str());
	}

	file.close();
}

bool VTile::getTileVertices(float* vertices, int& verticesLen, int offsetStart, int simplifyStep) {
	if (elevationTile.isValid()) {
		elevationTile.getVertices(vertices, offsetStart, simplifyStep);
		verticesLen = elevationTile.getVerticesArraySize();
		return true;
	}

	return false;
}

bool VTile::getTileIndices(unsigned int* indices, int& indicesLen, int offsetStart, int simplifyStep) {
	if (elevationTile.isValid()) {
		elevationTile.getIndices(indices, offsetStart, simplifyStep);
		indicesLen = elevationTile.getIndicesArraySize();
		return true;
	}

	return false;
}

void VTile::setMeshTranslate(const Vector3f& scale, const Vector3f& offset) {
	if (elevationTile.isValid()) {
		elevationTile.setMeshTranslate(scale.X, scale.Y, scale.Z, offset.X, offset.Y, offset.Z);
	}
}