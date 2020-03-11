
#include "Program.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>

#include "GLSL.h"


std::string readFileAsString(const std::string &fileName)
{
	std::string result;
	std::ifstream fileHandle(fileName);

	if (fileHandle.is_open())
	{
		fileHandle.seekg(0, std::ios::end);
		result.reserve((size_t) fileHandle.tellg());
		fileHandle.seekg(0, std::ios::beg);

		result.assign((std::istreambuf_iterator<char>(fileHandle)), std::istreambuf_iterator<char>());
	}
	else
	{
		std::cerr << "Could not open file: '" << fileName << "'" << std::endl;
	}

	return result;
}

void Program::setShaderNames(const std::string &v, const std::string &f)
{
	vShaderName = v;
	fShaderName = f;
}

bool Program::init()
{
	GLint rc;

	// Create shader handles
	GLuint VS = glCreateShader(GL_VERTEX_SHADER);
	GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shader sources
	std::string vShaderString = readFileAsString(vShaderName);
	std::string fShaderString = readFileAsString(fShaderName);
	const char *vshader = vShaderString.c_str();
	const char *fshader = fShaderString.c_str();
	CHECKED_GL_CALL(glShaderSource(VS, 1, &vshader, NULL));
	CHECKED_GL_CALL(glShaderSource(FS, 1, &fshader, NULL));

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

	// Create the program and link
	pid = glCreateProgram();
	CHECKED_GL_CALL(glAttachShader(pid, VS));
	CHECKED_GL_CALL(glAttachShader(pid, FS));
	CHECKED_GL_CALL(glLinkProgram(pid));
	CHECKED_GL_CALL(glGetProgramiv(pid, GL_LINK_STATUS, &rc));
	if (!rc)
	{
		if (isVerbose())
		{
			GLSL::printProgramInfoLog(pid);
			std::cout << "Error linking shaders " << vShaderName << " and " << fShaderName << std::endl;
		}
		return false;
	}

	findUniformsAndAttributes();

	return true;
}

void Program::bind()
{
	CHECKED_GL_CALL(glUseProgram(pid));
}

void Program::unbind()
{
	CHECKED_GL_CALL(glUseProgram(0));
}

void Program::addAttribute(const std::string &name)
{
	attributes[name] = GLSL::getAttribLocation(pid, name.c_str(), isVerbose());
}

void Program::addUniform(const std::string &name)
{
	uniforms[name] = GLSL::getUniformLocation(pid, name.c_str(), isVerbose());
}

GLint Program::getAttribute(const std::string &name) const
{
	std::map<std::string, GLint>::const_iterator attribute = attributes.find(name.c_str());
	if (attribute == attributes.end())
	{
		if (isVerbose())
		{
			std::cout << name << " is not an attribute variable" << std::endl;
		}
		return -1;
	}
	return attribute->second;
}

GLint Program::getUniform(const std::string &name) const
{
	std::map<std::string, GLint>::const_iterator uniform = uniforms.find(name.c_str());
	if (uniform == uniforms.end())
	{
		if (isVerbose())
		{
			std::cout << name << " is not a uniform variable" << std::endl;
		}
		return -1;
	}
	return uniform->second;
}

void Program::findUniformsAndAttributes() {
	GLint numActiveAttribs = 0;
	GLint numActiveUniforms = 0;
	glGetProgramiv(pid, GL_ACTIVE_ATTRIBUTES, &numActiveAttribs);
	glGetProgramiv(pid, GL_ACTIVE_UNIFORMS, &numActiveUniforms);

	// get attributes
	std::vector<GLchar> nameData(256);
	for (int attrib = 0; attrib < numActiveAttribs; ++attrib) {
		GLint arraySize = 0;
		GLenum type = 0;
		GLsizei actualLength = 0;
		glGetActiveAttrib(pid, attrib, nameData.size(), &actualLength, &arraySize, &type, &nameData[0]);
		std::string name((char*)&nameData[0], actualLength);
		if (arraySize == 1) {
			addAttribute(name);
		}
		else {
			for (int i = 0; i < arraySize; i++) {
				addAttribute(name.substr(0, name.length()-3) + "[" + std::to_string(i) + "]");
			}
		}
	}

	// get uniforms
	for(int unif = 0; unif < numActiveUniforms; ++unif)
	{
		GLint arraySize = 0;
		GLenum type = 0;
		GLsizei actualLength = 0;
		glGetActiveUniform(pid, unif, nameData.size(), &actualLength, &arraySize, &type, &nameData[0]);
		std::string name((char*)&nameData[0], actualLength);
		if (arraySize == 1) {
			addUniform(name);
		}
		else {
			for (int i = 0; i < arraySize; i++) {
				addUniform(name.substr(0, name.length()-3) + "[" + std::to_string(i) + "]");
			}
		}
	}
}
