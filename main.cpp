#include <vector>
#include <cassert>
#include <string>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../functions.cpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//#include "picopng.cpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const double PI = 4*atan(1);

using std::vector;

// The vertices should be specified in polar coordinates
vector<float> vertices;
vector<uint> indices;

vector<float> sphereToCart(double phi, double theta) {
	return {cos(theta)*sin(phi), sin(theta)*sin(phi), cos(phi)};
}

vector<float> makeSphere(int phiDivs, int thetaDivs) {
	// Set the top point so that I don't have to worry about
	// making sure it's not repeated
	vector<float> outvec {
		0.0f, 0.0f, 1.0f,	
	};
	
	// Reserve a bunch of values, approximates the surface area of a sphere
	// I don't actually know the formula, so this is just a guess
	// The * 3 is to approximately multiply by pi, and * 6 is to account for 
	// 6 vertices per grid square
	outvec.reserve(pow(phiDivs * thetaDivs, 2) * 3 * 6);
	
	double phi,theta;
	vector<float> temp;
	
	for (int phi_i=1; phi_i < phiDivs; phi_i++) {
		// Update phi value
		phi = (PI * phi_i) / phiDivs;
		
		for (int theta_i=1; theta_i <= thetaDivs; theta_i++) {
			// Update theta value
			theta = (2 * PI * theta_i) / thetaDivs;
			
			// Append cartesian coordinates to vector
			temp = sphereToCart(phi, theta);
			outvec.push_back(temp.at(0));
			outvec.push_back(temp.at(1));
			outvec.push_back(temp.at(2));
			
			// Add hsv color to vector
			//outvec.push_back(phi_i / phiDivs); // Hue
			//outvec.push_back(theta_i / thetaDivs); // Saturation
			//outvec.push_back( 0.5f ); // Value
			
			//outvec.push_back(phi / PI);
			//outvec.push_back(theta / (2*PI));
			
			//printf("%i, %i\n", phi_i, theta_i);
		}
	}
	
	// Add the bottom point
	outvec.push_back(0);
	outvec.push_back(0);
	outvec.push_back(-1);
	
	outvec.shrink_to_fit();
	return outvec;
}

vector<uint> makeIndices(int phiDivs, int thetaDivs, int arraySize) {
	
	// Testing EBO
	//return vector<uint>{ 0u, 1u, 2u };
	
	vector<uint> outvec;
	outvec.reserve(phiDivs * thetaDivs * 6);
	
	// Make the top part, since it needs triangles to start at indice 0
	for (int i=2; i <= thetaDivs; i++) {
		outvec.push_back(0);
		outvec.push_back(i-1);
		outvec.push_back(i);
	}
	// x is the index for going along the "theta" axis
	// y is the index for going along the "phi" axis	
	for (int y=1; y < phiDivs; y++) {
		for (int x=1; x < thetaDivs; x++) {
			// Note, adds "thetaDivs" to values when going down a row on the
			// sphere
			// Bottom left triangle in square
			outvec.push_back(x-1 + (y-1) * thetaDivs);
			outvec.push_back(x-1 +   y   * thetaDivs);
			outvec.push_back(x   +   y   * thetaDivs);
			
			// Top left triangle in square
			outvec.push_back(x-1 + (y-1) * thetaDivs);
			outvec.push_back(x   + (y-1) * thetaDivs);
			outvec.push_back(x   +   y   * thetaDivs);
		}
	}
	
	return outvec;
}

// Write data to a file for gnuplot to interpret
void writeData(std::string filename, vector<float> data) {
	FILE *file;
	file = fopen(filename.c_str(), "w");
	
	// Write data to file
	for (int i=2; i < data.size(); i += 3) {
		fprintf(file, "%.3f, %.3f, %.3f\n", data[i-2], data[i-1], data[i]);
	}
	
	// Close file
	fclose(file);
}

int main() {
	
	// Initialize vertices
	vertices = makeSphere(6, 6);
	indices  = makeIndices(6, 6, vertices.size());
	
	int arraySize = sizeof(vertices) / sizeof(vertices[0]);
	printf("%lu, %lu, %lu\n", sizeof(vertices), sizeof(vertices.data()),
			sizeof(vertices[0])*vertices.size());
	printf("Indices size: %lu\n", indices.size());

	writeData("test.txt", vertices);
	
	//return 0;
	
	glfwInit();
	GLFWwindow *win = glfwCreateWindow(512, 512, "tut2", NULL, NULL);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwMakeContextCurrent(win);

	glewExperimental = GL_TRUE;
	glewInit();
	
	glEnable(GL_DEBUG_OUTPUT);
	
	// Setup 2D texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	int width, height, nrChannels;
	unsigned char *data = stbi_load("texture.png", &width, &height,
			&nrChannels, 0);
	
	uint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	stbi_image_free(data);
	
	// Make buffer objects
	uint vao = makeVAO();
	uint vbo = makeVBO(vertices.data(), sizeof(vertices[0])*vertices.size());
	uint ebo = makeEBO(indices.data(), sizeof(indices[0])*indices.size());
	
	// Setup shaders
	uint vertexShader   = compileShaderFile("vert.glsl", GL_VERTEX_SHADER);
	checkShader(vertexShader, 0);
	
	uint fragmentShader = compileShaderFile("frag.glsl", GL_FRAGMENT_SHADER);
	checkShader(fragmentShader, 1);
	
	assert(glGetError() == GL_NO_ERROR);
	
	// Create program and attach shaders
	uint progId = glCreateProgram();
	glAttachShader(progId, vertexShader);
	glAttachShader(progId, fragmentShader);
	
	glLinkProgram(progId);
	
	checkProgram(progId);
	assert(glGetError() == GL_NO_ERROR);
	
	// xyz vertex input
	glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			6 * sizeof(float),
			(void*)0
	);
	glEnableVertexAttribArray(0);
	
	// hsv color input
	/*glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
			6 * sizeof(float),
			(void*) (3 * sizeof(float))
	);*/
	glEnableVertexAttribArray(1);
	
	assert(glGetError() == GL_NO_ERROR);
	
	glUseProgram(progId);
	glBindVertexArray(vao);
	
	checkError("An error happened before main loop");
	
	const float SPIN_SPEED = 0.05;
	while (!glfwWindowShouldClose(win)) {
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUseProgram(progId);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		
		checkError("An error happened in the main loop");
		glfwSwapBuffers(win);
		
		glfwPollEvents();
	}
	
	// Print the debug message log
	//GetFirstNMessages(10);
	
	glfwTerminate();
	return 0;
}
