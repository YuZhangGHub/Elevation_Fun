#include "VTile.h"
#include "VTileLoader.h" //For file path constants
#include "stb_image.h"
#include "tiny_gltf.h"

using namespace std;

VTile::VTile() {}

VTile::~VTile() {}

VTile::VTile(Vector2i tileIndices, Vector2f position, Vector2f coordinates, float width, int level):
	m_tileIndices(tileIndices), m_position(position), m_coordinates(coordinates), m_tileWidth(width), m_level(level) {


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

unsigned char* VTile::getTexture(int& width, int& height, int& nChannel) {
	return stbi_load(m_imageFile.c_str(), &width, &height, &nChannel, 0);
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

	elevationTile.setMeshTranslate(1.0f, 1.0f, 1.0f, this->m_position.X, this->m_position.Y, 0);
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

int VTile::getTileVertexArraySize() const {
	if (elevationTile.isValid()) {
		return elevationTile.getVerticesArraySize();
	}

	return -1;
}


bool VTile::getTileVertices(float* vertices, int offsetStart, int simplifyStep) {
	if (elevationTile.isValid()) {
		elevationTile.getVertices(vertices, offsetStart, simplifyStep);
		return true;
	}

	return false;
}

int VTile::getTileIndexArraySize() const {
	if (elevationTile.isValid()) {
		return elevationTile.getIndicesArraySize();
	}

	return -1;
}

bool VTile::getTileIndices(unsigned int* indices, int offsetStart, int simplifyStep) {
	if (elevationTile.isValid()) {
		elevationTile.getIndices(indices, offsetStart, simplifyStep);
		return true;
	}

	return false;
}

bool VTile::computeNormals(float* vertices, int vertexCount, unsigned int* indices, int indexCount) {
	if (elevationTile.isValid()) {
		elevationTile.computeNormals(vertices, vertexCount, indices, indexCount);
		return true;
	}

	return false;
}

tinygltf::Model& VTile::buildGltfModel() {
	// Rebuild mesh into gltf model;
	gltfModel.accessors.clear();
	gltfModel.buffers.clear();
	gltfModel.bufferViews.clear();
	gltfModel.images.clear();
	gltfModel.materials.clear();
	gltfModel.meshes.clear();
	gltfModel.nodes.clear();
	gltfModel.scenes.clear();
	gltfModel.textures.clear();

    const int verticesLen = elevationTile.getVerticesArraySize();
    float* vertices = new float[verticesLen];
    int indexLen = elevationTile.getIndicesArraySize();
    unsigned int* indices = new unsigned int[indexLen];

    elevationTile.getVertices(vertices, 0);
    elevationTile.getIndices(indices, 0);
    elevationTile.computeNormals(vertices, verticesLen, indices, indexLen);

    //Fill in byte data into buffer_vertex
    tinygltf::Buffer buffer_vertex;
    for (int i = 0; i < verticesLen; i++) {
        float value = vertices[i];
        unsigned char* pdata = (unsigned char*)&value;

        for (int j = 0; j < 4; j++) {
            buffer_vertex.data.push_back(*pdata++);
        }
    }

    //Fill in byte data into buffer_indices
    tinygltf::Buffer buffer_indices;
    for (int i = 0; i < indexLen; i++) {
        unsigned int value = indices[i];
        unsigned char* pdata = (unsigned char*)&value;

        for (int j = 0; j < 4; j++) {
            buffer_indices.data.push_back(*pdata++);
        }
    }

    //Fill in image byte data into buffer_image
    int width = 0;
    int height = 0;
    int nChannel = 0;
    unsigned char* image_data = getTexture(width, height, nChannel);
    tinygltf::Buffer buffer_image;
    int image_size = height * width * nChannel;
    for (int i = 0; i < image_size; i++) {
        buffer_image.data.push_back(image_data[i]);
    }

    gltfModel.buffers.push_back(buffer_vertex);
    gltfModel.buffers.push_back(buffer_indices);
    gltfModel.buffers.push_back(buffer_image);

    //Buffer views
    tinygltf::BufferView bufferVerticesView;
    bufferVerticesView.buffer = 0;
    bufferVerticesView.byteLength = buffer_vertex.data.size();
    bufferVerticesView.byteStride = 32;
    bufferVerticesView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    bufferVerticesView.byteOffset = 0;

    tinygltf::BufferView bufferIndicesView;
    bufferIndicesView.buffer = 1;
    bufferIndicesView.byteLength = buffer_indices.data.size();
    bufferIndicesView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    bufferIndicesView.byteOffset = 0;

    tinygltf::BufferView bufferImageView;
    bufferImageView.buffer = 2;
    bufferImageView.byteLength = buffer_image.data.size();
    bufferImageView.byteOffset = 0;

    gltfModel.bufferViews.push_back(bufferVerticesView);
    gltfModel.bufferViews.push_back(bufferIndicesView);
    gltfModel.bufferViews.push_back(bufferImageView);

    //acccessors
    tinygltf::Accessor accessorPosition;
    accessorPosition.bufferView = 0;
    accessorPosition.byteOffset = 0;
    accessorPosition.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessorPosition.type = TINYGLTF_TYPE_VEC3;
    accessorPosition.count = verticesLen / 8;

    tinygltf::Accessor accessorNormal;
    accessorNormal.bufferView = 0;
    accessorNormal.byteOffset = 12;
    accessorNormal.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessorNormal.type = TINYGLTF_TYPE_VEC3;
    accessorNormal.count = verticesLen / 8;

    tinygltf::Accessor accessorTexCoord;
    accessorTexCoord.bufferView = 0;
    accessorTexCoord.byteOffset = 24;
    accessorTexCoord.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessorTexCoord.type = TINYGLTF_TYPE_VEC2;
    accessorTexCoord.count = verticesLen / 8;

    tinygltf::Accessor accessorIndices;
    accessorIndices.bufferView = 1;
    accessorIndices.byteOffset = 0;
    accessorIndices.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    accessorIndices.type = TINYGLTF_TYPE_SCALAR;
    accessorIndices.count = indexLen;

    gltfModel.accessors.push_back(accessorPosition);
    gltfModel.accessors.push_back(accessorNormal);
    gltfModel.accessors.push_back(accessorTexCoord);
    gltfModel.accessors.push_back(accessorIndices);

    //Primitive
    tinygltf::Primitive primitive;
    primitive.attributes["POSITION"] = 0;
    primitive.attributes["NORMAL"] = 1;
    primitive.attributes["TEXCOORD_0"] = 2;
    primitive.indices = 3;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    //Material
    tinygltf::TextureInfo ti;
    ti.index = 0;
    ti.texCoord = 0;
    tinygltf::Material material;
    material.pbrMetallicRoughness.baseColorTexture = ti;
    material.pbrMetallicRoughness.metallicFactor = 0.0;
    material.pbrMetallicRoughness.roughnessFactor = 1.0;
    gltfModel.materials.push_back(material);

    //Texture
    tinygltf::Texture texture;
    texture.sampler = 0;
    texture.source = 0;
    gltfModel.textures.push_back(texture);

    //Sampler, use default
    tinygltf::Sampler sampler;
    gltfModel.samplers.push_back(sampler);

    //Image
    tinygltf::Image image;
    image.bufferView = 2;
    image.bits = 8;
    image.pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    image.mimeType = "image/png";
    image.width = width;
    image.height = height;
    gltfModel.images.push_back(image);

    //Hook mesh, node and scene
    tinygltf::Mesh mesh;
    tinygltf::Node node;
    tinygltf::Scene scene;
    node.mesh = 0;
    mesh.primitives.push_back(primitive);
    scene.nodes.push_back(0);

    gltfModel.meshes.push_back(mesh);
    gltfModel.nodes.push_back(node);
    gltfModel.scenes.push_back(scene);
    gltfModel.asset.version = "2.0";

	return this->gltfModel;
}

