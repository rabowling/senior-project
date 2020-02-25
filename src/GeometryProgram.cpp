#include "GeometryProgram.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>

#include "GLSL.h"

bool GeometryProgram::init()
{
	GLint rc;

	// Create shader handles
	GLuint VS = glCreateShader(GL_VERTEX_SHADER);
	GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint GS = glCreateShader(GL_GEOMETRY_SHADER);

	// Read shader sources
	std::string vShaderString = readFileAsString(vShaderName);
	std::string fShaderString = readFileAsString(fShaderName);
    std::string gShaderString = readFileAsString(gShaderName);
	const char *vshader = vShaderString.c_str();
	const char *fshader = fShaderString.c_str();
    const char *gshader = gShaderString.c_str();
	CHECKED_GL_CALL(glShaderSource(VS, 1, &vshader, NULL));
	CHECKED_GL_CALL(glShaderSource(FS, 1, &fshader, NULL));
    CHECKED_GL_CALL(glShaderSource(GS, 1, &gshader, NULL));

	// Compile vertex shader
	CHECKED_GL_CALL(glCompileShader(VS));
	CHECKED_GL_CALL(glGetShaderiv(VS, GL_COMPILE_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printShaderInfoLog(VS);
			std::cout << "Error compiling vertex shader " << vShaderName << std::endl;
		}
		return false;
	}

	// Compile fragment shader
	CHECKED_GL_CALL(glCompileShader(FS));
	CHECKED_GL_CALL(glGetShaderiv(FS, GL_COMPILE_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printShaderInfoLog(FS);
			std::cout << "Error compiling fragment shader " << fShaderName << std::endl;
		}
		return false;
	}

    // Compile geometry shader
	CHECKED_GL_CALL(glCompileShader(GS));
	CHECKED_GL_CALL(glGetShaderiv(GS, GL_COMPILE_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printShaderInfoLog(GS);
			std::cout << "Error compiling geometry shader " << gShaderName << std::endl;
		}
		return false;
	}

	// Create the program and link
	pid = glCreateProgram();
	CHECKED_GL_CALL(glAttachShader(pid, VS));
	CHECKED_GL_CALL(glAttachShader(pid, FS));
    CHECKED_GL_CALL(glAttachShader(pid, GS));
	CHECKED_GL_CALL(glLinkProgram(pid));
	CHECKED_GL_CALL(glGetProgramiv(pid, GL_LINK_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printProgramInfoLog(pid);
			std::cout << "Error linking shaders " << vShaderName << ", " << fShaderName << ", and " << gShaderName << std::endl;
		}
		return false;
	}

	findUniformsAndAttributes();

	return true;
}

void GeometryProgram::setShaderNames(const std::string &v, const std::string &f, const std::string &g) {
    vShaderName = v;
	fShaderName = f;
    gShaderName = g;
}