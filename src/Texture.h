#pragma once
#ifndef __Texture__
#define __Texture__

#include <glad/glad.h>
#include <string>

class Texture
{
public:
	Texture();
	virtual ~Texture();
	void loadTexture(const std::string &filename);
	void init();
	void setUnit(GLint u) { unit = u; }
	GLint getUnit() const { return unit; }
	void bind(GLint handle);
	void unbind();
	void setWrapModes(GLint wrapS, GLint wrapT); // Must be called after init()
	GLint getID() const { return tid;}
	int width;
	int height;
	unsigned char *data;
private:
	GLuint tid;
	GLint unit;
	int ncomps;
	
};

#endif
