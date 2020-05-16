#ifndef __VTILE_H__
#define __VTILE_H__

#include "stdheader.h"
#include "Vector.h"
#ifndef _WIN64
#include "SFML/Graphics.hpp" // TODO: remove after testing png loading ends
#endif
#include "VElevationTile.h"


class VTile {

public:
	VTile(Vector2i tileIndices, Vector2f position, Vector2f coordinates, float width, int level);
	VTile();

	~VTile();

	shared_ptr<char[]> getImage();
	shared_ptr<char[]> getElevation();
	shared_ptr<char[]> getGltf();

	Vector2f getPosition();
	Vector2f getCoordinates();
	Vector2i getIndices();

	string getId();
	float width();
	int level();

	unsigned getImageSize();
	unsigned getElevationSize();
	unsigned getGltfSize();

#ifndef _WIN64
	sf::Texture getTexture();
#endif

	////////////////////////////////////////////////////////////
	/// \Set vertex's global transform for this tile, used for merge tiles into global picture. Need be used before get vertices.
	///
	/// \param scale Each vertex's scale
	/// \param offset Each vertex's offset
	///
	////////////////////////////////////////////////////////////
	void       setMeshTranslate(const Vector3f& scale, const Vector3f& offset);

	////////////////////////////////////////////////////////////
	/// \brief get Tile with Dem's vertices array, pass an array in with its offset, then it will fill
	/// in data by vertices by 5 floats: x, y, z, u, v
	///
	/// \param vertices Pass in float array, need be allocated outside. x, y, z will be meters and the origin(0, 0, ele) will be left top
	/// \param verticesLen Output vertex array length in this tile, will be 5 * vertex count
	/// \param offsetStart Pass in float array's expect array start index, default 0, it can use a big array for multiple tiles
	/// \param simplifyStep Pass in resample step for meshes, default 1, if it is 2, means each 2 dem point will have one sampling vertex.
	///
	////////////////////////////////////////////////////////////
	bool       getTileVertices(float* vertices, int& verticesLen, int offsetStart = 0, int simplifyStep = 1);

    ////////////////////////////////////////////////////////////
	/// \brief Get Tile with Dem square's triangles£¬pass in an arry with its offset. 
	///
	/// \param indices Passed in index array
	/// \param indicesLen, Output index array's length in this tile. Each square will have 6 indices, 2 trianlges.
	/// \param offsetStart Pass in float array's expect array start index, default 0, it can use a big array for multiple tiles
	/// \param simplifyStep Pass in resample step for meshes, default 1, if it is 2, means each 2 dem point will have one sampling vertex.
	///
	////////////////////////////////////////////////////////////
	bool       getTileIndices(unsigned int* indices, int& indicesLen, int offsetStart = 0, int simplifyStep = 1);

	////////////////////////////////////////////////////////////
	/// \brief Get Texture
	///
	/// \param width Output width
	/// \param height Output height
	///
	////////////////////////////////////////////////////////////
	unsigned char* getTexture(int& width, int& height);

private:

	float m_tileWidth;
	int m_level;
	
	string m_imageFile;
	string m_elevationFile;
	string m_gltfFile;

	Vector2f m_position;
	Vector2i m_tileIndices;
	Vector2f m_coordinates;

	shared_ptr<char[]> m_pImage; // TODO: If needed by IG, use unique_ptr
	shared_ptr<char[]> m_pGltf; 
	shared_ptr<char[]> m_pElevation; 

	unsigned m_imageSize;
	unsigned m_elevationSize;
	unsigned m_gltfSize;

#ifndef _WIN64
	sf::Texture tileTexture;
#endif
	VElevationTile elevationTile;

	void loadImage();
	void loadElevation();
	void loadGltf();

	void loadFile(string& filename, shared_ptr<char[]>& store, unsigned& size);
};

#endif // #ifndef __VTILE_H__