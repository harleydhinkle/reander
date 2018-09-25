#include "render.h"
#include "glm/ext.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"
#include <iostream>
geometry makeGeometry(vertex * verts, size_t vertCount, unsigned int * indices, size_t indxCount)
{
	//create an instance of geometry
	geometry newGeo = {};
	newGeo.size = indxCount;
	//generate buffers
	glGenVertexArrays(1, &newGeo.vao);
	glGenBuffers(1, &newGeo.vbo);
	glGenBuffers(1, &newGeo.ibo);

	//bind buffers
	glBindVertexArray(newGeo.vao);
	glBindBuffer(GL_ARRAY_BUFFER, newGeo.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER , newGeo.ibo);
	//p
	glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(vertex), verts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indxCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
	//D
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GLU_FALSE, sizeof(vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GLU_FALSE, sizeof(vertex), (void*)16);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GLU_FALSE, sizeof(vertex), (void*)32);
	//un
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//re
	return newGeo;
	
}

geometry loadGeometry(std::string load)
{
	using namespace tinyobj;
	attrib_t vertexA;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string er;
	bool didIt = LoadObj(&vertexA, &shapes, &materials, &er, load.c_str());
	if (!er.empty()) { // `err` may contain warning message.
		std::cerr << er << std::endl;
	}

	if (didIt ==shapes.size()<1) 
	{
		return{};
	}
	std::vector<vertex>vertice;
	std::vector<unsigned int>indices;
	size_t offset = 0;
	for (size_t f = 0; f < shapes[0].mesh.num_face_vertices.size(); f++)
	{
		int faceV = shapes[0].mesh.num_face_vertices[f];
		assert(faceV == 3);
		for (size_t v = 0; v < faceV; v++) {
			// access to vertex
			index_t idx = shapes[0].mesh.indices[offset + v];
			real_t vx = vertexA.vertices[3 * idx.vertex_index + 0];
			real_t vy = vertexA.vertices[3 * idx.vertex_index + 1];
			real_t vz = vertexA.vertices[3 * idx.vertex_index + 2];
			real_t nx = vertexA.normals[3 * idx.normal_index + 0];
			real_t ny = vertexA.normals[3 * idx.normal_index + 1];
			real_t nz = vertexA.normals[3 * idx.normal_index + 2];
			real_t tx = vertexA.texcoords[2 * idx.texcoord_index + 0];
			real_t ty = vertexA.texcoords[2 * idx.texcoord_index + 1];
			// Optional: vertex colors
			// tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
			// tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
			// tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
			vertice.push_back(vertex{ {vx,vy,vz,1},{nx,ny,nz,1},{tx,ty} });
			indices.push_back(3 * f+v);
		}
		offset += faceV;
	}
	return makeGeometry(&vertice[0],vertice.size(),&indices[0], shapes[0].mesh.indices.size());
}

void freeGeometry(geometry & geo)
{
	glDeleteBuffers(1, &geo.vbo);
	glDeleteBuffers(1, &geo.ibo);
	glDeleteVertexArrays(1, &geo.vao);
	geo = {};


}

shader makeShader(const char * vertSource, const char * fragSource)
{
	shader newShad = {};
	newShad.program = glCreateProgram();
	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vert, 1, &vertSource, 0);
	
	glShaderSource(frag, 1, &fragSource, 0);
	
	glCompileShader(vert);
	GLint vertex_compiled;
	glGetShaderiv(vert, GL_COMPILE_STATUS, &vertex_compiled);
	if (vertex_compiled != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetShaderInfoLog(vert, 1024, &log_length, message);
		printf("%s\n",message);
	}
	glCompileShader(frag);
	GLint fragment_compiled;
	glGetShaderiv(frag, GL_COMPILE_STATUS, &fragment_compiled);
	if (fragment_compiled != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetShaderInfoLog(frag, 1024, &log_length, message);
		// Write the error to a log
		printf("%s\n", message);
	}
	glAttachShader(newShad.program, vert);
	glAttachShader(newShad.program, frag);
	glLinkProgram(newShad.program);
	GLint program_linked;
	glGetProgramiv(newShad.program, GL_LINK_STATUS, &program_linked);
	if (program_linked != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetProgramInfoLog(newShad.program, 1024, &log_length, message);
		// Write the error to a log
		printf("%s\n", message);
	}
	glDeleteShader(vert);
	glDeleteShader(frag);

	return newShad;
}

void freeShader(shader & shad)
{
	glDeleteProgram(shad.program);
	shad = {};
}

texture loadTexture(const char * imagePath)
{

	int imageWidth, imageHeight, imageFormat;
	imageWidth = imageHeight = imageFormat = -1;
	unsigned char *rawPixelData = nullptr;

	//load the image 
	stbi_set_flip_vertically_on_load(true);
	rawPixelData = stbi_load(imagePath, &imageWidth, &imageHeight, &imageFormat, STBI_default);

	//get the format/spec/whatever



	//pass it opengl
	texture newTex = makeTexture(imageWidth, imageHeight, imageFormat, rawPixelData);



	//destroy any other data
	stbi_image_free(rawPixelData);


	return newTex;
}

texture makeTexture(unsigned width, unsigned height, unsigned channels, const unsigned char * pixels)
{
	GLuint oglFormat = 0;
	switch (channels)
	{
	case 1: oglFormat = GL_RED; break;
	case 2: oglFormat = GL_RG; break;
	case 3: oglFormat = GL_RGB; break; 
	case 4: oglFormat = GL_RGBA; break;
	}


	texture newTex = { 0,width,height,channels };


	glGenTextures(1, &newTex.handle);
	glBindTexture(GL_TEXTURE_2D, newTex.handle);

	glTexImage2D(GL_TEXTURE_2D, 0, oglFormat, width, height, 0, oglFormat, GL_UNSIGNED_BYTE, pixels);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	return newTex;
}

void freeTexture(texture & tex)
{
	glDeleteTextures(1, &tex.handle);
	tex = {};
}

void draw(const shader & shad, const geometry & geo)
{
	glUseProgram(shad.program);
	glBindVertexArray(geo.vao);
	glDrawElements(GL_TRIANGLES, geo.size, GL_UNSIGNED_INT, 0);
}

void setUniform(const shader & shad, GLuint location, const glm::mat4 & value)
{
	glProgramUniformMatrix4fv(shad.program, location, 1, GL_FALSE, glm::value_ptr(value));

}

void setUniform(const shader & shad, GLuint location, const texture & value, GLuint textureSlot)
{
	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(GL_TEXTURE_2D, value.handle);
	glProgramUniform1i(shad.program, location, textureSlot);
}

void setUniform(const shader & shad, GLuint location, const glm::vec3 & value)
{
	glProgramUniform3fv(shad.program, location, 1, glm::value_ptr(value));
}



