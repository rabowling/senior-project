
#pragma once

#ifndef LAB471_SHAPE_H_INCLUDED
#define LAB471_SHAPE_H_INCLUDED

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Program;

class Shape
{

public:

	void loadMesh(const std::string &meshName);
	void saveObj(const std::string &fileName);
	void init();
	void draw(const Program *prog) const;

private:

	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;

	unsigned int eleBufID = 0;
	unsigned int posBufID = 0;
	unsigned int norBufID = 0;
	unsigned int texBufID = 0;
	unsigned int vaoID = 0;

	glm::vec3 scale;
	glm::vec3 shift;
	int gridSizeX;
	int gridSizeZ;
};

#endif // LAB471_SHAPE_H_INCLUDED
