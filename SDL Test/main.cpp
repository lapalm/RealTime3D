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
	0, 0,   1, 1,   1, 0,   0, 0,  
	0, 1,   1, 1,   0, 0,   1, 1,   
	1, 0,   0, 0,   0, 1,   1, 1,  
	0, 0,   1, 1,   1, 0,   0, 0,   
	0, 1,   1, 1,   0, 0,   0, 1,
	1, 1,   0, 0,   1, 1,   1, 0,   
	0, 0,   1, 0,   1, 1,   0, 0,   
	1, 1,   0, 1,   0, 0,   1, 0,  
    1, 1,   0, 0,   1, 1,   0, 1

};

glm::vec4 playerPosi();

GLfloat dx = 0.0f;
GLfloat dy = -1.0f;
GLfloat dz = -4.0;
GLfloat sx = 1.0f;
GLfloat sy = 1.0f;
GLfloat r = 0.0f;
GLuint meshObjects[6];
GLuint mvpShaderProgram;
glm::mat4 MVP;
std::stack<glm::mat4> mvStack;

glm::vec3 eye(0.0f, 3.0f, 4.0f); //Current Camera Position
glm::vec3 playerEye(0.0f, 3.0f, 5.0f); //Player Camera Position
glm::vec3 at(0.0f, 1.0f, 3.0f); // A point the camera is aiming towards
glm::vec3 playerAt(0.0f, 1.0f, 3.0f); // Player point the camera is aiming towards
glm::vec3 up(0.0f, 1.0f, 0.0f); // Vector to indicate which way up the camera is oriented
glm::vec3 playerUp(0.0f, 1.0f, 0.0f); //Player Vector to indicate which way up the camera is oriented



glm::vec4 lightPos(0.0f, 0.0f, 1.0f, 1.0f);

// Texture and lighting
GLuint textures[6];

GLuint loadBitmap(char *fname);

rt3d::lightStruct light0 = {
	{ 0.2f, 0.2f, 0.2f, 1.0f }, // ambient
	{ 0.7f, 0.7f, 0.7f, 1.0f }, // diffuse
	{ 0.8f, 0.8f, 0.8f, 1.0f }, // specular
	{ 0.0f, 0.0f, 1.0f, 1.0f }  // position
};

rt3d::materialStruct material0 = { // material with transparency
	{ 0.4f, 0.2f, 0.2f, 0.3f }, // ambient
	{ 0.8f, 0.5f, 0.5f, 0.3f }, // diffuse
	{ 1.0f, 0.8f, 0.8f, 0.3f }, // specular
	2.0f  // shininess
};

rt3d::materialStruct material1 = { // material with no transparency
	{ 0.2f, 0.2f, 0.4f, 1.0f }, // ambient
	{ 0.5f, 0.5f, 0.8f, 1.0f }, // diffuse
	{ 0.8f, 0.8f, 1.0f, 1.0f }, // specular
	2.0f  // shininess
};




// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
		rt3d::exitFatalError("Unable to initialize SDL");
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // 8 bit alpha buffering
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

glm::vec3 moveForward(glm::vec3 cam, GLfloat angle, GLfloat d) {
	return glm::vec3(cam.x + d*std::sin(angle*DEG_TO_RADIAN), cam.y, cam.z - d*std::cos(angle*DEG_TO_RADIAN));
}

glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::cos(angle*DEG_TO_RADIAN), pos.y, pos.z + d*std::sin(angle*DEG_TO_RADIAN));
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
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}



