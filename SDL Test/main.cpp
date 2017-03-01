// Based loosly on the first triangle OpenGL tutorial
// http://www.opengl.org/wiki/Tutorial:_OpenGL_3.1_The_First_Triangle_%28C%2B%2B/Win%29
// This program will render two triangles
// Most of the OpenGL code for dealing with buffer objects, etc has been moved to a 
// utility library, to make creation and display of mesh objects as simple as possible

// Windows specific: Uncomment the following line to open a console window for debug output
#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif

#include <stack>
#include "rt3d.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define DEG_TO_RADIAN 0.017453293
using namespace rt3d;
using namespace std;



// Data would normally be read from files
GLuint cubeVertCount = 36;
GLfloat cubeVerts[] = { 
	0, 0, 0,        1, 1, 0,        1, 0, 0,        0, 0, 0,        0, 1, 0,
	1, 1, 0,        0, 0, 0,        0, 1, 1,        0, 1, 0,        0, 0, 0,
	0, 0, 1,        0, 1, 1,        0, 1, 0,        1, 1, 1,        1, 1, 0,
	0, 1, 0,        0, 1, 1,        1, 1, 1,        1, 0, 0,        1, 1, 0,
	1, 1, 1,        1, 0, 0,        1, 1, 1,        1, 0, 1,        0, 0, 0,
	1, 0, 0,        1, 0, 1,        0, 0, 0,        1, 0, 1,        0, 0, 1,
	0, 0, 1,        1, 0, 1,        1, 1, 1,        0, 0, 1,        1, 1, 1,
	0, 1, 1
};

GLfloat cubeNorms[] = {
	0, 0, -1,       0, 0, -1,       0, 0, -1,       0, 0, -1,       0, 0, -1,
	0, 0, -1,       -1, 0, 0,       -1, 0, 0,       -1, 0, 0,       -1, 0, 0,
	-1, 0, 0,       -1, 0, 0,       0, 1, 0,        0, 1, 0,        0, 1, 0,
	0, 1, 0,        0, 1, 0,        0, 1, 0,        1, 0, 0,        1, 0, 0,
	1, 0, 0,        1, 0, 0,        1, 0, 0,        1, 0, 0,        0, -1, 0,
	0, -1, 0,       0, -1, 0,       0, -1, 0,       0, -1, 0,       0, -1, 0,
	0, 0, 1,        0, 0, 1,        0, 0, 1,        0, 0, 1,        0, 0, 1,
	0, 0, 1

};

GLfloat cubeColours[] = { 0.0f, 0.0f, 0.0f,
0.0f, 1.0f, 0.0f,
1.0f, 1.0f, 0.0f,
1.0f, 0.0f, 0.0f,
0.0f, 0.0f, 1.0f,
0.0f, 1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
1.0f, 0.0f, 1.0f };

GLuint cubeIndexCount = 36;
GLuint cubeIndices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35
}; 

GLfloat cubeTexCoords[] = { 
	0, 0,   1, 1,   1, 0,   0, 0,   0, 1,   1, 1,   0, 0,   1, 1,   1, 0,   0, 0,
	0, 1,   1, 1,   0, 0,   1, 1,   1, 0,   0, 0,   0, 1,   1, 1,   0, 0,   0, 1,
	1, 1,   0, 0,   1, 1,   1, 0,   0, 0,   1, 0,   1, 1,   0, 0,   1, 1,   0, 1,
	0, 0,   1, 0,   1, 1,   0, 0,   1, 1,   0, 1

};


GLfloat dx = 0.0f;
GLfloat dy = 0.0f;
GLfloat sx = 1.0f;
GLfloat sy = 1.0f;
GLfloat r = 0.0f;
GLuint meshObjects[6];
GLuint mvpShaderProgram;
glm::mat4 MVP;
std::stack<glm::mat4> mvStack;

// Texture and lighting
GLuint textures[6];

GLuint loadBitmap(char *fname);

rt3d::lightStruct light0 = {
	{ 0.2f, 0.2f, 0.2f, 1.0f }, // ambient
	{ 0.7f, 0.7f, 0.7f, 1.0f }, // diffuse
	{ 0.8f, 0.8f, 0.8f, 1.0f }, // specular
	{ 0.0f, 0.0f, 1.0f, 1.0f }  // position
};

rt3d::materialStruct material0 = {
	{ 0.4f, 0.2f, 0.2f, 1.0f }, // ambient
	{ 0.8f, 0.5f, 0.5f, 1.0f }, // diffuse
	{ 1.0f, 0.8f, 0.8f, 1.0f }, // specular
	2.0f  // shininess
};



// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
		rt3d::exitFatalError("Unable to initialize SDL");

	// Request an OpenGL 3.0 context.
	// Not able to use SDL to choose profile (yet), should default to core profile on 3.2 or later
	// If you request a context not supported by your drivers, no OpenGL context will be created

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)

													   // Create 800x600 window
	window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!window) // Check window was created OK
		rt3d::exitFatalError("Unable to create window");

	context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
	SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