void draw(SDL_Window * window) {
	// clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// set up projection matrix
	glm::mat4 projection(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 50.0f);
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "projection", glm::value_ptr(projection));
	
	// set base position for scene
	glm::mat4 modelview(1.0);
	mvStack.push(modelview);
	at = moveForward(playerEye, r, 1.0f);
	mvStack.top() = glm::lookAt(eye, playerEye, up);

	glm::vec4 tmp = mvStack.top()*lightPos;
	rt3d::setLightPos(mvpShaderProgram, glm::value_ptr(tmp));

	// draw a cube for ground plane
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.1f, -10.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, 0.1f, 20.0f));
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(mvpShaderProgram, material1);
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// Cubes for buildings Different texture & individual material properties & positions
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	rt3d::materialStruct tmpMaterial = material0;
	for (int a = 0; a<4; a++) {
		for (int b = 0; b<4; b++) {
			tmpMaterial.ambient[0] = a * 0.33f;
			tmpMaterial.ambient[1] = b * 0.33f;
			tmpMaterial.diffuse[0] = a * 0.33f;
			tmpMaterial.diffuse[1] = b * 0.33f;
			tmpMaterial.specular[0] = a * 0.33f;
			tmpMaterial.specular[1] = b * 0.33f;
			rt3d::setMaterial(mvpShaderProgram, tmpMaterial);
			mvStack.push(mvStack.top());
			mvStack.top() = glm::translate(mvStack.top(), glm::vec3(a*3.0f, 0.0f, b*3.0f));
			mvStack.top() = glm::scale(mvStack.top(), glm::vec3(1.0f, 1.0f*(a + 0.1f), 1));
			rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
			rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
			mvStack.pop();
		}
	}

	//Player

	//at = moveForward(playerEye, r, 1.0f); 

	rt3d::setMaterial(mvpShaderProgram, material1);

	// render the "player"
	glBindTexture(GL_TEXTURE_2D, textures[0]); 
	mvStack.push(mvStack.top()); //modelview push modelview to stack
	
	mvStack.top() = glm::translate(mvStack.top(), playerEye);
	mvStack.top() = glm::rotate(mvStack.top(), float(r*DEG_TO_RADIAN), glm::vec3(glm::vec3(0.0f, 1.0f, 0.0f)));
	printf(" \n Draw Rotation: %f degrees \n", r);
	//glm::vec3 playerRotationAxis( ??, ??, ??);
	//glm::rotate(angle_in_degrees, myRotationAxis );
	
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	glDepthMask(GL_TRUE);

	SDL_GL_SwapWindow(window); // swap buffers
}

void update(void) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_W]) playerEye = moveForward(playerEye, r, -0.1f);
	if (keys[SDL_SCANCODE_S]) playerEye = moveForward(playerEye, r, 0.1f);
	if (keys[SDL_SCANCODE_A]) playerEye = moveRight(playerEye, r, 0.1f);
	if (keys[SDL_SCANCODE_D]) playerEye = moveRight(playerEye, r, -0.1f);
	if (keys[SDL_SCANCODE_R]) playerEye.y += 0.1f;
	if (keys[SDL_SCANCODE_F]) playerEye.y -= 0.1f;
	if (keys[SDL_SCANCODE_COMMA]) r += 1.0f;
	if (keys[SDL_SCANCODE_PERIOD]) r -= 1.0f;

	//glm::lookAt(eye, playerEye, up)

	//if (keys[SDL_SCANCODE_E]) r += 1.0f;
	//if (keys[SDL_SCANCODE_Q]) r -= 1.0f;

	if (keys[SDL_SCANCODE_P]) {
		dx = 0.0;
		dy = 0.0;
		sx = 1.0;
		sy = 1.0;
		r = 0.0;
	}
	
	eye.x = playerEye.x + 0.0f;
	cout << "playerEye.x: " << eye.x << endl;
	eye.y = playerEye.y + 3.0f;
	cout << "playerEye.y: " << eye.y << endl;
	//eye.y = playerEye.z + 10.0f;
	//cout << "playerEye.z: " << eye.y << endl;
	eye = moveForward(playerEye, r, 10.0f);
	cout << "playerEye.CameraDirection: " << eye.z << endl;
	printf("Camera Rotation: %f degrees", r);
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