void init(void) {
	// For this simple example we'll be using the most basic of shader programs
	mvpShaderProgram = rt3d::initShaders("phong-tex.vert", "phong-tex.frag");

	textures[0] = loadBitmap("fabric.bmp");
	textures[1] = loadBitmap("studdedmetal.bmp");
	textures[2] = loadBitmap("chocobo.bmp");
	textures[3] = loadBitmap("fatchocobo.bmp");
	textures[4] = loadBitmap("chocobo2.bmp");
	textures[5] = loadBitmap("randompic.bmp");
	

	rt3d::setLight(mvpShaderProgram, light0);
	rt3d::setMaterial(mvpShaderProgram, material0);

	//MVP = glm::mat4(1.0); //init to identify matrix
	// Going to create our mesh objects here
	meshObjects[0] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeNorms, cubeTexCoords, cubeIndexCount, cubeIndices);
	
	meshObjects[1] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeNorms, cubeTexCoords, cubeIndexCount, cubeIndices);

	meshObjects[2] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeNorms, cubeTexCoords, cubeIndexCount, cubeIndices);

	meshObjects[3] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeNorms, cubeTexCoords, cubeIndexCount, cubeIndices);

	meshObjects[4] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeNorms, cubeTexCoords, cubeIndexCount, cubeIndices);

	meshObjects[5] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeNorms, cubeTexCoords, cubeIndexCount, cubeIndices);
	glEnable(GL_DEPTH_TEST); // enable depth testing
}

void draw(SDL_Window * window) {
	// clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// set up projection matrix
	glm::mat4 projection(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 50.0f);
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "projection", glm::value_ptr(projection));

	// render the sun
	glm::mat4 modelview(1.0);
	glBindTexture(GL_TEXTURE_2D, textures[0]); // fabric texture
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, 0.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), r, glm::vec3(0.0f, 1.0f, 0.0f));
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// render the sun 2
	glBindTexture(GL_TEXTURE_2D, textures[1]); // fabric texture
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, -1.5f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), r, glm::vec3(0.0f, 1.0f, 0.0f));
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[1], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// render the sun 3
	glBindTexture(GL_TEXTURE_2D, textures[2]); // fabric texture
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(1.5f, -1.5f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), r, glm::vec3(0.0f, 1.0f, 0.0f));
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[2], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// render the sun 4
	glBindTexture(GL_TEXTURE_2D, textures[3]); // fabric texture
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(1.5f, 0.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), r, glm::vec3(0.0f, 1.0f, 0.0f));
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[3], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// render the sun 5
	glBindTexture(GL_TEXTURE_2D, textures[4]); // fabric texture
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-1.5f, 0.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), r, glm::vec3(0.0f, 1.0f, 0.0f));
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[4], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// render the sun 5
	glBindTexture(GL_TEXTURE_2D, textures[5]); // fabric texture
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-1.5f, -1.5f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), r, glm::vec3(0.0f, 1.0f, 0.0f));
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[5], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();



	// Now to push another stack, add transformations and draw moon #1
	// then pop stack. 
	// Repeat until all moons rendered, then pop stack...
	// then for each additional planet � push stack, add transforms, draw planet,
	// draw moons, pop stack


	SDL_GL_SwapWindow(window); // swap buffers
}

void update(void) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_W]) dy += 0.1;
	if (keys[SDL_SCANCODE_S]) dy -= 0.1;
	if (keys[SDL_SCANCODE_D]) dx += 0.1;
	if (keys[SDL_SCANCODE_A]) dx -= 0.1;

	if (keys[SDL_SCANCODE_UP]) sy += 0.1;
	if (keys[SDL_SCANCODE_DOWN]) sy -= 0.1;
	if (keys[SDL_SCANCODE_LEFT]) sx += 0.1;
	if (keys[SDL_SCANCODE_RIGHT]) sx -= 0.1;

	if (keys[SDL_SCANCODE_R]) {
		dx = 0.0;
		dy = 0.0;
		sx = 1.0;
		sy = 1.0;
		r = 0.0;
	}

	if (keys[SDL_SCANCODE_Q]) r += 0.3;
	if (keys[SDL_SCANCODE_E]) r -= 0.3;

}

// A simple texture loading function
// lots of room for improvement - and better error checking!
GLuint loadBitmap(char *fname)
{
	GLuint texID;
	glGenTextures(1, &texID); // generate texture ID
							  // load file - using core SDL library
	SDL_Surface *tmpSurface;
	tmpSurface = SDL_LoadBMP(fname);
	if (!tmpSurface)
	{
		std::cout << "Error loading bitmap" << std::endl;
	}
	std::cout << "Success loading bitmap" << std::endl;
	// bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	SDL_PixelFormat *format = tmpSurface->format;
	GLuint externalFormat, internalFormat;
	if (format->Amask) {
		internalFormat = GL_RGBA;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGBA : GL_BGRA;
	}
	else {
		internalFormat = GL_RGB;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tmpSurface->w, tmpSurface->h, 0,
		externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(tmpSurface); // texture loaded, free the temporary buffer
	return texID;	// return value of texture ID
}



// Program entry point - SDL manages the actual WinMain entry point for us
int main(int argc, char *argv[]) {
	SDL_Window * hWindow; // window handle
	SDL_GLContext glContext; // OpenGL context handle
	hWindow = setupRC(glContext); // Create window and render context 

								  // Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit(1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

	bool running = true; // set running to true
	SDL_Event sdlEvent;  // variable to detect SDL events
	while (running) {	// the event loop
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT)
				running = false;
		}
		update();
		draw(hWindow); // call the draw function
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(hWindow);
	SDL_Quit();
	return 0;
}